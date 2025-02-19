// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/preferences/public/cpp/scoped_pref_update.h"

#include <utility>

#include "base/bind.h"
#include "base/memory/ptr_util.h"
#include "components/prefs/pref_service.h"
#include "services/preferences/public/cpp/dictionary_value_update.h"

namespace prefs {

ScopedDictionaryPrefUpdate::ScopedDictionaryPrefUpdate(PrefService* service,
                                                       base::StringPiece path)
    : service_(service), path_(path.as_string()) {}

ScopedDictionaryPrefUpdate::~ScopedDictionaryPrefUpdate() {
  if (!updated_paths_.empty())
    service_->ReportUserPrefChanged(path_, std::move(updated_paths_));
}

std::unique_ptr<DictionaryValueUpdate> ScopedDictionaryPrefUpdate::Get() {
  return base::MakeUnique<DictionaryValueUpdate>(
      base::Bind(&ScopedDictionaryPrefUpdate::RecordPath,
                 base::Unretained(this)),
      static_cast<base::DictionaryValue*>(
          service_->GetMutableUserPref(path_, base::Value::Type::DICTIONARY)),
      std::vector<std::string>());
}

std::unique_ptr<DictionaryValueUpdate> ScopedDictionaryPrefUpdate::
operator->() {
  return Get();
}

void ScopedDictionaryPrefUpdate::RecordPath(
    const std::vector<std::string>& path) {
  updated_paths_.insert(std::move(path));
}

}  // namespace prefs
