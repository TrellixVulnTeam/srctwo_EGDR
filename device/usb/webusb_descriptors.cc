// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/usb/webusb_descriptors.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/logging.h"
#include "base/stl_util.h"
#include "components/device_event_log/device_event_log.h"
#include "device/usb/usb_device_handle.h"
#include "net/base/io_buffer.h"

using net::IOBufferWithSize;

namespace device {

namespace {

// These constants are defined by the Universal Serial Device 3.0 Specification
// Revision 1.0.
const uint8_t kGetDescriptorRequest = 0x06;

const uint8_t kBosDescriptorType = 0x0F;
const uint8_t kDeviceCapabilityDescriptorType = 0x10;

const uint8_t kPlatformDevCapabilityType = 0x05;

// These constants are defined by the WebUSB specification:
// http://wicg.github.io/webusb/
const uint8_t kGetUrlRequest = 0x02;

const uint8_t kWebUsbCapabilityUUID[16] = {
    // Little-endian encoding of {3408b638-09a9-47a0-8bfd-a0768815b665}.
    0x38, 0xB6, 0x08, 0x34, 0xA9, 0x09, 0xA0, 0x47,
    0x8B, 0xFD, 0xA0, 0x76, 0x88, 0x15, 0xB6, 0x65};

const int kControlTransferTimeoutMs = 2000;  // 2 seconds

using ReadWebUsbDescriptorsCallback =
    base::Callback<void(const GURL& landing_page)>;

void OnReadLandingPage(uint8_t landing_page_id,
                       const ReadWebUsbDescriptorsCallback& callback,
                       UsbTransferStatus status,
                       scoped_refptr<net::IOBuffer> buffer,
                       size_t length) {
  if (status != UsbTransferStatus::COMPLETED) {
    USB_LOG(EVENT) << "Failed to read WebUSB URL descriptor: "
                   << static_cast<int>(landing_page_id);
    callback.Run(GURL());
    return;
  }

  GURL url;
  ParseWebUsbUrlDescriptor(
      std::vector<uint8_t>(buffer->data(), buffer->data() + length), &url);
  callback.Run(url);
}

void ReadLandingPage(uint8_t vendor_code,
                     uint8_t landing_page_id,
                     scoped_refptr<UsbDeviceHandle> device_handle,
                     const ReadWebUsbDescriptorsCallback& callback) {
  auto buffer = base::MakeRefCounted<IOBufferWithSize>(255);
  device_handle->ControlTransfer(
      UsbTransferDirection::INBOUND, UsbControlTransferType::VENDOR,
      UsbControlTransferRecipient::DEVICE, vendor_code, landing_page_id,
      kGetUrlRequest, buffer, buffer->size(), kControlTransferTimeoutMs,
      base::Bind(&OnReadLandingPage, landing_page_id, callback));
}

void OnReadBosDescriptor(scoped_refptr<UsbDeviceHandle> device_handle,
                         const ReadWebUsbDescriptorsCallback& callback,
                         UsbTransferStatus status,
                         scoped_refptr<net::IOBuffer> buffer,
                         size_t length) {
  if (status != UsbTransferStatus::COMPLETED) {
    USB_LOG(EVENT) << "Failed to read BOS descriptor.";
    callback.Run(GURL());
    return;
  }

  WebUsbPlatformCapabilityDescriptor descriptor;
  if (!descriptor.ParseFromBosDescriptor(
          std::vector<uint8_t>(buffer->data(), buffer->data() + length))) {
    callback.Run(GURL());
    return;
  }

  if (descriptor.landing_page_id) {
    ReadLandingPage(descriptor.vendor_code, descriptor.landing_page_id,
                    device_handle, callback);
  } else {
    callback.Run(GURL());
  }
}

void OnReadBosDescriptorHeader(scoped_refptr<UsbDeviceHandle> device_handle,
                               const ReadWebUsbDescriptorsCallback& callback,
                               UsbTransferStatus status,
                               scoped_refptr<net::IOBuffer> buffer,
                               size_t length) {
  if (status != UsbTransferStatus::COMPLETED || length != 5) {
    USB_LOG(EVENT) << "Failed to read BOS descriptor header.";
    callback.Run(GURL());
    return;
  }

  const uint8_t* data = reinterpret_cast<uint8_t*>(buffer->data());
  uint16_t new_length = data[2] | (data[3] << 8);
  scoped_refptr<IOBufferWithSize> new_buffer = new IOBufferWithSize(new_length);
  device_handle->ControlTransfer(
      UsbTransferDirection::INBOUND, UsbControlTransferType::STANDARD,
      UsbControlTransferRecipient::DEVICE, kGetDescriptorRequest,
      kBosDescriptorType << 8, 0, new_buffer, new_buffer->size(),
      kControlTransferTimeoutMs,
      base::Bind(&OnReadBosDescriptor, device_handle, callback));
}

}  // namespace

WebUsbPlatformCapabilityDescriptor::WebUsbPlatformCapabilityDescriptor()
    : version(0), vendor_code(0) {}

WebUsbPlatformCapabilityDescriptor::~WebUsbPlatformCapabilityDescriptor() {}

bool WebUsbPlatformCapabilityDescriptor::ParseFromBosDescriptor(
    const std::vector<uint8_t>& bytes) {
  if (bytes.size() < 5) {
    // Too short for the BOS descriptor header.
    return false;
  }

  // Validate the BOS descriptor, defined in Table 9-12 of the Universal Serial
  // Bus 3.1 Specification, Revision 1.0.
  uint16_t total_length = bytes[2] + (bytes[3] << 8);
  if (bytes[0] != 5 ||                                    // bLength
      bytes[1] != kBosDescriptorType ||                   // bDescriptorType
      5 > total_length || total_length > bytes.size()) {  // wTotalLength
    return false;
  }

  uint8_t num_device_caps = bytes[4];
  std::vector<uint8_t>::const_iterator it = bytes.begin();
  std::vector<uint8_t>::const_iterator end = it + total_length;
  std::advance(it, 5);

  uint8_t length = 0;
  for (size_t i = 0; i < num_device_caps; ++i, std::advance(it, length)) {
    if (it == end) {
      return false;
    }

    // Validate the Device Capability descriptor, defined in Table 9-13 of the
    // Universal Serial Bus 3.1 Specification, Revision 1.0.
    length = it[0];
    if (length < 3 || std::distance(it, end) < length ||  // bLength
        it[1] != kDeviceCapabilityDescriptorType) {       // bDescriptorType
      return false;
    }

    if (it[2] != kPlatformDevCapabilityType) {  // bDevCapabilityType
      continue;
    }

    // Validate the Platform Capability Descriptor, defined in Table 9-18 of the
    // Universal Serial Bus 3.1 Specification, Revision 1.0.
    if (length < 20) {
      // Platform capability descriptors must be at least 20 bytes.
      return false;
    }

    if (memcmp(&it[4], kWebUsbCapabilityUUID, sizeof(kWebUsbCapabilityUUID)) !=
        0) {  // PlatformCapabilityUUID
      continue;
    }

    if (length < 22) {
      // The WebUSB capability descriptor must be at least 22 bytes (to allow
      // for future versions).
      return false;
    }

    version = it[20] + (it[21] << 8);  // bcdVersion
    if (version < 0x0100) {
      continue;
    }

    // Version 1.0 defines two fields for a total length of 24 bytes.
    if (length != 24) {
      return false;
    }

    vendor_code = it[22];
    landing_page_id = it[23];
    return true;
  }

  return false;
}

// Parses a WebUSB URL Descriptor:
// http://wicg.github.io/webusb/#dfn-url-descriptor
//
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |     length    |     type      |    prefix     |    data[0]    |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |     data[1]   |      ...
// +-+-+-+-+-+-+-+-+-+-+-+------
bool ParseWebUsbUrlDescriptor(const std::vector<uint8_t>& bytes, GURL* output) {
  const uint8_t kDescriptorType = 0x03;
  const uint8_t kDescriptorMinLength = 3;

  if (bytes.size() < kDescriptorMinLength) {
    return false;
  }

  // Validate that the length is consistent and fits within the buffer.
  uint8_t length = bytes[0];
  if (length < kDescriptorMinLength || length > bytes.size() ||
      bytes[1] != kDescriptorType) {
    return false;
  }

  // Look up the URL prefix and append the rest of the data in the descriptor.
  std::string url;
  switch (bytes[2]) {
    case 0:
      url.append("http://");
      break;
    case 1:
      url.append("https://");
      break;
    default:
      return false;
  }
  url.append(reinterpret_cast<const char*>(bytes.data() + 3), length - 3);

  *output = GURL(url);
  if (!output->is_valid()) {
    return false;
  }

  return true;
}

void ReadWebUsbDescriptors(scoped_refptr<UsbDeviceHandle> device_handle,
                           const ReadWebUsbDescriptorsCallback& callback) {
  auto buffer = base::MakeRefCounted<IOBufferWithSize>(5);
  device_handle->ControlTransfer(
      UsbTransferDirection::INBOUND, UsbControlTransferType::STANDARD,
      UsbControlTransferRecipient::DEVICE, kGetDescriptorRequest,
      kBosDescriptorType << 8, 0, buffer, buffer->size(),
      kControlTransferTimeoutMs,
      base::Bind(&OnReadBosDescriptorHeader, device_handle, callback));
}

}  // namespace device
