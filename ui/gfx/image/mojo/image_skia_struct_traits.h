// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_IMAGE_MOJO_IMAGE_SKIA_STRUCT_TRAITS_H_
#define UI_GFX_IMAGE_MOJO_IMAGE_SKIA_STRUCT_TRAITS_H_

#include <stdint.h>

#include <vector>

#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/image/image_skia_rep.h"
#include "ui/gfx/image/mojo/image.mojom-shared.h"

namespace mojo {

template <>
struct StructTraits<gfx::mojom::SharedBufferSkBitmapDataView, SkBitmap> {
  struct Context {
    Context();
    ~Context();

    mojo::ScopedSharedBufferHandle shared_buffer_handle;
    uint64_t buffer_byte_size = 0;
  };

  static void* SetUpContext(const SkBitmap& input);
  static void TearDownContext(const SkBitmap& input, void* context);
  static mojo::ScopedSharedBufferHandle shared_buffer_handle(
      const SkBitmap& input,
      void* context);
  static uint64_t buffer_byte_size(const SkBitmap& input, void* context);

  static bool IsNull(const SkBitmap& input) { return input.isNull(); }
  static void SetToNull(SkBitmap* out) { out->reset(); }

  static bool Read(gfx::mojom::SharedBufferSkBitmapDataView data,
                   SkBitmap* out);
};

template <>
struct StructTraits<gfx::mojom::ImageSkiaRepDataView, gfx::ImageSkiaRep> {
  static SkBitmap bitmap(const gfx::ImageSkiaRep& input) {
    return input.sk_bitmap();
  }
  static float scale(const gfx::ImageSkiaRep& input);

  static bool IsNull(const gfx::ImageSkiaRep& input) { return input.is_null(); }
  static void SetToNull(gfx::ImageSkiaRep* out) { *out = gfx::ImageSkiaRep(); }

  static bool Read(gfx::mojom::ImageSkiaRepDataView data,
                   gfx::ImageSkiaRep* out);
};

template <>
struct StructTraits<gfx::mojom::ImageSkiaDataView, gfx::ImageSkia> {
  static std::vector<gfx::ImageSkiaRep> image_reps(const gfx::ImageSkia& input);

  static bool IsNull(const gfx::ImageSkia& input) { return input.isNull(); }
  static void SetToNull(gfx::ImageSkia* out) { *out = gfx::ImageSkia(); }

  static bool Read(gfx::mojom::ImageSkiaDataView data, gfx::ImageSkia* out);
};

}  // namespace mojo

#endif  // UI_GFX_IMAGE_MOJO_IMAGE_SKIA_STRUCT_TRAITS_H_
