// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/login/quick_unlock/fingerprint_storage.h"

#include "chrome/browser/chromeos/login/quick_unlock/quick_unlock_utils.h"
#include "chrome/common/pref_names.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"

namespace chromeos {
namespace quick_unlock {

// static
void FingerprintStorage::RegisterProfilePrefs(PrefRegistrySimple* registry) {
  registry->RegisterIntegerPref(prefs::kQuickUnlockFingerprintRecord, 0);
}

FingerprintStorage::FingerprintStorage(PrefService* pref_service)
    : pref_service_(pref_service) {}

FingerprintStorage::~FingerprintStorage() {}

bool FingerprintStorage::IsFingerprintAuthenticationAvailable() const {
  return !ExceededUnlockAttempts() && IsFingerprintEnabled() && HasRecord();
}

bool FingerprintStorage::HasRecord() const {
  return pref_service_->GetInteger(prefs::kQuickUnlockFingerprintRecord) != 0;
}

void FingerprintStorage::AddUnlockAttempt() {
  ++unlock_attempt_count_;
}

void FingerprintStorage::ResetUnlockAttemptCount() {
  unlock_attempt_count_ = 0;
}

bool FingerprintStorage::ExceededUnlockAttempts() const {
  return unlock_attempt_count() >= kMaximumUnlockAttempts;
}

}  // namespace quick_unlock
}  // namespace chromeos
