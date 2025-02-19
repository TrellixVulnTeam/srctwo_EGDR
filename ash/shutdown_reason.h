// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_SHUTDOWN_REASON_H_
#define ASH_SHUTDOWN_REASON_H_

namespace ash {

enum class ShutdownReason {
  UNKNOWN,                 // Reason unknown or not applicable.
  POWER_BUTTON,            // User pressed the (physical) power button.
  LOGIN_SHUT_DOWN_BUTTON,  // User pressed the login screen shut down button.
  TRAY_SHUT_DOWN_BUTTON,   // User pressed the tray shut down button.
};

}  // namespace ash

#endif  // ASH_SHUTDOWN_REASON_H_
