// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/common/api/declarative/declarative_manifest_handler.h"

#include "extensions/common/api/declarative/declarative_manifest_data.h"
#include "extensions/common/extension.h"
#include "extensions/common/manifest_constants.h"

namespace extensions {

DeclarativeManifestHandler::DeclarativeManifestHandler() {
}

DeclarativeManifestHandler::~DeclarativeManifestHandler() {
}

bool DeclarativeManifestHandler::Parse(Extension* extension,
                                       base::string16* error) {
  const base::Value* event_rules = NULL;
  CHECK(extension->manifest()->Get(manifest_keys::kEventRules, &event_rules));
  std::unique_ptr<DeclarativeManifestData> data =
      DeclarativeManifestData::FromValue(*event_rules, error);
  if (!data)
    return false;

  extension->SetManifestData(manifest_keys::kEventRules, std::move(data));
  return true;
}

const std::vector<std::string> DeclarativeManifestHandler::Keys() const {
  return SingleKey(manifest_keys::kEventRules);
}

}  // namespace extensions
