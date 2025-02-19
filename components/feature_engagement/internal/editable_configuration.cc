// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/feature_engagement/internal/editable_configuration.h"

#include <map>

#include "base/feature_list.h"
#include "base/logging.h"
#include "components/feature_engagement/internal/configuration.h"

namespace feature_engagement {

EditableConfiguration::EditableConfiguration() = default;

EditableConfiguration::~EditableConfiguration() = default;

void EditableConfiguration::SetConfiguration(
    const base::Feature* feature,
    const FeatureConfig& feature_config) {
  configs_[feature->name] = feature_config;
}

const FeatureConfig& EditableConfiguration::GetFeatureConfig(
    const base::Feature& feature) const {
  auto it = configs_.find(feature.name);
  DCHECK(it != configs_.end());
  return it->second;
}

const FeatureConfig& EditableConfiguration::GetFeatureConfigByName(
    const std::string& feature_name) const {
  auto it = configs_.find(feature_name);
  DCHECK(it != configs_.end());
  return it->second;
}

const Configuration::ConfigMap& EditableConfiguration::GetRegisteredFeatures()
    const {
  return configs_;
}

}  // namespace feature_engagement
