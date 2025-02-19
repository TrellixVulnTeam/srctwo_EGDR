// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_RENDERER_SEARCHBOX_SEARCHBOX_EXTENSION_H_
#define CHROME_RENDERER_SEARCHBOX_SEARCHBOX_EXTENSION_H_

#include "base/macros.h"
#include "base/strings/string16.h"

namespace v8 {
class Extension;
}

namespace blink {
class WebLocalFrame;
}

namespace extensions_v8 {

// Reference implementation of the SearchBox API as described in:
// http://dev.chromium.org/searchbox
class SearchBoxExtension {
 public:
  // Returns the v8::Extension object handling searchbox bindings. Returns null
  // if match-preview is not enabled. Caller takes ownership of returned object.
  static v8::Extension* Get();

  // Helpers to dispatch Javascript events.
  static void DispatchChromeIdentityCheckResult(blink::WebLocalFrame* frame,
                                                const base::string16& identity,
                                                bool identity_match);
  static void DispatchFocusChange(blink::WebLocalFrame* frame);
  static void DispatchHistorySyncCheckResult(blink::WebLocalFrame* frame,
                                             bool sync_history);
  static void DispatchInputCancel(blink::WebLocalFrame* frame);
  static void DispatchInputStart(blink::WebLocalFrame* frame);
  static void DispatchKeyCaptureChange(blink::WebLocalFrame* frame);
  static void DispatchMostVisitedChanged(blink::WebLocalFrame* frame);
  static void DispatchThemeChange(blink::WebLocalFrame* frame);

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(SearchBoxExtension);
};

}  // namespace extensions_v8

#endif  // CHROME_RENDERER_SEARCHBOX_SEARCHBOX_EXTENSION_H_
