// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/sync/driver/data_type_status_table.h"

#include "components/sync/driver/data_type_manager.h"

namespace syncer {

namespace {

ModelTypeSet GetTypesFromErrorMap(
    const DataTypeStatusTable::TypeErrorMap& errors) {
  ModelTypeSet result;
  for (DataTypeStatusTable::TypeErrorMap::const_iterator it = errors.begin();
       it != errors.end(); ++it) {
    DCHECK(!result.Has(it->first));
    result.Put(it->first);
  }
  return result;
}

}  // namespace

DataTypeStatusTable::DataTypeStatusTable() {}

DataTypeStatusTable::DataTypeStatusTable(const DataTypeStatusTable& other) =
    default;

DataTypeStatusTable::~DataTypeStatusTable() {}

void DataTypeStatusTable::UpdateFailedDataTypes(const TypeErrorMap& errors) {
  DVLOG(1) << "Setting " << errors.size() << " new failed types.";

  for (TypeErrorMap::const_iterator iter = errors.begin(); iter != errors.end();
       ++iter) {
    SyncError::ErrorType failure_type = iter->second.error_type();
    switch (failure_type) {
      case SyncError::UNSET:
        NOTREACHED();
        break;
      case SyncError::UNRECOVERABLE_ERROR:
        unrecoverable_errors_.insert(*iter);
        break;
      case SyncError::DATATYPE_ERROR:
      case SyncError::DATATYPE_POLICY_ERROR:
        data_type_errors_.insert(*iter);
        break;
      case SyncError::CRYPTO_ERROR:
        crypto_errors_.insert(*iter);
        break;
      case SyncError::PERSISTENCE_ERROR:
        persistence_errors_.insert(*iter);
        break;
      case SyncError::UNREADY_ERROR:
        unready_errors_.insert(*iter);
        break;
    }
  }
}

void DataTypeStatusTable::Reset() {
  DVLOG(1) << "Resetting data type errors.";
  unrecoverable_errors_.clear();
  data_type_errors_.clear();
  crypto_errors_.clear();
  persistence_errors_.clear();
  unready_errors_.clear();
}

void DataTypeStatusTable::ResetCryptoErrors() {
  crypto_errors_.clear();
}

void DataTypeStatusTable::ResetPersistenceErrorsFrom(
    ModelTypeSet purged_types) {
  for (ModelTypeSet::Iterator iter = purged_types.First(); iter.Good();
       iter.Inc()) {
    persistence_errors_.erase(iter.Get());
  }
}

bool DataTypeStatusTable::ResetDataTypeErrorFor(ModelType type) {
  return data_type_errors_.erase(type) > 0;
}

bool DataTypeStatusTable::ResetUnreadyErrorFor(ModelType type) {
  return unready_errors_.erase(type) > 0;
}

DataTypeStatusTable::TypeErrorMap DataTypeStatusTable::GetAllErrors() const {
  TypeErrorMap result;
  result.insert(data_type_errors_.begin(), data_type_errors_.end());
  result.insert(crypto_errors_.begin(), crypto_errors_.end());
  result.insert(persistence_errors_.begin(), persistence_errors_.end());
  result.insert(unready_errors_.begin(), unready_errors_.end());
  result.insert(unrecoverable_errors_.begin(), unrecoverable_errors_.end());
  return result;
}

ModelTypeSet DataTypeStatusTable::GetFailedTypes() const {
  ModelTypeSet result = GetFatalErrorTypes();
  result.PutAll(GetCryptoErrorTypes());
  result.PutAll(GetUnreadyErrorTypes());
  return result;
}

ModelTypeSet DataTypeStatusTable::GetFatalErrorTypes() const {
  ModelTypeSet result;
  result.PutAll(GetTypesFromErrorMap(data_type_errors_));
  result.PutAll(GetTypesFromErrorMap(unrecoverable_errors_));
  return result;
}

ModelTypeSet DataTypeStatusTable::GetCryptoErrorTypes() const {
  ModelTypeSet result = GetTypesFromErrorMap(crypto_errors_);
  return result;
}

ModelTypeSet DataTypeStatusTable::GetPersistenceErrorTypes() const {
  ModelTypeSet result = GetTypesFromErrorMap(persistence_errors_);
  return result;
}

ModelTypeSet DataTypeStatusTable::GetUnreadyErrorTypes() const {
  ModelTypeSet result = GetTypesFromErrorMap(unready_errors_);
  return result;
}

ModelTypeSet DataTypeStatusTable::GetUnrecoverableErrorTypes() const {
  ModelTypeSet result = GetTypesFromErrorMap(unrecoverable_errors_);
  return result;
}

SyncError DataTypeStatusTable::GetUnrecoverableError() const {
  // Just return the first one. It is assumed all the unrecoverable errors
  // have the same cause. The others are just tracked to know which types
  // were involved.
  return (unrecoverable_errors_.empty()
              ? SyncError()
              : unrecoverable_errors_.begin()->second);
}

}  // namespace syncer
