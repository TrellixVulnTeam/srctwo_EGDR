// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebGLContextAttributeHelpers_h
#define WebGLContextAttributeHelpers_h

#include "core/html/canvas/CanvasContextCreationAttributes.h"
#include "modules/webgl/WebGLContextAttributes.h"
#include "public/platform/Platform.h"

namespace blink {

WebGLContextAttributes ToWebGLContextAttributes(
    const CanvasContextCreationAttributes&);

// Set up the attributes that can be used to create a GL context via the
// Platform API.
Platform::ContextAttributes ToPlatformContextAttributes(
    const CanvasContextCreationAttributes&,
    unsigned web_gl_version,
    bool support_own_offscreen_surface);

}  // namespace blink

#endif  // WebGLContextAttributeHelpers_h
