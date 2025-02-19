// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMECAST_BASE_CAST_FEATURES_H_
#define CHROMECAST_BASE_CAST_FEATURES_H_

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_set>

#include "base/feature_list.h"
#include "base/macros.h"

namespace base {
class DictionaryValue;
class ListValue;
}

namespace chromecast {

// Add Cast Features here.
extern const base::Feature kAllowUserMediaAccess;
extern const base::Feature kEnableQuic;
extern const base::Feature kTripleBuffer720;
extern const base::Feature kSingleBuffer;

// Below are utilities needed by the Cast receiver to persist feature
// information. Client code which is simply querying the status of a feature
// will not need to use these utilities.

// Initialize the global base::FeatureList instance, taking into account
// overrides from DCS and the command line. |dcs_features| and
// |dcs_experiment_ids| are read from the PrefService in the browser process.
// |cmd_line_enable_features| and |cmd_line_disable_features| should be passed
// to this function, unmodified from the command line.
//
// This function should be called before the browser's main loop. After this is
// called, the other functions in this file may be called on any thread.
void InitializeFeatureList(const base::DictionaryValue& dcs_features,
                           const base::ListValue& dcs_experiment_ids,
                           const std::string& cmd_line_enable_features,
                           const std::string& cmd_line_disable_features);

// Given a dictionary of features, create a copy that is ready to be persisted
// to disk. Encodes all values as strings,  which is how the FieldTrial
// classes expect the param data.
base::DictionaryValue GetOverriddenFeaturesForStorage(
    const base::DictionaryValue& features);

// Query the set of experiment ids set for this run. Intended only for metrics
// reporting. Must be called after InitializeFeatureList(). May be called on any
// thread.
const std::unordered_set<int32_t>& GetDCSExperimentIds();

// Reset static state to ensure clean unittests. For tests only.
void ResetCastFeaturesForTesting();

}  // namespace chromecast

#endif  // CHROMECAST_BASE_CAST_FEATURES_H_
