// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_RENDERER_SET_ICON_NATIVES_H_
#define EXTENSIONS_RENDERER_SET_ICON_NATIVES_H_

#include "base/macros.h"
#include "extensions/renderer/object_backed_native_handler.h"
#include "v8/include/v8.h"

namespace extensions {
class ScriptContext;

// Functions exposed to extension JS to implement the setIcon extension API.
class SetIconNatives : public ObjectBackedNativeHandler {
 public:
  explicit SetIconNatives(ScriptContext* context);

 private:
  bool ConvertImageDataToBitmapValue(const v8::Local<v8::Object> image_data,
                                     v8::Local<v8::Value>* image_data_bitmap);
  bool ConvertImageDataSetToBitmapValueSet(
      v8::Local<v8::Object>& details,
      v8::Local<v8::Object>* bitmap_set_value);
  void SetIconCommon(const v8::FunctionCallbackInfo<v8::Value>& args);

  DISALLOW_COPY_AND_ASSIGN(SetIconNatives);
};

}  // namespace extensions

#endif  // EXTENSIONS_RENDERER_SET_ICON_NATIVES_H_
