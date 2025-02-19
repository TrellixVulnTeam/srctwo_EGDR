// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/signin/easy_unlock_metrics.h"

#include "base/logging.h"
#include "base/metrics/histogram_macros.h"

void RecordEasyUnlockDidUserManuallyUnlockPhone(bool did_unlock) {
  UMA_HISTOGRAM_BOOLEAN("EasyUnlock.AuthEvent.DidUserManuallyUnlockPhone",
                        did_unlock);
}

void RecordEasyUnlockSigninDuration(const base::TimeDelta& duration) {
  UMA_HISTOGRAM_MEDIUM_TIMES("EasyUnlock.AuthEvent.SignIn.Duration", duration);
}

void RecordEasyUnlockSigninEvent(EasyUnlockAuthEvent event) {
  DCHECK_LT(event, EASY_UNLOCK_AUTH_EVENT_COUNT);
  UMA_HISTOGRAM_ENUMERATION("EasyUnlock.AuthEvent.SignIn", event,
                            EASY_UNLOCK_AUTH_EVENT_COUNT);
}

void RecordEasyUnlockScreenUnlockDuration(const base::TimeDelta& duration) {
  UMA_HISTOGRAM_MEDIUM_TIMES("EasyUnlock.AuthEvent.Unlock.Duration", duration);
}

void RecordEasyUnlockScreenUnlockEvent(EasyUnlockAuthEvent event) {
  DCHECK_LT(event, EASY_UNLOCK_AUTH_EVENT_COUNT);
  UMA_HISTOGRAM_ENUMERATION("EasyUnlock.AuthEvent.Unlock", event,
                            EASY_UNLOCK_AUTH_EVENT_COUNT);
}

void RecordEasyUnlockTrialRunEvent(EasyUnlockTrialRunEvent event) {
  DCHECK_LT(event, EASY_UNLOCK_TRIAL_RUN_EVENT_COUNT);
  UMA_HISTOGRAM_ENUMERATION("EasyUnlock.TrialRun.Events",
                            event,
                            EASY_UNLOCK_TRIAL_RUN_EVENT_COUNT);
}
