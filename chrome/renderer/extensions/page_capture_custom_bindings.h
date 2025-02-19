// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_RENDERER_EXTENSIONS_PAGE_CAPTURE_CUSTOM_BINDINGS_H_
#define CHROME_RENDERER_EXTENSIONS_PAGE_CAPTURE_CUSTOM_BINDINGS_H_

#include "extensions/renderer/object_backed_native_handler.h"

namespace extensions {

// Implements custom bindings for the pageCapture API.
class PageCaptureCustomBindings : public ObjectBackedNativeHandler {
 public:
  explicit PageCaptureCustomBindings(ScriptContext* context);

 private:
  // Creates a Blob with the content of the specified file.
  void CreateBlob(const v8::FunctionCallbackInfo<v8::Value>& args);
  void SendResponseAck(const v8::FunctionCallbackInfo<v8::Value>& args);
};

}  // namespace extensions

#endif  // CHROME_RENDERER_EXTENSIONS_PAGE_CAPTURE_CUSTOM_BINDINGS_H_
