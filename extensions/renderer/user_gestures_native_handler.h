// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_RENDERER_USER_GESTURES_NATIVE_HANDLER_H_
#define EXTENSIONS_RENDERER_USER_GESTURES_NATIVE_HANDLER_H_

#include "extensions/renderer/object_backed_native_handler.h"

namespace extensions {

class UserGesturesNativeHandler : public ObjectBackedNativeHandler {
 public:
  explicit UserGesturesNativeHandler(ScriptContext* context);

 private:
  void IsProcessingUserGesture(const v8::FunctionCallbackInfo<v8::Value>& args);
  void RunWithUserGesture(const v8::FunctionCallbackInfo<v8::Value>& args);
  void RunWithoutUserGesture(const v8::FunctionCallbackInfo<v8::Value>& args);
};

}  // namespace extensions

#endif  // EXTENSIONS_RENDERER_USER_GESTURES_NATIVE_HANDLER_H_
