// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_PREFS_PROFILE_PREF_STORE_MANAGER_H_
#define CHROME_BROWSER_PREFS_PROFILE_PREF_STORE_MANAGER_H_

#include <stddef.h>

#include <memory>
#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "services/preferences/public/interfaces/preferences.mojom.h"
#include "services/preferences/public/interfaces/tracked_preference_validation_delegate.mojom.h"

class PersistentPrefStore;
class PrefService;

namespace base {
class DictionaryValue;
}  // namespace base

namespace service_manager {
class Connector;
}

namespace user_prefs {
class PrefRegistrySyncable;
}  // namespace user_prefs

// Provides a facade through which the user preference store may be accessed and
// managed.
class ProfilePrefStoreManager {
 public:
  // Instantiates a ProfilePrefStoreManager with the configuration required to
  // manage the user preferences of the profile at |profile_path|.
  // |seed| and |legacy_device_id| are used to track preference value changes
  // and must be the same on each launch in order to verify loaded preference
  // values.
  ProfilePrefStoreManager(const base::FilePath& profile_path,
                          const std::string& seed,
                          const std::string& legacy_device_id);

  ~ProfilePrefStoreManager();

  static const bool kPlatformSupportsPreferenceTracking;

  // Register user prefs used by the profile preferences system.
  static void RegisterProfilePrefs(user_prefs::PrefRegistrySyncable* registry);

  // Retrieves the time of the last preference reset event, if any, for
  // |pref_service|. Assumes that |pref_service| is backed by a PrefStore that
  // was built by ProfilePrefStoreManager.
  // If no reset has occurred, returns a null |Time|.
  static base::Time GetResetTime(PrefService* pref_service);

  // Clears the time of the last preference reset event, if any, for
  // |pref_service|. Assumes that |pref_service| is backed by a PrefStore that
  // was built by ProfilePrefStoreManager.
  static void ClearResetTime(PrefService* pref_service);

#if defined(OS_WIN)
  // Call before startup tasks kick in to use a different registry path for
  // storing and validating tracked preference MACs. Callers are responsible
  // for ensuring that the key is deleted on shutdown. For testing only.
  static void SetPreferenceValidationRegistryPathForTesting(
      const base::string16* path);
#endif

  // Creates a PersistentPrefStore providing access to the user preferences of
  // the managed profile. If |reset_on_load_observer| is provided, it will be
  // notified if a reset occurs as a result of loading the profile's prefs. An
  // optional |validation_delegate| will be notified of the status of each
  // tracked preference as they are checked.
  // |tracking_configuration| is used for preference tracking.
  // |reporting_ids_count| is the count of all possible tracked preference IDs
  // (possibly greater than |tracking_configuration.size()|).
  PersistentPrefStore* CreateProfilePrefStore(
      std::vector<prefs::mojom::TrackedPreferenceMetadataPtr>
          tracking_configuration,
      size_t reporting_ids_count,
      scoped_refptr<base::SequencedTaskRunner> io_task_runner,
      prefs::mojom::ResetOnLoadObserverPtr reset_on_load_observer,
      prefs::mojom::TrackedPreferenceValidationDelegatePtr validation_delegate);

  // Initializes the preferences for the managed profile with the preference
  // values in |master_prefs|. Acts synchronously, including blocking IO.
  // Returns true on success.
  bool InitializePrefsFromMasterPrefs(
      std::vector<prefs::mojom::TrackedPreferenceMetadataPtr>
          tracking_configuration,
      size_t reporting_ids_count,
      std::unique_ptr<base::DictionaryValue> master_prefs);

 private:
  // Connects to the pref service over mojo and configures it.
  void ConfigurePrefService(
      std::vector<prefs::mojom::TrackedPreferenceMetadataPtr>
          tracking_configuration,
      size_t reporting_ids_count,
      prefs::mojom::ResetOnLoadObserverPtr reset_on_load_observer,
      prefs::mojom::TrackedPreferenceValidationDelegatePtr validation_delegate,
      service_manager::Connector* connector);

  prefs::mojom::TrackedPersistentPrefStoreConfigurationPtr
  CreateTrackedPrefStoreConfiguration(
      std::vector<prefs::mojom::TrackedPreferenceMetadataPtr>
          tracking_configuration,
      size_t reporting_ids_count,
      prefs::mojom::ResetOnLoadObserverPtr reset_on_load_observer,
      prefs::mojom::TrackedPreferenceValidationDelegatePtr validation_delegate);

  const base::FilePath profile_path_;
  const std::string seed_;
  const std::string legacy_device_id_;

  DISALLOW_COPY_AND_ASSIGN(ProfilePrefStoreManager);
};

#endif  // CHROME_BROWSER_PREFS_PROFILE_PREF_STORE_MANAGER_H_
