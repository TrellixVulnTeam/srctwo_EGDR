// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_CORE_BROWSER_WEBDATA_AUTOCOMPLETE_SYNC_BRIDGE_H_
#define COMPONENTS_AUTOFILL_CORE_BROWSER_WEBDATA_AUTOCOMPLETE_SYNC_BRIDGE_H_

#include <memory>
#include <string>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/optional.h"
#include "base/scoped_observer.h"
#include "base/supports_user_data.h"
#include "components/autofill/core/browser/webdata/autofill_change.h"
#include "components/autofill/core/browser/webdata/autofill_webdata_service_observer.h"
#include "components/sync/model/metadata_change_list.h"
#include "components/sync/model/model_error.h"
#include "components/sync/model/model_type_sync_bridge.h"

namespace autofill {

class AutofillTable;
class AutofillWebDataBackend;
class AutofillWebDataService;

class AutocompleteSyncBridge
    : public base::SupportsUserData::Data,
      public syncer::ModelTypeSyncBridge,
      public AutofillWebDataServiceObserverOnDBSequence {
 public:
  AutocompleteSyncBridge();
  AutocompleteSyncBridge(
      AutofillWebDataBackend* backend,
      const ChangeProcessorFactory& change_processor_factory);
  ~AutocompleteSyncBridge() override;

  static void CreateForWebDataServiceAndBackend(
      AutofillWebDataService* web_data_service,
      AutofillWebDataBackend* web_data_backend);

  static base::WeakPtr<syncer::ModelTypeSyncBridge> FromWebDataService(
      AutofillWebDataService* web_data_service);

  // syncer::ModelTypeSyncBridge implementation.
  std::unique_ptr<syncer::MetadataChangeList> CreateMetadataChangeList()
      override;
  base::Optional<syncer::ModelError> MergeSyncData(
      std::unique_ptr<syncer::MetadataChangeList> metadata_change_list,
      syncer::EntityChangeList entity_data) override;
  base::Optional<syncer::ModelError> ApplySyncChanges(
      std::unique_ptr<syncer::MetadataChangeList> metadata_change_list,
      syncer::EntityChangeList entity_changes) override;
  void GetData(StorageKeyList storage_keys, DataCallback callback) override;
  void GetAllData(DataCallback callback) override;
  std::string GetClientTag(const syncer::EntityData& entity_data) override;
  std::string GetStorageKey(const syncer::EntityData& entity_data) override;

  // AutofillWebDataServiceObserverOnDBSequence implementation.
  void AutofillEntriesChanged(const AutofillChangeList& changes) override;

 private:
  // Returns the table associated with the |web_data_backend_|.
  AutofillTable* GetAutofillTable() const;

  // Respond to local autofill entries changing by notifying sync of the
  // changes.
  void ActOnLocalChanges(const AutofillChangeList& changes);

  // Synchronously load sync metadata from the autofill table and pass it to the
  // processor so that it can start tracking changes.
  void LoadMetadata();

  base::ThreadChecker thread_checker_;

  // AutocompleteSyncBridge is owned by |web_data_backend_| through
  // SupportsUserData, so it's guaranteed to outlive |this|.
  AutofillWebDataBackend* const web_data_backend_;

  ScopedObserver<AutofillWebDataBackend, AutocompleteSyncBridge>
      scoped_observer_;

  DISALLOW_COPY_AND_ASSIGN(AutocompleteSyncBridge);
};

}  // namespace autofill

#endif  // COMPONENTS_AUTOFILL_CORE_BROWSER_WEBDATA_AUTOCOMPLETE_SYNC_BRIDGE_H_
