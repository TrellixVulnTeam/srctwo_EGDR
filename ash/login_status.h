// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_LOGIN_STATUS_H_
#define ASH_LOGIN_STATUS_H_

namespace ash {

enum class LoginStatus {
  NOT_LOGGED_IN,  // Not logged in
  LOCKED,         // A user has locked the screen
  USER,           // A regular user is logged in
  OWNER,          // The owner of the device is logged in
  GUEST,          // A guest is logged in (i.e. incognito)
  PUBLIC,         // A public account is logged in
  SUPERVISED,     // A supervised user is logged in
  KIOSK_APP,      // Is in kiosk app mode
  ARC_KIOSK_APP   // Is in ARC kiosk mode
};

}  // namespace ash

#endif  // ASH_LOGIN_STATUS_H_
