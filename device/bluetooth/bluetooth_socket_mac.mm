// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/bluetooth/bluetooth_socket_mac.h"

#import <IOBluetooth/IOBluetooth.h>
#include <stdint.h>

#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include "base/bind.h"
#include "base/callback.h"
#include "base/callback_helpers.h"
#include "base/mac/scoped_cftyperef.h"
#include "base/memory/ref_counted.h"
#include "base/numerics/safe_conversions.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/strings/sys_string_conversions.h"
#include "base/threading/thread_restrictions.h"
#include "device/bluetooth/bluetooth_adapter_mac.h"
#include "device/bluetooth/bluetooth_channel_mac.h"
#include "device/bluetooth/bluetooth_classic_device_mac.h"
#include "device/bluetooth/bluetooth_device.h"
#include "device/bluetooth/bluetooth_l2cap_channel_mac.h"
#include "device/bluetooth/bluetooth_rfcomm_channel_mac.h"
#include "net/base/io_buffer.h"
#include "net/base/net_errors.h"

#if !defined(MAC_OS_X_VERSION_10_7) || \
    MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_7

@interface IOBluetoothDevice (LionSDKDeclarations)
- (IOReturn)performSDPQuery:(id)target uuids:(NSArray*)uuids;
@end

#endif  // MAC_OS_X_VERSION_10_7

using device::BluetoothSocket;

// A simple helper class that forwards SDP query completed notifications to its
// wrapped |socket_|.
@interface SDPQueryListener : NSObject {
 @private
  // The socket that registered for notifications.
  scoped_refptr<device::BluetoothSocketMac> socket_;

  // Callbacks associated with the request that triggered this SDP query.
  base::Closure success_callback_;
  BluetoothSocket::ErrorCompletionCallback error_callback_;

  // The device being queried.
  IOBluetoothDevice* device_;  // weak
}

- (id)initWithSocket:(scoped_refptr<device::BluetoothSocketMac>)socket
              device:(IOBluetoothDevice*)device
    success_callback:(base::Closure)success_callback
      error_callback:(BluetoothSocket::ErrorCompletionCallback)error_callback;
- (void)sdpQueryComplete:(IOBluetoothDevice*)device status:(IOReturn)status;

@end

@implementation SDPQueryListener

- (id)initWithSocket:(scoped_refptr<device::BluetoothSocketMac>)socket
              device:(IOBluetoothDevice*)device
    success_callback:(base::Closure)success_callback
      error_callback:(BluetoothSocket::ErrorCompletionCallback)error_callback {
  if ((self = [super init])) {
    socket_ = socket;
    device_ = device;
    success_callback_ = success_callback;
    error_callback_ = error_callback;
  }

  return self;
}

- (void)sdpQueryComplete:(IOBluetoothDevice*)device status:(IOReturn)status {
  DCHECK_EQ(device, device_);
  socket_->OnSDPQueryComplete(
      status, device, success_callback_, error_callback_);
}

@end

// A simple helper class that forwards RFCOMM channel opened notifications to
// its wrapped |socket_|.
@interface BluetoothRfcommConnectionListener : NSObject {
 @private
  // The socket that owns |self|.
  device::BluetoothSocketMac* socket_;  // weak

  // The OS mechanism used to subscribe to and unsubscribe from RFCOMM channel
  // creation notifications.
  IOBluetoothUserNotification* rfcommNewChannelNotification_;  // weak
}

- (id)initWithSocket:(device::BluetoothSocketMac*)socket
           channelID:(BluetoothRFCOMMChannelID)channelID;
- (void)rfcommChannelOpened:(IOBluetoothUserNotification*)notification
                    channel:(IOBluetoothRFCOMMChannel*)rfcommChannel;

@end

@implementation BluetoothRfcommConnectionListener

- (id)initWithSocket:(device::BluetoothSocketMac*)socket
           channelID:(BluetoothRFCOMMChannelID)channelID {
  if ((self = [super init])) {
    socket_ = socket;

    SEL selector = @selector(rfcommChannelOpened:channel:);
    const auto kIncomingDirection =
        kIOBluetoothUserNotificationChannelDirectionIncoming;
    rfcommNewChannelNotification_ =
        [IOBluetoothRFCOMMChannel
          registerForChannelOpenNotifications:self
                                     selector:selector
                                withChannelID:channelID
                                    direction:kIncomingDirection];
  }

  return self;
}

- (void)dealloc {
  [rfcommNewChannelNotification_ unregister];
  [super dealloc];
}

- (void)rfcommChannelOpened:(IOBluetoothUserNotification*)notification
                    channel:(IOBluetoothRFCOMMChannel*)rfcommChannel {
  if (notification != rfcommNewChannelNotification_) {
    // This case is reachable if there are pre-existing RFCOMM channels open at
    // the time that the listener is created. In that case, each existing
    // channel calls into this method with a different notification than the one
    // this class registered with. Ignore those; this class is only interested
    // in channels that have opened since it registered for notifications.
    return;
  }

  socket_->OnChannelOpened(std::unique_ptr<device::BluetoothChannelMac>(
      new device::BluetoothRfcommChannelMac(NULL, [rfcommChannel retain])));
}

@end

// A simple helper class that forwards L2CAP channel opened notifications to
// its wrapped |socket_|.
@interface BluetoothL2capConnectionListener : NSObject {
 @private
  // The socket that owns |self|.
  device::BluetoothSocketMac* socket_;  // weak

  // The OS mechanism used to subscribe to and unsubscribe from L2CAP channel
  // creation notifications.
  IOBluetoothUserNotification* l2capNewChannelNotification_;  // weak
}

- (id)initWithSocket:(device::BluetoothSocketMac*)socket
                 psm:(BluetoothL2CAPPSM)psm;
- (void)l2capChannelOpened:(IOBluetoothUserNotification*)notification
                   channel:(IOBluetoothL2CAPChannel*)l2capChannel;

@end

@implementation BluetoothL2capConnectionListener

- (id)initWithSocket:(device::BluetoothSocketMac*)socket
                 psm:(BluetoothL2CAPPSM)psm {
  if ((self = [super init])) {
    socket_ = socket;

    SEL selector = @selector(l2capChannelOpened:channel:);
    const auto kIncomingDirection =
        kIOBluetoothUserNotificationChannelDirectionIncoming;
    l2capNewChannelNotification_ =
        [IOBluetoothL2CAPChannel
          registerForChannelOpenNotifications:self
                                     selector:selector
                                      withPSM:psm
                                    direction:kIncomingDirection];
  }

  return self;
}

- (void)dealloc {
  [l2capNewChannelNotification_ unregister];
  [super dealloc];
}

- (void)l2capChannelOpened:(IOBluetoothUserNotification*)notification
                   channel:(IOBluetoothL2CAPChannel*)l2capChannel {
  if (notification != l2capNewChannelNotification_) {
    // This case is reachable if there are pre-existing L2CAP channels open at
    // the time that the listener is created. In that case, each existing
    // channel calls into this method with a different notification than the one
    // this class registered with. Ignore those; this class is only interested
    // in channels that have opened since it registered for notifications.
    return;
  }

  socket_->OnChannelOpened(std::unique_ptr<device::BluetoothChannelMac>(
      new device::BluetoothL2capChannelMac(NULL, [l2capChannel retain])));
}

@end

namespace device {
namespace {

// It's safe to use 0 to represent invalid channel or PSM port numbers, as both
// are required to be non-zero for valid services.
const BluetoothRFCOMMChannelID kInvalidRfcommChannelId = 0;
const BluetoothL2CAPPSM kInvalidL2capPsm = 0;

const char kInvalidOrUsedChannel[] = "Invalid channel or already in use";
const char kInvalidOrUsedPsm[] = "Invalid PSM or already in use";
const char kProfileNotFound[] = "Profile not found";
const char kSDPQueryFailed[] = "SDP query failed";
const char kSocketConnecting[] = "The socket is currently connecting";
const char kSocketAlreadyConnected[] = "The socket is already connected";
const char kSocketNotConnected[] = "The socket is not connected";
const char kReceivePending[] = "A Receive operation is pending";

template <class T>
void empty_queue(std::queue<T>& queue) {
  std::queue<T> empty;
  std::swap(queue, empty);
}

// Converts |uuid| to a IOBluetoothSDPUUID instance.
IOBluetoothSDPUUID* GetIOBluetoothSDPUUID(const BluetoothUUID& uuid) {
  // The canonical UUID format is XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX.
  const std::string uuid_str = uuid.canonical_value();
  DCHECK_EQ(uuid_str.size(), 36U);
  DCHECK_EQ(uuid_str[8], '-');
  DCHECK_EQ(uuid_str[13], '-');
  DCHECK_EQ(uuid_str[18], '-');
  DCHECK_EQ(uuid_str[23], '-');
  std::string numbers_only = uuid_str;
  numbers_only.erase(23, 1);
  numbers_only.erase(18, 1);
  numbers_only.erase(13, 1);
  numbers_only.erase(8, 1);
  std::vector<uint8_t> uuid_bytes_vector;
  base::HexStringToBytes(numbers_only, &uuid_bytes_vector);
  DCHECK_EQ(uuid_bytes_vector.size(), 16U);

  return [IOBluetoothSDPUUID uuidWithBytes:&uuid_bytes_vector.front()
                                    length:uuid_bytes_vector.size()];
}

// Converts the given |integer| to a string.
NSString* IntToNSString(int integer) {
  return [@(integer) stringValue];
}

// Returns a dictionary containing the Bluetooth service definition
// corresponding to the provided |uuid|, |name|, and |protocol_definition|. Does
// not include a service name in the definition if |name| is null.
NSDictionary* BuildServiceDefinition(const BluetoothUUID& uuid,
                                     const std::string* name,
                                     NSArray* protocol_definition) {
  NSMutableDictionary* service_definition = [NSMutableDictionary dictionary];

  if (name) {
    // TODO(isherman): The service's language is currently hardcoded to English.
    // The language should ideally be specified in the chrome.bluetooth API
    // instead.
    const int kEnglishLanguageBase = 100;
    const int kServiceNameKey =
        kEnglishLanguageBase + kBluetoothSDPAttributeIdentifierServiceName;
    NSString* service_name = base::SysUTF8ToNSString(*name);
    [service_definition setObject:service_name
                           forKey:IntToNSString(kServiceNameKey)];
  }

  const int kUUIDsKey = kBluetoothSDPAttributeIdentifierServiceClassIDList;
  NSArray* uuids = @[GetIOBluetoothSDPUUID(uuid)];
  [service_definition setObject:uuids forKey:IntToNSString(kUUIDsKey)];

  const int kProtocolDefinitionsKey =
      kBluetoothSDPAttributeIdentifierProtocolDescriptorList;
  [service_definition setObject:protocol_definition
                         forKey:IntToNSString(kProtocolDefinitionsKey)];

  return service_definition;
}

// Returns a dictionary containing the Bluetooth RFCOMM service definition
// corresponding to the provided |uuid| and |options|.
NSDictionary* BuildRfcommServiceDefinition(
    const BluetoothUUID& uuid,
    const BluetoothAdapter::ServiceOptions& options) {
  int channel_id = options.channel ? *options.channel : kInvalidRfcommChannelId;
  NSArray* rfcomm_protocol_definition = @[
    @[ [IOBluetoothSDPUUID uuid16:kBluetoothSDPUUID16L2CAP] ],
    @[
      [IOBluetoothSDPUUID uuid16:kBluetoothSDPUUID16RFCOMM],
      @{
        @"DataElementType" : @1,  // Unsigned integer.
        @"DataElementSize" : @1,  // 1 byte.
        @"DataElementValue" : @(channel_id),
      },
    ],
  ];
  return BuildServiceDefinition(
      uuid, options.name.get(), rfcomm_protocol_definition);
}

// Returns a dictionary containing the Bluetooth L2CAP service definition
// corresponding to the provided |uuid| and |options|.
NSDictionary* BuildL2capServiceDefinition(
    const BluetoothUUID& uuid,
    const BluetoothAdapter::ServiceOptions& options) {
  int psm = options.psm ? *options.psm : kInvalidL2capPsm;
  NSArray* l2cap_protocol_definition = @[
    @[
      [IOBluetoothSDPUUID uuid16:kBluetoothSDPUUID16L2CAP],
      @{
        @"DataElementType" : @1,  // Unsigned integer.
        @"DataElementSize" : @2,  // 2 bytes.
        @"DataElementValue" : @(psm),
      },
    ],
  ];
  return BuildServiceDefinition(
      uuid, options.name.get(), l2cap_protocol_definition);
}

// Registers a Bluetooth service with the specified |service_definition| in the
// system SDP server. Returns the registered service on success. If the service
// could not be registered, or if |verify_service_callback| indicates that the
// to-be-registered service was not configured correctly, returns nil.
IOBluetoothSDPServiceRecord* RegisterService(
    NSDictionary* service_definition,
    const base::Callback<bool(IOBluetoothSDPServiceRecord*)>&
        verify_service_callback) {
  // Attempt to register the service.
  IOBluetoothSDPServiceRecord* service_record = [IOBluetoothSDPServiceRecord
      publishedServiceRecordWithDictionary:service_definition];

  // Verify that the registered service was configured correctly. If not,
  // withdraw the service.
  if (!service_record || !verify_service_callback.Run(service_record)) {
    [service_record removeServiceRecord];
    service_record = nil;
  }

  return service_record;
}

// Returns true iff the |requested_channel_id| was registered in the RFCOMM
// |service_record|. If it was, also updates |registered_channel_id| with the
// registered value, as the requested id may have been left unspecified.
bool VerifyRfcommService(const int* requested_channel_id,
                         BluetoothRFCOMMChannelID* registered_channel_id,
                         IOBluetoothSDPServiceRecord* service_record) {
  // Test whether the requested channel id was available.
  // TODO(isherman): The OS doesn't seem to actually pick a random channel if we
  // pass in |kInvalidRfcommChannelId|.
  BluetoothRFCOMMChannelID rfcomm_channel_id;
  IOReturn result = [service_record getRFCOMMChannelID:&rfcomm_channel_id];
  if (result != kIOReturnSuccess ||
      (requested_channel_id && rfcomm_channel_id != *requested_channel_id)) {
    return false;
  }

  *registered_channel_id = rfcomm_channel_id;
  return true;
}

// Registers an RFCOMM service with the specified |uuid|, |options.channel_id|,
// and |options.name| in the system SDP server. Automatically allocates a
// channel if |options.channel_id| is null. Does not specify a name if
// |options.name| is null. Returns a handle to the registered service and
// updates |registered_channel_id| to the actual channel id, or returns nil if
// the service could not be registered.
IOBluetoothSDPServiceRecord* RegisterRfcommService(
    const BluetoothUUID& uuid,
    const BluetoothAdapter::ServiceOptions& options,
    BluetoothRFCOMMChannelID* registered_channel_id) {
  return RegisterService(
      BuildRfcommServiceDefinition(uuid, options),
      base::Bind(
          &VerifyRfcommService, options.channel.get(), registered_channel_id));
}

// Returns true iff the |requested_psm| was registered in the L2CAP
// |service_record|. If it was, also updates |registered_psm| with the
// registered value, as the requested PSM may have been left unspecified.
bool VerifyL2capService(const int* requested_psm,
                        BluetoothL2CAPPSM* registered_psm,
                        IOBluetoothSDPServiceRecord* service_record) {
  // Test whether the requested PSM was available.
  // TODO(isherman): The OS doesn't seem to actually pick a random PSM if we
  // pass in |kInvalidL2capPsm|.
  BluetoothL2CAPPSM l2cap_psm;
  IOReturn result = [service_record getL2CAPPSM:&l2cap_psm];
  if (result != kIOReturnSuccess ||
      (requested_psm && l2cap_psm != *requested_psm)) {
    return false;
  }

  *registered_psm = l2cap_psm;
  return true;
}

// Registers an L2CAP service with the specified |uuid|, |options.psm|, and
// |options.name| in the system SDP server. Automatically allocates a PSM if
// |options.psm| is null. Does not register a name if |options.name| is null.
// Returns a handle to the registered service and updates |registered_psm| to
// the actual PSM, or returns nil if the service could not be registered.
IOBluetoothSDPServiceRecord* RegisterL2capService(
    const BluetoothUUID& uuid,
    const BluetoothAdapter::ServiceOptions& options,
    BluetoothL2CAPPSM* registered_psm) {
  return RegisterService(
      BuildL2capServiceDefinition(uuid, options),
      base::Bind(&VerifyL2capService, options.psm.get(), registered_psm));
}

}  // namespace

// static
scoped_refptr<BluetoothSocketMac> BluetoothSocketMac::CreateSocket() {
  return make_scoped_refptr(new BluetoothSocketMac());
}

void BluetoothSocketMac::Connect(
    IOBluetoothDevice* device,
    const BluetoothUUID& uuid,
    const base::Closure& success_callback,
    const ErrorCompletionCallback& error_callback) {
  DCHECK(thread_checker_.CalledOnValidThread());

  uuid_ = uuid;

  // Perform an SDP query on the |device| to refresh the cache, in case the
  // services that the |device| advertises have changed since the previous
  // query.
  DVLOG(1) << BluetoothClassicDeviceMac::GetDeviceAddress(device) << " "
           << uuid_.canonical_value() << ": Sending SDP query.";
  SDPQueryListener* listener =
      [[SDPQueryListener alloc] initWithSocket:this
                                        device:device
                              success_callback:success_callback
                                error_callback:error_callback];
  [device performSDPQuery:[listener autorelease]
                    uuids:@[GetIOBluetoothSDPUUID(uuid_)]];
}

void BluetoothSocketMac::ListenUsingRfcomm(
    scoped_refptr<BluetoothAdapterMac> adapter,
    const BluetoothUUID& uuid,
    const BluetoothAdapter::ServiceOptions& options,
    const base::Closure& success_callback,
    const ErrorCompletionCallback& error_callback) {
  DCHECK(thread_checker_.CalledOnValidThread());

  adapter_ = adapter;
  uuid_ = uuid;

  DVLOG(1) << uuid_.canonical_value() << ": Registering RFCOMM service.";
  BluetoothRFCOMMChannelID registered_channel_id;
  service_record_.reset(
      RegisterRfcommService(uuid, options, &registered_channel_id));
  if (!service_record_.get()) {
    error_callback.Run(kInvalidOrUsedChannel);
    return;
  }

  rfcomm_connection_listener_.reset(
      [[BluetoothRfcommConnectionListener alloc]
          initWithSocket:this
               channelID:registered_channel_id]);

  success_callback.Run();
}

void BluetoothSocketMac::ListenUsingL2cap(
    scoped_refptr<BluetoothAdapterMac> adapter,
    const BluetoothUUID& uuid,
    const BluetoothAdapter::ServiceOptions& options,
    const base::Closure& success_callback,
    const ErrorCompletionCallback& error_callback) {
  DCHECK(thread_checker_.CalledOnValidThread());

  adapter_ = adapter;
  uuid_ = uuid;

  DVLOG(1) << uuid_.canonical_value() << ": Registering L2CAP service.";
  BluetoothL2CAPPSM registered_psm;
  service_record_.reset(RegisterL2capService(uuid, options, &registered_psm));
  if (!service_record_.get()) {
    error_callback.Run(kInvalidOrUsedPsm);
    return;
  }

  l2cap_connection_listener_.reset(
      [[BluetoothL2capConnectionListener alloc] initWithSocket:this
                                                           psm:registered_psm]);

  success_callback.Run();
}

void BluetoothSocketMac::OnSDPQueryComplete(
      IOReturn status,
      IOBluetoothDevice* device,
      const base::Closure& success_callback,
      const ErrorCompletionCallback& error_callback) {
  DCHECK(thread_checker_.CalledOnValidThread());
  DVLOG(1) << BluetoothClassicDeviceMac::GetDeviceAddress(device) << " "
           << uuid_.canonical_value() << ": SDP query complete.";

  if (status != kIOReturnSuccess) {
    error_callback.Run(kSDPQueryFailed);
    return;
  }

  IOBluetoothSDPServiceRecord* record = [device
      getServiceRecordForUUID:GetIOBluetoothSDPUUID(uuid_)];
  if (record == nil) {
    error_callback.Run(kProfileNotFound);
    return;
  }

  if (is_connecting()) {
    error_callback.Run(kSocketConnecting);
    return;
  }

  if (channel_) {
    error_callback.Run(kSocketAlreadyConnected);
    return;
  }

  // Since RFCOMM is built on top of L2CAP, a service record with both should
  // always be treated as RFCOMM.
  BluetoothRFCOMMChannelID rfcomm_channel_id = kInvalidRfcommChannelId;
  BluetoothL2CAPPSM l2cap_psm = kInvalidL2capPsm;
  status = [record getRFCOMMChannelID:&rfcomm_channel_id];
  if (status != kIOReturnSuccess) {
    status = [record getL2CAPPSM:&l2cap_psm];
    if (status != kIOReturnSuccess) {
      error_callback.Run(kProfileNotFound);
      return;
    }
  }

  if (rfcomm_channel_id != kInvalidRfcommChannelId) {
    DVLOG(1) << BluetoothClassicDeviceMac::GetDeviceAddress(device) << " "
             << uuid_.canonical_value()
             << ": Opening RFCOMM channel: " << rfcomm_channel_id;
  } else {
    DCHECK_NE(l2cap_psm, kInvalidL2capPsm);
    DVLOG(1) << BluetoothClassicDeviceMac::GetDeviceAddress(device) << " "
             << uuid_.canonical_value()
             << ": Opening L2CAP channel: " << l2cap_psm;
  }

  // Note: It's important to set the connect callbacks *prior* to opening the
  // channel, as opening the channel can synchronously call into
  // OnChannelOpenComplete().
  connect_callbacks_.reset(new ConnectCallbacks());
  connect_callbacks_->success_callback = success_callback;
  connect_callbacks_->error_callback = error_callback;

  if (rfcomm_channel_id != kInvalidRfcommChannelId) {
    channel_ = BluetoothRfcommChannelMac::OpenAsync(
        this, device, rfcomm_channel_id, &status);
  } else {
    DCHECK_NE(l2cap_psm, kInvalidL2capPsm);
    channel_ =
        BluetoothL2capChannelMac::OpenAsync(this, device, l2cap_psm, &status);
  }
  if (status != kIOReturnSuccess) {
    ReleaseChannel();
    std::stringstream error;
    error << "Failed to connect bluetooth socket ("
          << BluetoothClassicDeviceMac::GetDeviceAddress(device) << "): ("
          << status << ")";
    error_callback.Run(error.str());
    return;
  }

  DVLOG(1) << BluetoothClassicDeviceMac::GetDeviceAddress(device) << " "
           << uuid_.canonical_value() << ": channel opening in background.";
}

void BluetoothSocketMac::OnChannelOpened(
    std::unique_ptr<BluetoothChannelMac> channel) {
  DCHECK(thread_checker_.CalledOnValidThread());
  DVLOG(1) << uuid_.canonical_value() << ": Incoming channel pending.";

  accept_queue_.push(linked_ptr<BluetoothChannelMac>(channel.release()));
  if (accept_request_)
    AcceptConnectionRequest();

  // TODO(isherman): Currently, the socket remains alive even after the app that
  // requested it is closed. That's not great, as a misbehaving app could
  // saturate all of the system's RFCOMM channels, and then they would not be
  // freed until the user restarts Chrome.  http://crbug.com/367316
  // TODO(isherman): Likewise, the socket currently remains alive even if the
  // underlying channel is closed, e.g. via the client disconnecting, or the
  // user closing the Bluetooth connection via the system menu. This functions
  // essentially as a minor memory leak.  http://crbug.com/367319
}

void BluetoothSocketMac::OnChannelOpenComplete(
    const std::string& device_address,
    IOReturn status) {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK(is_connecting());

  DVLOG(1) << device_address << " " << uuid_.canonical_value()
           << ": channel open complete.";

  std::unique_ptr<ConnectCallbacks> temp = std::move(connect_callbacks_);
  if (status != kIOReturnSuccess) {
    ReleaseChannel();
    std::stringstream error;
    error << "Failed to connect bluetooth socket (" << device_address << "): ("
          << status << ")";
    temp->error_callback.Run(error.str());
    return;
  }

  temp->success_callback.Run();
}

void BluetoothSocketMac::Close() {
  DCHECK(thread_checker_.CalledOnValidThread());

  if (channel_)
    ReleaseChannel();
  else if (service_record_.get())
    ReleaseListener();
}

void BluetoothSocketMac::Disconnect(const base::Closure& callback) {
  DCHECK(thread_checker_.CalledOnValidThread());

  Close();
  callback.Run();
}

void BluetoothSocketMac::Receive(
    int /* buffer_size */,
    const ReceiveCompletionCallback& success_callback,
    const ReceiveErrorCompletionCallback& error_callback) {
  DCHECK(thread_checker_.CalledOnValidThread());

  if (is_connecting()) {
    error_callback.Run(BluetoothSocket::kSystemError, kSocketConnecting);
    return;
  }

  if (!channel_) {
    error_callback.Run(BluetoothSocket::kDisconnected, kSocketNotConnected);
    return;
  }

  // Only one pending read at a time
  if (receive_callbacks_) {
    error_callback.Run(BluetoothSocket::kIOPending, kReceivePending);
    return;
  }

  // If there is at least one packet, consume it and succeed right away.
  if (!receive_queue_.empty()) {
    scoped_refptr<net::IOBufferWithSize> buffer = receive_queue_.front();
    receive_queue_.pop();
    success_callback.Run(buffer->size(), buffer);
    return;
  }

  // Set the receive callback to use when data is received.
  receive_callbacks_.reset(new ReceiveCallbacks());
  receive_callbacks_->success_callback = success_callback;
  receive_callbacks_->error_callback = error_callback;
}

void BluetoothSocketMac::OnChannelDataReceived(void* data, size_t length) {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK(!is_connecting());

  int data_size = base::checked_cast<int>(length);
  scoped_refptr<net::IOBufferWithSize> buffer(
      new net::IOBufferWithSize(data_size));
  memcpy(buffer->data(), data, buffer->size());

  // If there is a pending read callback, call it now.
  if (receive_callbacks_) {
    std::unique_ptr<ReceiveCallbacks> temp = std::move(receive_callbacks_);
    temp->success_callback.Run(buffer->size(), buffer);
    return;
  }

  // Otherwise, enqueue the buffer for later use
  receive_queue_.push(buffer);
}

void BluetoothSocketMac::Send(scoped_refptr<net::IOBuffer> buffer,
                              int buffer_size,
                              const SendCompletionCallback& success_callback,
                              const ErrorCompletionCallback& error_callback) {
  DCHECK(thread_checker_.CalledOnValidThread());

  if (is_connecting()) {
    error_callback.Run(kSocketConnecting);
    return;
  }

  if (!channel_) {
    error_callback.Run(kSocketNotConnected);
    return;
  }

  // Create and enqueue request in preparation of async writes.
  linked_ptr<SendRequest> request(new SendRequest());
  request->buffer_size = buffer_size;
  request->success_callback = success_callback;
  request->error_callback = error_callback;
  send_queue_.push(request);

  // |writeAsync| accepts buffers of max. mtu bytes per call, so we need to emit
  // multiple write operations if buffer_size > mtu.
  uint16_t mtu = channel_->GetOutgoingMTU();
  scoped_refptr<net::DrainableIOBuffer> send_buffer(
      new net::DrainableIOBuffer(buffer.get(), buffer_size));
  while (send_buffer->BytesRemaining() > 0) {
    int byte_count = send_buffer->BytesRemaining();
    if (byte_count > mtu)
      byte_count = mtu;
    IOReturn status =
        channel_->WriteAsync(send_buffer->data(), byte_count, request.get());

    if (status != kIOReturnSuccess) {
      std::stringstream error;
      error << "Failed to connect bluetooth socket ("
            << channel_->GetDeviceAddress() << "): (" << status << ")";
      // Remember the first error only
      if (request->status == kIOReturnSuccess)
        request->status = status;
      request->error_signaled = true;
      request->error_callback.Run(error.str());
      // We may have failed to issue any write operation. In that case, there
      // will be no corresponding completion callback for this particular
      // request, so we must forget about it now.
      if (request->active_async_writes == 0) {
        send_queue_.pop();
      }
      return;
    }

    request->active_async_writes++;
    send_buffer->DidConsume(byte_count);
  }
}

void BluetoothSocketMac::OnChannelWriteComplete(void* refcon, IOReturn status) {
  DCHECK(thread_checker_.CalledOnValidThread());

  // Note: We use "CHECK" below to ensure we never run into unforeseen
  // occurrences of asynchronous callbacks, which could lead to data
  // corruption.
  CHECK_EQ(static_cast<SendRequest*>(refcon), send_queue_.front().get());

  // Keep a local linked_ptr to avoid releasing the request too early if we end
  // up removing it from the queue.
  linked_ptr<SendRequest> request = send_queue_.front();

  // Remember the first error only
  if (status != kIOReturnSuccess) {
    if (request->status == kIOReturnSuccess)
      request->status = status;
  }

  // Figure out if we are done with this async request
  request->active_async_writes--;
  if (request->active_async_writes > 0)
    return;

  // If this was the last active async write for this request, remove it from
  // the queue and call the appropriate callback associated to the request.
  send_queue_.pop();
  if (request->status != kIOReturnSuccess) {
    if (!request->error_signaled) {
      std::stringstream error;
      error << "Failed to connect bluetooth socket ("
            << channel_->GetDeviceAddress() << "): (" << status << ")";
      request->error_signaled = true;
      request->error_callback.Run(error.str());
    }
  } else {
    request->success_callback.Run(request->buffer_size);
  }
}

void BluetoothSocketMac::OnChannelClosed() {
  DCHECK(thread_checker_.CalledOnValidThread());

  if (receive_callbacks_) {
    std::unique_ptr<ReceiveCallbacks> temp = std::move(receive_callbacks_);
    temp->error_callback.Run(BluetoothSocket::kDisconnected,
                             kSocketNotConnected);
  }

  ReleaseChannel();
}

void BluetoothSocketMac::Accept(
    const AcceptCompletionCallback& success_callback,
    const ErrorCompletionCallback& error_callback) {
  DCHECK(thread_checker_.CalledOnValidThread());

  // Allow only one pending accept at a time.
  if (accept_request_) {
    error_callback.Run(net::ErrorToString(net::ERR_IO_PENDING));
    return;
  }

  accept_request_.reset(new AcceptRequest);
  accept_request_->success_callback = success_callback;
  accept_request_->error_callback = error_callback;

  if (accept_queue_.size() >= 1)
    AcceptConnectionRequest();
}

void BluetoothSocketMac::AcceptConnectionRequest() {
  DCHECK(thread_checker_.CalledOnValidThread());
  DVLOG(1) << uuid_.canonical_value() << ": Accepting pending connection.";

  linked_ptr<BluetoothChannelMac> channel = accept_queue_.front();
  accept_queue_.pop();

  adapter_->DeviceConnected(channel->GetDevice());
  BluetoothDevice* device = adapter_->GetDevice(channel->GetDeviceAddress());
  DCHECK(device);

  scoped_refptr<BluetoothSocketMac> client_socket =
      BluetoothSocketMac::CreateSocket();

  client_socket->uuid_ = uuid_;
  client_socket->channel_.reset(channel.release());

  // Associating the socket can synchronously call into OnChannelOpenComplete().
  // Make sure to first set the new socket to be connecting and hook it up to
  // run the accept callback with the device object.
  client_socket->connect_callbacks_.reset(new ConnectCallbacks());
  client_socket->connect_callbacks_->success_callback =
      base::Bind(accept_request_->success_callback, device, client_socket);
  client_socket->connect_callbacks_->error_callback =
      accept_request_->error_callback;
  accept_request_.reset();

  // Now it's safe to associate the socket with the channel.
  client_socket->channel_->SetSocket(client_socket.get());

  DVLOG(1) << uuid_.canonical_value() << ": Accept complete.";
}

BluetoothSocketMac::AcceptRequest::AcceptRequest() {}

BluetoothSocketMac::AcceptRequest::~AcceptRequest() {}

BluetoothSocketMac::SendRequest::SendRequest()
    : status(kIOReturnSuccess), active_async_writes(0), error_signaled(false) {}

BluetoothSocketMac::SendRequest::~SendRequest() {}

BluetoothSocketMac::ReceiveCallbacks::ReceiveCallbacks() {}

BluetoothSocketMac::ReceiveCallbacks::~ReceiveCallbacks() {}

BluetoothSocketMac::ConnectCallbacks::ConnectCallbacks() {}

BluetoothSocketMac::ConnectCallbacks::~ConnectCallbacks() {}

BluetoothSocketMac::BluetoothSocketMac() {}

BluetoothSocketMac::~BluetoothSocketMac() {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK(!channel_);
  DCHECK(!rfcomm_connection_listener_);
}

void BluetoothSocketMac::ReleaseChannel() {
  DCHECK(thread_checker_.CalledOnValidThread());
  channel_.reset();

  // Closing the channel above prevents the callback delegate from being called
  // so it is now safe to release all callback state.
  connect_callbacks_.reset();
  receive_callbacks_.reset();
  empty_queue(receive_queue_);
  empty_queue(send_queue_);
}

void BluetoothSocketMac::ReleaseListener() {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK(service_record_.get());

  [service_record_ removeServiceRecord];
  service_record_.reset();
  rfcomm_connection_listener_.reset();
  l2cap_connection_listener_.reset();

  // Destroying the listener above prevents the callback delegate from being
  // called so it is now safe to release all callback state.
  accept_request_.reset();
  empty_queue(accept_queue_);
}

}  // namespace device
