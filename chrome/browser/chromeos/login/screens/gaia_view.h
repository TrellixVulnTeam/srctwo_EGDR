// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_LOGIN_SCREENS_GAIA_VIEW_H_
#define CHROME_BROWSER_CHROMEOS_LOGIN_SCREENS_GAIA_VIEW_H_

#include <string>

#include "base/bind.h"
#include "base/compiler_specific.h"
#include "base/macros.h"
#include "chrome/browser/chromeos/login/oobe_screen.h"

namespace chromeos {

class GaiaView {
 public:
  constexpr static OobeScreen kScreenId = OobeScreen::SCREEN_GAIA_SIGNIN;

  GaiaView() = default;
  virtual ~GaiaView() = default;

  // Decides whether an auth extension should be pre-loaded. If it should,
  // pre-loads it.
  virtual void MaybePreloadAuthExtension() = 0;

  virtual void DisableRestrictiveProxyCheckForTest() = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(GaiaView);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_LOGIN_SCREENS_GAIA_VIEW_H_
