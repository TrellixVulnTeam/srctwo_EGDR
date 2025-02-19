// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/renderer/api_definitions_natives.h"

#include "extensions/common/features/feature.h"
#include "extensions/common/features/feature_provider.h"
#include "extensions/renderer/dispatcher.h"
#include "extensions/renderer/script_context.h"

namespace extensions {

ApiDefinitionsNatives::ApiDefinitionsNatives(Dispatcher* dispatcher,
                                             ScriptContext* context)
    : ObjectBackedNativeHandler(context), dispatcher_(dispatcher) {
  RouteFunction(
      "GetExtensionAPIDefinitionsForTest", "test",
      base::Bind(&ApiDefinitionsNatives::GetExtensionAPIDefinitionsForTest,
                 base::Unretained(this)));
}

void ApiDefinitionsNatives::GetExtensionAPIDefinitionsForTest(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  std::vector<std::string> apis;
  const FeatureProvider* feature_provider = FeatureProvider::GetAPIFeatures();
  for (const auto& map_entry : feature_provider->GetAllFeatures()) {
    if (!feature_provider->GetParent(*map_entry.second) &&
        context()->GetAvailability(map_entry.first).is_available()) {
      apis.push_back(map_entry.first);
    }
  }
  args.GetReturnValue().Set(
      dispatcher_->v8_schema_registry()->GetSchemas(apis));
}

}  // namespace extensions
