// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/safe_browsing/incident_reporting/state_store.h"

#include <utility>

#include "base/memory/ptr_util.h"
#include "base/metrics/histogram_macros.h"
#include "base/strings/string_number_conversions.h"
#include "base/values.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/safe_browsing/incident_reporting/incident.h"
#include "chrome/browser/safe_browsing/incident_reporting/platform_state_store.h"
#include "components/prefs/pref_service.h"
#include "components/safe_browsing/common/safe_browsing_prefs.h"

namespace safe_browsing {

// StateStore::Transaction -----------------------------------------------------

StateStore::Transaction::Transaction(StateStore* store) : store_(store) {
#if DCHECK_IS_ON()
  DCHECK(!store_->has_transaction_);
  store_->has_transaction_ = true;
#endif
}

StateStore::Transaction::~Transaction() {
#if DCHECK_IS_ON()
  store_->has_transaction_ = false;
#endif
  if (pref_update_)
    platform_state_store::Store(store_->profile_, store_->incidents_sent_);
}

void StateStore::Transaction::MarkAsReported(IncidentType type,
                                             const std::string& key,
                                             IncidentDigest digest) {
  std::string type_string(base::IntToString(static_cast<int32_t>(type)));
  base::DictionaryValue* incidents_sent = GetPrefDict();
  base::DictionaryValue* type_dict = nullptr;
  if (!incidents_sent->GetDictionaryWithoutPathExpansion(type_string,
                                                         &type_dict)) {
    type_dict = incidents_sent->SetDictionaryWithoutPathExpansion(
        type_string, base::MakeUnique<base::DictionaryValue>());
  }
  type_dict->SetKey(key, base::Value(base::UintToString(digest)));
}

void StateStore::Transaction::Clear(IncidentType type, const std::string& key) {
  // Nothing to do if the pref dict does not exist.
  if (!store_->incidents_sent_)
    return;

  // Use the read-only view on the preference to figure out if there is a value
  // to remove before committing to making a change since any use of GetPrefDict
  // will result in a full serialize-and-write operation on the preferences
  // store.
  std::string type_string(base::IntToString(static_cast<int32_t>(type)));
  const base::DictionaryValue* const_type_dict = nullptr;
  if (store_->incidents_sent_->GetDictionaryWithoutPathExpansion(
          type_string, &const_type_dict) &&
      const_type_dict->GetWithoutPathExpansion(key, nullptr)) {
    base::DictionaryValue* type_dict = nullptr;
    GetPrefDict()->GetDictionaryWithoutPathExpansion(type_string, &type_dict);
    type_dict->RemoveWithoutPathExpansion(key, nullptr);
  }
}

void StateStore::Transaction::ClearForType(IncidentType type) {
  // Nothing to do if the pref dict does not exist.
  if (!store_->incidents_sent_)
    return;

  // Use the read-only view on the preference to figure out if there is a value
  // to remove before committing to making a change since any use of GetPrefDict
  // will result in a full serialize-and-write operation on the preferences
  // store.
  std::string type_string(base::IntToString(static_cast<int32_t>(type)));
  const base::DictionaryValue* type_dict = nullptr;
  if (store_->incidents_sent_->GetDictionaryWithoutPathExpansion(type_string,
                                                                 &type_dict)) {
    GetPrefDict()->RemoveWithoutPathExpansion(type_string, nullptr);
  }
}

void StateStore::Transaction::ClearAll() {
  // Clear the preference if it exists and contains any values.
  if (store_->incidents_sent_ && !store_->incidents_sent_->empty())
    GetPrefDict()->Clear();
}

base::DictionaryValue* StateStore::Transaction::GetPrefDict() {
  if (!pref_update_) {
    pref_update_.reset(new DictionaryPrefUpdate(
        store_->profile_->GetPrefs(), prefs::kSafeBrowsingIncidentsSent));
    // Getting the dict will cause it to be created if it doesn't exist.
    // Unconditionally refresh the store's read-only view on the preference so
    // that it will always be correct.
    store_->incidents_sent_ = pref_update_->Get();
  }
  return pref_update_->Get();
}

void StateStore::Transaction::ReplacePrefDict(
    std::unique_ptr<base::DictionaryValue> pref_dict) {
  GetPrefDict()->Swap(pref_dict.get());
}


// StateStore ------------------------------------------------------------------

StateStore::StateStore(Profile* profile)
    : profile_(profile),
      incidents_sent_(nullptr)
#if DCHECK_IS_ON()
      ,
      has_transaction_(false)
#endif
{
  // Cache a read-only view of the preference.
  const base::Value* value =
      profile_->GetPrefs()->GetUserPrefValue(prefs::kSafeBrowsingIncidentsSent);
  if (value)
    value->GetAsDictionary(&incidents_sent_);

  // Apply the platform data.
  Transaction transaction(this);
  std::unique_ptr<base::DictionaryValue> value_dict(
      platform_state_store::Load(profile_));

  InitializationResult state_store_init_result = PSS_MATCHES;
  if (!value_dict) {
    state_store_init_result = PSS_NULL;
  } else if (value_dict->empty()) {
    if (incidents_sent_ && !incidents_sent_->empty())
      state_store_init_result = PSS_EMPTY;
    transaction.ClearAll();
  } else if (!incidents_sent_ || !incidents_sent_->Equals(value_dict.get())) {
    state_store_init_result = PSS_DIFFERS;
    transaction.ReplacePrefDict(std::move(value_dict));
  }
  UMA_HISTOGRAM_ENUMERATION("SBIRS.StateStoreInit", state_store_init_result,
                            NUM_INITIALIZATION_RESULTS);
  if (incidents_sent_)
    CleanLegacyValues(&transaction);
}

StateStore::~StateStore() {
#if DCHECK_IS_ON()
  DCHECK(!has_transaction_);
#endif
}

bool StateStore::HasBeenReported(IncidentType type,
                                 const std::string& key,
                                 IncidentDigest digest) {
  const base::DictionaryValue* type_dict = nullptr;
  std::string digest_string;
  return (incidents_sent_ &&
          incidents_sent_->GetDictionaryWithoutPathExpansion(
              base::IntToString(static_cast<int32_t>(type)), &type_dict) &&
          type_dict->GetStringWithoutPathExpansion(key, &digest_string) &&
          digest_string == base::UintToString(digest));
}

void StateStore::CleanLegacyValues(Transaction* transaction) {
  static const IncidentType kLegacyTypes[] = {
      // TODO(grt): remove in M44 (crbug.com/451173).
      IncidentType::OMNIBOX_INTERACTION,
  };

  for (IncidentType type : kLegacyTypes)
    transaction->ClearForType(type);
}

}  // namespace safe_browsing
