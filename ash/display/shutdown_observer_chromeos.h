// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_DISPLAY_SHUTDOWN_OBSERVER_CHROMEOS_H_
#define ASH_DISPLAY_SHUTDOWN_OBSERVER_CHROMEOS_H_

#include "ash/session/session_observer.h"
#include "base/macros.h"

namespace display {
class DisplayConfigurator;
}

namespace ash {

// Adds self as SessionObserver and listens for OnChromeTerminating() on
// behalf of |display_configurator_|.
class ShutdownObserver : public SessionObserver {
 public:
  explicit ShutdownObserver(display::DisplayConfigurator* display_configurator);
  ~ShutdownObserver() override;

 private:
  // SessionObserver:
  void OnChromeTerminating() override;

  display::DisplayConfigurator* const display_configurator_;
  ScopedSessionObserver scoped_session_observer_;

  DISALLOW_COPY_AND_ASSIGN(ShutdownObserver);
};

}  // namespace ash

#endif  // ASH_DISPLAY_SHUTDOWN_OBSERVER_CHROMEOS_H_
