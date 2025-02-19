// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jni/ResourceFactory_jni.h"
#include "ui/android/resources/nine_patch_resource.h"
#include "ui/gfx/geometry/rect.h"

using base::android::JavaParamRef;

namespace ui {

jlong CreateBitmapResource(JNIEnv* env, const JavaParamRef<jclass>& clazz) {
  return reinterpret_cast<intptr_t>(new Resource());
}

jlong CreateNinePatchBitmapResource(JNIEnv* env,
                                    const JavaParamRef<jclass>& clazz,
                                    jint padding_left,
                                    jint padding_top,
                                    jint padding_right,
                                    jint padding_bottom,
                                    jint aperture_left,
                                    jint aperture_top,
                                    jint aperture_right,
                                    jint aperture_bottom) {
  gfx::Rect padding(padding_left, padding_top, padding_right - padding_left,
                    padding_bottom - padding_top);
  gfx::Rect aperture(aperture_left, aperture_top,
                     aperture_right - aperture_left,
                     aperture_bottom - aperture_top);
  return reinterpret_cast<intptr_t>(new NinePatchResource(padding, aperture));
}

}  // namespace ui
