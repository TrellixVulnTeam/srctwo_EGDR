// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_RENDERER_BINDINGS_API_BINDING_HOOKS_DELEGATE_H_
#define EXTENSIONS_RENDERER_BINDINGS_API_BINDING_HOOKS_DELEGATE_H_

#include "extensions/renderer/bindings/api_binding_hooks.h"
#include "extensions/renderer/bindings/api_binding_types.h"
#include "v8/include/v8.h"

namespace extensions {
class APITypeReferenceMap;

// A per-API set of custom hooks to override the default behavior.
class APIBindingHooksDelegate {
 public:
  virtual ~APIBindingHooksDelegate();

  // Allows custom implementations to return a different event object.
  // Populates |event_out| and returns true if a custom implementation should
  // be used, otherwise returns false.
  virtual bool CreateCustomEvent(v8::Local<v8::Context> context,
                                 const binding::RunJSFunctionSync& run_js_sync,
                                 const std::string& event_name,
                                 v8::Local<v8::Value>* event_out);

  // Allows custom implementations to handle a given request.
  virtual APIBindingHooks::RequestResult HandleRequest(
      const std::string& method_name,
      const APISignature* signature,
      v8::Local<v8::Context> context,
      std::vector<v8::Local<v8::Value>>* arguments,
      const APITypeReferenceMap& refs);

  // Allows custom implementations to add additional properties or types to an
  // API object.
  virtual void InitializeTemplate(v8::Isolate* isolate,
                                  v8::Local<v8::ObjectTemplate> object_template,
                                  const APITypeReferenceMap& type_refs) {}
};

}  // namespace extensions

#endif  // EXTENSIONS_RENDERER_BINDINGS_API_BINDING_HOOKS_DELEGATE_H_
