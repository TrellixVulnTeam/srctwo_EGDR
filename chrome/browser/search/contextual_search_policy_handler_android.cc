// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/search/contextual_search_policy_handler_android.h"

#include "base/values.h"
#include "chrome/common/pref_names.h"
#include "components/policy/core/common/policy_map.h"
#include "components/policy/policy_constants.h"
#include "components/prefs/pref_value_map.h"

namespace policy {

ContextualSearchPolicyHandlerAndroid::ContextualSearchPolicyHandlerAndroid()
    : TypeCheckingPolicyHandler(key::kContextualSearchEnabled,
                                base::Value::Type::BOOLEAN) {}

ContextualSearchPolicyHandlerAndroid::~ContextualSearchPolicyHandlerAndroid() {
}

void ContextualSearchPolicyHandlerAndroid::ApplyPolicySettings(
    const PolicyMap& policies,
    PrefValueMap* prefs) {
  const base::Value* value = policies.GetValue(policy_name());
  bool contextual_search_enabled = true;
  // From a Contextual Search preference point of view, "false" means the
  // feature is turned off completely. "" means the feature is uninitialized and
  // an opt-in screen is presented to the user, after which the preference is
  // either "true" or "false", depending on their choice. Here a false policy
  // explicitly disables Contextual Search.
  if (value &&
      value->GetAsBoolean(&contextual_search_enabled) &&
      !contextual_search_enabled) {
    prefs->SetString(prefs::kContextualSearchEnabled, "false");
  }
}

}  // namespace policy
