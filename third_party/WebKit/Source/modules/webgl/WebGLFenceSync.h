// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebGLFenceSync_h
#define WebGLFenceSync_h

#include "modules/webgl/WebGLSync.h"

namespace blink {

class WebGL2RenderingContextBase;

class WebGLFenceSync : public WebGLSync {
 public:
  static WebGLSync* Create(WebGL2RenderingContextBase*,
                           GLenum condition,
                           GLbitfield flags);

 protected:
  WebGLFenceSync(WebGL2RenderingContextBase*,
                 GLenum condition,
                 GLbitfield flags);
};

}  // namespace blink

#endif  // WebGLFenceSync_h
