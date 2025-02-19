// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/idle_detector.h"

#include "base/bind.h"
#include "base/logging.h"
#include "ui/base/user_activity/user_activity_detector.h"

namespace chromeos {

IdleDetector::IdleDetector(const base::Closure& on_idle_callback)
    : idle_callback_(on_idle_callback) {}

IdleDetector::~IdleDetector() {
  ui::UserActivityDetector* user_activity_detector =
      ui::UserActivityDetector::Get();
  if (user_activity_detector && user_activity_detector->HasObserver(this))
    user_activity_detector->RemoveObserver(this);
}

void IdleDetector::OnUserActivity(const ui::Event* event) {
  ResetTimer();
}

void IdleDetector::Start(const base::TimeDelta& timeout) {
  timeout_ = timeout;
  if (!ui::UserActivityDetector::Get()->HasObserver(this))
    ui::UserActivityDetector::Get()->AddObserver(this);
  ResetTimer();
}

void IdleDetector::ResetTimer() {
  if (timer_.IsRunning())
    timer_.Reset();
  else
    timer_.Start(FROM_HERE, timeout_, idle_callback_);
}

}  // namespace chromeos
