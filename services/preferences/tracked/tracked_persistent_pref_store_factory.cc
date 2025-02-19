// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/preferences/tracked/tracked_persistent_pref_store_factory.h"

#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "components/prefs/json_pref_store.h"
#include "components/prefs/pref_filter.h"
#include "services/preferences/public/interfaces/tracked_preference_validation_delegate.mojom.h"
#include "services/preferences/tracked/pref_hash_filter.h"
#include "services/preferences/tracked/pref_hash_store_impl.h"
#include "services/preferences/tracked/segregated_pref_store.h"
#include "services/preferences/tracked/temp_scoped_dir_cleaner.h"
#include "services/preferences/tracked/tracked_preferences_migration.h"

#if defined(OS_WIN)
#include "base/files/scoped_temp_dir.h"
#include "base/strings/string_util.h"
#include "services/preferences/tracked/registry_hash_store_contents_win.h"
#endif

namespace {

void RemoveValueSilently(const base::WeakPtr<JsonPrefStore> pref_store,
                         const std::string& key) {
  if (pref_store) {
    pref_store->RemoveValueSilently(
        key, WriteablePrefStore::DEFAULT_PREF_WRITE_FLAGS);
  }
}

std::unique_ptr<PrefHashStore> CreatePrefHashStore(
    const prefs::mojom::TrackedPersistentPrefStoreConfiguration& config,
    bool use_super_mac) {
  return base::MakeUnique<PrefHashStoreImpl>(
      config.seed, config.legacy_device_id, use_super_mac);
}

std::pair<std::unique_ptr<PrefHashStore>, std::unique_ptr<HashStoreContents>>
GetExternalVerificationPrefHashStorePair(
    const prefs::mojom::TrackedPersistentPrefStoreConfiguration& config,
    scoped_refptr<TempScopedDirCleaner> temp_dir_cleaner) {
#if defined(OS_WIN)
  return std::make_pair(base::MakeUnique<PrefHashStoreImpl>(
                            config.registry_seed, config.legacy_device_id,
                            false /* use_super_mac */),
                        base::MakeUnique<RegistryHashStoreContentsWin>(
                            config.registry_path,
                            config.unprotected_pref_filename.DirName()
                                .BaseName()
                                .LossyDisplayName(),
                            std::move(temp_dir_cleaner)));
#else
  return std::make_pair(nullptr, nullptr);
#endif
}

}  // namespace

PersistentPrefStore* CreateTrackedPersistentPrefStore(
    prefs::mojom::TrackedPersistentPrefStoreConfigurationPtr config,
    scoped_refptr<base::SequencedTaskRunner> io_task_runner) {
  std::vector<prefs::mojom::TrackedPreferenceMetadataPtr>
      unprotected_configuration;
  std::vector<prefs::mojom::TrackedPreferenceMetadataPtr>
      protected_configuration;
  std::set<std::string> protected_pref_names;
  std::set<std::string> unprotected_pref_names;
  for (auto& metadata : config->tracking_configuration) {
    if (metadata->enforcement_level > prefs::mojom::TrackedPreferenceMetadata::
                                          EnforcementLevel::NO_ENFORCEMENT) {
      protected_pref_names.insert(metadata->name);
      protected_configuration.push_back(std::move(metadata));
    } else {
      unprotected_pref_names.insert(metadata->name);
      unprotected_configuration.push_back(std::move(metadata));
    }
  }
  config->tracking_configuration.clear();

  scoped_refptr<TempScopedDirCleaner> temp_scoped_dir_cleaner;
#if defined(OS_WIN)
  // For tests that create a profile in a ScopedTempDir, share a ref_counted
  // object between the unprotected and protected hash filter's
  // RegistryHashStoreContentsWin which will clear the registry keys when
  // destroyed. (https://crbug.com/721245)
  if (base::StartsWith(config->unprotected_pref_filename.DirName()
                           .BaseName()
                           .LossyDisplayName(),
                       base::ScopedTempDir::GetTempDirPrefix(),
                       base::CompareCase::INSENSITIVE_ASCII)) {
    temp_scoped_dir_cleaner =
        base::MakeRefCounted<TempScopedDirRegistryCleaner>();
  }
#endif

  std::unique_ptr<PrefHashFilter> unprotected_pref_hash_filter(
      new PrefHashFilter(CreatePrefHashStore(*config, false),
                         GetExternalVerificationPrefHashStorePair(
                             *config, temp_scoped_dir_cleaner),
                         unprotected_configuration, nullptr,
                         config->validation_delegate.get(),
                         config->reporting_ids_count, false));
  std::unique_ptr<PrefHashFilter> protected_pref_hash_filter(new PrefHashFilter(
      CreatePrefHashStore(*config, true),
      GetExternalVerificationPrefHashStorePair(*config,
                                               temp_scoped_dir_cleaner),
      protected_configuration, std::move(config->reset_on_load_observer),
      config->validation_delegate.get(), config->reporting_ids_count, true));

  PrefHashFilter* raw_unprotected_pref_hash_filter =
      unprotected_pref_hash_filter.get();
  PrefHashFilter* raw_protected_pref_hash_filter =
      protected_pref_hash_filter.get();

  scoped_refptr<JsonPrefStore> unprotected_pref_store(
      new JsonPrefStore(config->unprotected_pref_filename, io_task_runner.get(),
                        std::move(unprotected_pref_hash_filter)));
  scoped_refptr<JsonPrefStore> protected_pref_store(
      new JsonPrefStore(config->protected_pref_filename, io_task_runner.get(),
                        std::move(protected_pref_hash_filter)));

  SetupTrackedPreferencesMigration(
      unprotected_pref_names, protected_pref_names,
      base::Bind(&RemoveValueSilently, unprotected_pref_store->AsWeakPtr()),
      base::Bind(&RemoveValueSilently, protected_pref_store->AsWeakPtr()),
      base::Bind(&JsonPrefStore::RegisterOnNextSuccessfulWriteReply,
                 unprotected_pref_store->AsWeakPtr()),
      base::Bind(&JsonPrefStore::RegisterOnNextSuccessfulWriteReply,
                 protected_pref_store->AsWeakPtr()),
      CreatePrefHashStore(*config, false), CreatePrefHashStore(*config, true),
      raw_unprotected_pref_hash_filter, raw_protected_pref_hash_filter);

  return new SegregatedPrefStore(unprotected_pref_store, protected_pref_store,
                                 protected_pref_names,
                                 std::move(config->validation_delegate));
}

void InitializeMasterPrefsTracking(
    prefs::mojom::TrackedPersistentPrefStoreConfigurationPtr configuration,
    base::DictionaryValue* master_prefs) {
  PrefHashFilter(
      CreatePrefHashStore(*configuration, false),
      GetExternalVerificationPrefHashStorePair(*configuration, nullptr),
      configuration->tracking_configuration, nullptr, nullptr,
      configuration->reporting_ids_count, false)
      .Initialize(master_prefs);
}
