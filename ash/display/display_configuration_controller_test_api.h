// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_DISPLAY_DISPLAY_CONFIGURATION_CONTROLLER_TEST_API_H_
#define ASH_DISPLAY_DISPLAY_CONFIGURATION_CONTROLLER_TEST_API_H_

#include <stdint.h>

#include "base/macros.h"

namespace ash {
class DisplayConfigurationController;
class ScreenRotationAnimator;

// Accesses private data from a DisplayConfigurationController for testing.
class DisplayConfigurationControllerTestApi {
 public:
  explicit DisplayConfigurationControllerTestApi(
      DisplayConfigurationController* controller);

  // Wrapper functions for DisplayConfigurationController.
  void DisableDisplayAnimator();
  ScreenRotationAnimator* GetScreenRotationAnimatorForDisplay(
      int64_t display_id);

 private:
  DisplayConfigurationController* controller_;

  DISALLOW_COPY_AND_ASSIGN(DisplayConfigurationControllerTestApi);
};

}  // namespace ash

#endif  // ASH_DISPLAY_DISPLAY_CONFIGURATION_CONTROLLER_TEST_API_H_
