// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_PREFERENCES_PUBLIC_CPP_TRACKED_MOCK_VALIDATION_DELEGATE_H_
#define SERVICES_PREFERENCES_PUBLIC_CPP_TRACKED_MOCK_VALIDATION_DELEGATE_H_

#include <stddef.h>

#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "services/preferences/public/interfaces/preferences.mojom.h"
#include "services/preferences/public/interfaces/tracked_preference_validation_delegate.mojom.h"

class MockValidationDelegate;

// A mock tracked preference validation delegate for use by tests.
class MockValidationDelegateRecord
    : public base::RefCounted<MockValidationDelegateRecord> {
 public:
  struct ValidationEvent {
    ValidationEvent(
        const std::string& path,
        prefs::mojom::TrackedPreferenceValidationDelegate::ValueState state,
        prefs::mojom::TrackedPreferenceValidationDelegate::ValueState
            external_validation_state,
        bool is_personal,
        prefs::mojom::TrackedPreferenceMetadata::PrefTrackingStrategy
            tracking_strategy)
        : pref_path(path),
          value_state(state),
          external_validation_value_state(external_validation_state),
          is_personal(is_personal),
          strategy(tracking_strategy) {}

    std::string pref_path;
    prefs::mojom::TrackedPreferenceValidationDelegate::ValueState value_state;
    prefs::mojom::TrackedPreferenceValidationDelegate::ValueState
        external_validation_value_state;
    bool is_personal;
    prefs::mojom::TrackedPreferenceMetadata::PrefTrackingStrategy strategy;
  };

  MockValidationDelegateRecord();

  // Returns the number of recorded validations.
  size_t recorded_validations_count() const { return validations_.size(); }

  // Returns the number of validations of a given value state.
  size_t CountValidationsOfState(
      prefs::mojom::TrackedPreferenceValidationDelegate::ValueState value_state)
      const;

  // Returns the number of external validations of a given value state.
  size_t CountExternalValidationsOfState(
      prefs::mojom::TrackedPreferenceValidationDelegate::ValueState value_state)
      const;

  // Returns the event for the preference with a given path.
  const ValidationEvent* GetEventForPath(const std::string& pref_path) const;

 private:
  friend class MockValidationDelegate;
  friend class base::RefCounted<MockValidationDelegateRecord>;

  ~MockValidationDelegateRecord();

  // Adds a new validation event.
  void RecordValidation(
      const std::string& pref_path,
      prefs::mojom::TrackedPreferenceValidationDelegate::ValueState value_state,
      prefs::mojom::TrackedPreferenceValidationDelegate::ValueState
          external_validation_value_state,
      bool is_personal,
      prefs::mojom::TrackedPreferenceMetadata::PrefTrackingStrategy strategy);

  std::vector<ValidationEvent> validations_;

  DISALLOW_COPY_AND_ASSIGN(MockValidationDelegateRecord);
};

class MockValidationDelegate
    : public prefs::mojom::TrackedPreferenceValidationDelegate {
 public:
  explicit MockValidationDelegate(
      scoped_refptr<MockValidationDelegateRecord> record);
  ~MockValidationDelegate() override;

  // TrackedPreferenceValidationDelegate implementation.
  void OnAtomicPreferenceValidation(
      const std::string& pref_path,
      std::unique_ptr<base::Value> value,
      prefs::mojom::TrackedPreferenceValidationDelegate::ValueState value_state,
      prefs::mojom::TrackedPreferenceValidationDelegate::ValueState
          external_validation_value_state,
      bool is_personal) override;
  void OnSplitPreferenceValidation(
      const std::string& pref_path,
      const std::vector<std::string>& invalid_keys,
      const std::vector<std::string>& external_validation_invalid_keys,
      prefs::mojom::TrackedPreferenceValidationDelegate::ValueState value_state,
      prefs::mojom::TrackedPreferenceValidationDelegate::ValueState
          external_validation_value_state,
      bool is_personal) override;

 private:
  // Adds a new validation event.
  void RecordValidation(
      const std::string& pref_path,
      prefs::mojom::TrackedPreferenceValidationDelegate::ValueState value_state,
      prefs::mojom::TrackedPreferenceValidationDelegate::ValueState
          external_validation_value_state,
      bool is_personal,
      prefs::mojom::TrackedPreferenceMetadata::PrefTrackingStrategy strategy);

  scoped_refptr<MockValidationDelegateRecord> record_;

  DISALLOW_COPY_AND_ASSIGN(MockValidationDelegate);
};

#endif  // SERVICES_PREFERENCES_PUBLIC_CPP_TRACKED_MOCK_VALIDATION_DELEGATE_H_
