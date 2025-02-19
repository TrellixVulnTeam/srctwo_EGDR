// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_LOGIN_SCREENS_MOCK_DEVICE_DISABLED_SCREEN_VIEW_H_
#define CHROME_BROWSER_CHROMEOS_LOGIN_SCREENS_MOCK_DEVICE_DISABLED_SCREEN_VIEW_H_

#include "chrome/browser/chromeos/login/screens/device_disabled_screen_view.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace chromeos {

class MockDeviceDisabledScreenView : public DeviceDisabledScreenView {
 public:
  MockDeviceDisabledScreenView();
  ~MockDeviceDisabledScreenView() override;

  void SetDelegate(Delegate* delegate) override;

  MOCK_METHOD0(Show, void());
  MOCK_METHOD0(Hide, void());
  MOCK_METHOD1(UpdateMessage, void(const std::string& message));

 private:
  MOCK_METHOD1(MockSetDelegate, void(Delegate* delegate));

  Delegate* delegate_;
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_LOGIN_SCREENS_MOCK_DEVICE_DISABLED_SCREEN_VIEW_H_
