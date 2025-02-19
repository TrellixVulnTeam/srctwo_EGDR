// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/renderer/test_native_handler.h"

#include "extensions/renderer/wake_event_page.h"

namespace extensions {

TestNativeHandler::TestNativeHandler(ScriptContext* context)
    : ObjectBackedNativeHandler(context) {
  RouteFunction(
      "GetWakeEventPage", "test",
      base::Bind(&TestNativeHandler::GetWakeEventPage, base::Unretained(this)));
}

void TestNativeHandler::GetWakeEventPage(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  CHECK_EQ(0, args.Length());
  args.GetReturnValue().Set(WakeEventPage::Get()->GetForContext(context()));
}

}  // namespace extensions
