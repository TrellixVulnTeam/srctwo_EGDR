// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/public/cpp/shell_window_ids.h"
#include "base/auto_reset.h"
#include "chrome/browser/chromeos/bluetooth/bluetooth_pairing_dialog.h"
#include "chrome/test/base/web_ui_browser_test.h"
#include "content/public/browser/web_ui.h"
#include "content/public/test/browser_test_utils.h"
#include "device/bluetooth/bluetooth_adapter_factory.h"
#include "device/bluetooth/test/mock_bluetooth_adapter.h"
#include "device/bluetooth/test/mock_bluetooth_device.h"
#include "extensions/browser/extension_function.h"
#include "testing/gmock/include/gmock/gmock.h"

class BluetoothPairingUITest : public WebUIBrowserTest {
 public:
  BluetoothPairingUITest();
  ~BluetoothPairingUITest() override;

  void ShowDialog();

 private:
  scoped_refptr<testing::NiceMock<device::MockBluetoothAdapter>> mock_adapter_;
  std::unique_ptr<device::MockBluetoothDevice> mock_device_;
};

BluetoothPairingUITest::BluetoothPairingUITest() {}

BluetoothPairingUITest::~BluetoothPairingUITest() {}

void BluetoothPairingUITest::ShowDialog() {
  // Since we use mocks, callbacks are never called for the bluetooth API
  // functions. :(
  base::AutoReset<bool> ignore_did_respond(
      &ExtensionFunction::ignore_all_did_respond_for_testing_do_not_use, true);
  mock_adapter_ = new testing::NiceMock<device::MockBluetoothAdapter>();
  device::BluetoothAdapterFactory::SetAdapterForTesting(mock_adapter_);
  EXPECT_CALL(*mock_adapter_, IsPresent())
      .WillRepeatedly(testing::Return(true));
  EXPECT_CALL(*mock_adapter_, IsPowered())
      .WillRepeatedly(testing::Return(true));

  const bool kNotPaired = false;
  const bool kNotConnected = false;
  mock_device_.reset(
      new testing::NiceMock<device::MockBluetoothDevice>(
          nullptr,
          0,
          "Bluetooth 2.0 Mouse",
          "28:CF:DA:00:00:00",
          kNotPaired,
          kNotConnected));

  EXPECT_CALL(*mock_adapter_, GetDevice(testing::_))
      .WillOnce(testing::Return(mock_device_.get()));

  chromeos::BluetoothPairingDialog* dialog =
      new chromeos::BluetoothPairingDialog(
          mock_device_->GetAddress(), mock_device_->GetNameForDisplay(),
          mock_device_->IsPaired(), mock_device_->IsConnected());
  dialog->ShowInContainer(ash::kShellWindowId_SystemModalContainer);

  content::WebUI* webui = dialog->GetWebUIForTest();
  content::WebContents* webui_webcontents = webui->GetWebContents();
  content::WaitForLoadStop(webui_webcontents);
  SetWebUIInstance(webui);
}
