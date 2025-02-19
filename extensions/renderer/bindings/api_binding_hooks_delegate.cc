// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/renderer/bindings/api_binding_hooks_delegate.h"

namespace extensions {

APIBindingHooksDelegate::~APIBindingHooksDelegate() {}

bool APIBindingHooksDelegate::CreateCustomEvent(
    v8::Local<v8::Context> context,
    const binding::RunJSFunctionSync& run_js_sync,
    const std::string& event_name,
    v8::Local<v8::Value>* event_out) {
  return false;
}

APIBindingHooks::RequestResult APIBindingHooksDelegate::HandleRequest(
    const std::string& method_name,
    const APISignature* signature,
    v8::Local<v8::Context> context,
    std::vector<v8::Local<v8::Value>>* arguments,
    const APITypeReferenceMap& refs) {
  return APIBindingHooks::RequestResult(
      APIBindingHooks::RequestResult::NOT_HANDLED);
}

}  // namespace extensions
