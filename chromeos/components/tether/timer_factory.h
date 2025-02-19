// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_COMPONENTS_TETHER_TIMER_FACTORY_H_
#define CHROMEOS_COMPONENTS_TETHER_TIMER_FACTORY_H_

#include <memory>

#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/timer/timer.h"

namespace chromeos {

namespace tether {

// Serves as a simple Timer creator, injected into classes that use Timers.
// Is intended to be overriden during testing in order to stub or mock the
// Timers used by the object under test.
class TimerFactory {
 public:
  virtual ~TimerFactory();

  virtual std::unique_ptr<base::Timer> CreateOneShotTimer();
};

}  // namespace tether

}  // namespace chromeos

#endif  // CHROMEOS_COMPONENTS_TETHER_TIMER_FACTORY_H_
