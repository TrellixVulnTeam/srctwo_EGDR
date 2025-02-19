// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_BLUETOOTH_FRAME_CONNECTED_BLUETOOTH_DEVICES_H_
#define CONTENT_BROWSER_BLUETOOTH_FRAME_CONNECTED_BLUETOOTH_DEVICES_H_

#include <memory>
#include <string>
#include <unordered_map>

#include "base/optional.h"
#include "content/common/bluetooth/web_bluetooth_device_id.h"
#include "content/common/content_export.h"
#include "third_party/WebKit/public/platform/modules/bluetooth/web_bluetooth.mojom.h"
#include "url/origin.h"

namespace device {
class BluetoothGattConnection;
}  // namespace device

namespace content {

struct GATTConnectionAndServerClient;
class RenderFrameHost;
class WebContentsImpl;

// Holds information about connected devices and updates the WebContents
// when new connections are made or connections closed. WebContents must
// outlive this class.
// This class does not keep track of the status of the connections. Owners of
// this class should inform it when a connection is terminated so that the
// connection is removed from the appropriate maps.
class CONTENT_EXPORT FrameConnectedBluetoothDevices final {
 public:
  // |rfh| should be the RenderFrameHost that owns the WebBluetoothServiceImpl
  // that owns this map.
  explicit FrameConnectedBluetoothDevices(RenderFrameHost* rfh);
  ~FrameConnectedBluetoothDevices();

  // Returns true if the map holds a connection to |device_id|.
  bool IsConnectedToDeviceWithId(const WebBluetoothDeviceId& device_id);

  // If a connection doesn't exist already for |device_id|, adds a connection to
  // the map and increases the WebContents count of connected devices.
  void Insert(const WebBluetoothDeviceId& device_id,
              std::unique_ptr<device::BluetoothGattConnection> connection,
              blink::mojom::WebBluetoothServerClientAssociatedPtr client);

  // Deletes the BluetoothGattConnection for |device_id| and decrements the
  // WebContents count of connected devices if |device_id| had a connection.
  void CloseConnectionToDeviceWithId(const WebBluetoothDeviceId& device_id);

  // Deletes the BluetoothGattConnection for |device_address| and decrements the
  // WebContents count of connected devices if |device_address| had a
  // connection. Returns the device_id of the device associated with the
  // connection.
  base::Optional<WebBluetoothDeviceId> CloseConnectionToDeviceWithAddress(
      const std::string& device_address);

 private:
  // Increments the Connected Devices count of the frame's WebContents.
  void IncrementDevicesConnectedCount();
  // Decrements the Connected Devices count of the frame's WebContents.
  void DecrementDevicesConnectedCount();

  // WebContentsImpl that owns the WebBluetoothServiceImpl that owns this map.
  WebContentsImpl* web_contents_impl_;

  // Keeps the BluetoothGattConnection objects alive so that connections don't
  // get closed.
  std::unordered_map<WebBluetoothDeviceId,
                     std::unique_ptr<GATTConnectionAndServerClient>,
                     WebBluetoothDeviceIdHash>
      device_id_to_connection_map_;

  // Keeps track of which device addresses correspond to which ids.
  std::unordered_map<std::string, WebBluetoothDeviceId>
      device_address_to_id_map_;

  DISALLOW_COPY_AND_ASSIGN(FrameConnectedBluetoothDevices);
};

}  // namespace content

#endif  // CONTENT_BROWSER_BLUETOOTH_FRAME_CONNECTED_BLUETOOTH_DEVICES_H_
