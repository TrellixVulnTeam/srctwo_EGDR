// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_CHILD_WEBTHEMEENGINE_IMPL_ANDROID_H_
#define CONTENT_CHILD_WEBTHEMEENGINE_IMPL_ANDROID_H_

#include "third_party/WebKit/public/platform/WebThemeEngine.h"

namespace content {

class WebThemeEngineImpl : public blink::WebThemeEngine {
 public:
  // WebThemeEngine methods:
  blink::WebSize GetSize(blink::WebThemeEngine::Part) override;
  void GetOverlayScrollbarStyle(
      blink::WebThemeEngine::ScrollbarStyle*) override;
  void Paint(blink::WebCanvas* canvas,
             blink::WebThemeEngine::Part part,
             blink::WebThemeEngine::State state,
             const blink::WebRect& rect,
             const blink::WebThemeEngine::ExtraParams* extra_params) override;
};

}  // namespace content

#endif  // CONTENT_CHILD_WEBTHEMEENGINE_IMPL_ANDROID_H_
