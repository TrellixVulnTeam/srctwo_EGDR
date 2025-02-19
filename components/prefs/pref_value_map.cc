// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/prefs/pref_value_map.h"

#include <map>
#include <memory>
#include <utility>

#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/values.h"

PrefValueMap::PrefValueMap() {}

PrefValueMap::~PrefValueMap() {}

bool PrefValueMap::GetValue(const std::string& key,
                            const base::Value** value) const {
  auto it = prefs_.find(key);
  if (it == prefs_.end())
    return false;

  const base::Value* got_value = it->second.get();
  if (value && got_value)
    *value = got_value;

  return !!got_value;
}

bool PrefValueMap::GetValue(const std::string& key, base::Value** value) {
  auto it = prefs_.find(key);
  if (it == prefs_.end())
    return false;

  base::Value* got_value = it->second.get();
  if (value && got_value)
    *value = got_value;

  return !!got_value;
}

bool PrefValueMap::SetValue(const std::string& key,
                            std::unique_ptr<base::Value> value) {
  DCHECK(value);

  std::unique_ptr<base::Value>& existing_value = prefs_[key];
  if (existing_value && value->Equals(existing_value.get()))
    return false;

  existing_value = std::move(value);
  return true;
}

bool PrefValueMap::RemoveValue(const std::string& key) {
  return prefs_.erase(key) != 0;
}

void PrefValueMap::Clear() {
  prefs_.clear();
}

void PrefValueMap::Swap(PrefValueMap* other) {
  prefs_.swap(other->prefs_);
}

PrefValueMap::iterator PrefValueMap::begin() {
  return prefs_.begin();
}

PrefValueMap::iterator PrefValueMap::end() {
  return prefs_.end();
}

PrefValueMap::const_iterator PrefValueMap::begin() const {
  return prefs_.begin();
}

PrefValueMap::const_iterator PrefValueMap::end() const {
  return prefs_.end();
}

bool PrefValueMap::empty() const {
  return prefs_.empty();
}

bool PrefValueMap::GetBoolean(const std::string& key,
                              bool* value) const {
  const base::Value* stored_value = nullptr;
  return GetValue(key, &stored_value) && stored_value->GetAsBoolean(value);
}

void PrefValueMap::SetBoolean(const std::string& key, bool value) {
  SetValue(key, base::MakeUnique<base::Value>(value));
}

bool PrefValueMap::GetString(const std::string& key,
                             std::string* value) const {
  const base::Value* stored_value = nullptr;
  return GetValue(key, &stored_value) && stored_value->GetAsString(value);
}

void PrefValueMap::SetString(const std::string& key,
                             const std::string& value) {
  SetValue(key, base::MakeUnique<base::Value>(value));
}

bool PrefValueMap::GetInteger(const std::string& key, int* value) const {
  const base::Value* stored_value = nullptr;
  return GetValue(key, &stored_value) && stored_value->GetAsInteger(value);
}

void PrefValueMap::SetInteger(const std::string& key, const int value) {
  SetValue(key, base::MakeUnique<base::Value>(value));
}

void PrefValueMap::SetDouble(const std::string& key, const double value) {
  SetValue(key, base::MakeUnique<base::Value>(value));
}

void PrefValueMap::GetDifferingKeys(
    const PrefValueMap* other,
    std::vector<std::string>* differing_keys) const {
  differing_keys->clear();

  // Put everything into ordered maps.
  std::map<std::string, base::Value*> this_prefs;
  std::map<std::string, base::Value*> other_prefs;
  for (const auto& pair : prefs_)
    this_prefs[pair.first] = pair.second.get();
  for (const auto& pair : other->prefs_)
    other_prefs[pair.first] = pair.second.get();

  // Walk over the maps in lockstep, adding everything that is different.
  auto this_pref = this_prefs.begin();
  auto other_pref = other_prefs.begin();
  while (this_pref != this_prefs.end() && other_pref != other_prefs.end()) {
    const int diff = this_pref->first.compare(other_pref->first);
    if (diff == 0) {
      if (!this_pref->second->Equals(other_pref->second))
        differing_keys->push_back(this_pref->first);
      ++this_pref;
      ++other_pref;
    } else if (diff < 0) {
      differing_keys->push_back(this_pref->first);
      ++this_pref;
    } else if (diff > 0) {
      differing_keys->push_back(other_pref->first);
      ++other_pref;
    }
  }

  // Add the remaining entries.
  for ( ; this_pref != this_prefs.end(); ++this_pref)
      differing_keys->push_back(this_pref->first);
  for ( ; other_pref != other_prefs.end(); ++other_pref)
      differing_keys->push_back(other_pref->first);
}

std::unique_ptr<base::DictionaryValue> PrefValueMap::AsDictionaryValue() const {
  auto dictionary = base::MakeUnique<base::DictionaryValue>();
  for (const auto& value : prefs_) {
    dictionary->Set(value.first, value.second->CreateDeepCopy());
  }
  return dictionary;
}
