// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "skia/ext/platform_canvas.h"

#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "build/build_config.h"
#include "third_party/skia/include/core/SkMetaData.h"
#include "third_party/skia/include/core/SkTypes.h"

namespace skia {

SkBitmap ReadPixels(SkCanvas* canvas) {
  SkBitmap bitmap;
  bitmap.allocPixels(canvas->imageInfo());
  if (!canvas->readPixels(bitmap, 0, 0))
    bitmap.reset();
  return bitmap;
}

bool GetWritablePixels(SkCanvas* canvas, SkPixmap* result) {
  if (!canvas || !result) {
    return false;
  }

  SkImageInfo info;
  size_t row_bytes;
  void* pixels = canvas->accessTopLayerPixels(&info, &row_bytes);
  if (!pixels) {
    result->reset();
    return false;
  }

  result->reset(info, pixels, row_bytes);
  return true;
}

size_t PlatformCanvasStrideForWidth(unsigned width) {
  return 4 * width;
}

#if !defined(WIN32)

std::unique_ptr<SkCanvas> CreatePlatformCanvasWithPixels(
    int width,
    int height,
    bool is_opaque,
    uint8_t* data,
    OnFailureType failureType) {

  SkBitmap bitmap;
  bitmap.setInfo(SkImageInfo::MakeN32(width, height,
      is_opaque ? kOpaque_SkAlphaType : kPremul_SkAlphaType));

  if (data) {
    bitmap.setPixels(data);
  } else {
      if (!bitmap.tryAllocPixels()) {
        if (CRASH_ON_FAILURE == failureType)
          SK_CRASH();
        return nullptr;
      }

      // Follow the logic in SkCanvas::createDevice(), initialize the bitmap if
      // it is not opaque.
      if (!is_opaque)
        bitmap.eraseARGB(0, 0, 0, 0);
  }

  return base::MakeUnique<SkCanvas>(bitmap);
}

#endif

}  // namespace skia
