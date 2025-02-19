// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_RENDERER_LAZY_BACKGROUND_PAGE_NATIVE_HANDLER_H_
#define EXTENSIONS_RENDERER_LAZY_BACKGROUND_PAGE_NATIVE_HANDLER_H_

#include "extensions/renderer/object_backed_native_handler.h"

namespace extensions {

class LazyBackgroundPageNativeHandler : public ObjectBackedNativeHandler {
 public:
  explicit LazyBackgroundPageNativeHandler(ScriptContext* context);
  void IncrementKeepaliveCount(const v8::FunctionCallbackInfo<v8::Value>& args);
  void DecrementKeepaliveCount(const v8::FunctionCallbackInfo<v8::Value>& args);
};

}  // namespace extensions

#endif  // EXTENSIONS_RENDERER_LAZY_BACKGROUND_PAGE_NATIVE_HANDLER_H_
