// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_QUADS_SOLID_COLOR_DRAW_QUAD_H_
#define CC_QUADS_SOLID_COLOR_DRAW_QUAD_H_

#include <memory>

#include "cc/cc_export.h"
#include "components/viz/common/quads/draw_quad.h"
#include "third_party/skia/include/core/SkColor.h"

namespace cc {

class CC_EXPORT SolidColorDrawQuad : public viz::DrawQuad {
 public:
  SolidColorDrawQuad();

  void SetNew(const viz::SharedQuadState* shared_quad_state,
              const gfx::Rect& rect,
              const gfx::Rect& visible_rect,
              SkColor color,
              bool force_anti_aliasing_off);

  void SetAll(const viz::SharedQuadState* shared_quad_state,
              const gfx::Rect& rect,
              const gfx::Rect& visible_rect,
              bool needs_blending,
              SkColor color,
              bool force_anti_aliasing_off);

  SkColor color;
  bool force_anti_aliasing_off;

  static const SolidColorDrawQuad* MaterialCast(const viz::DrawQuad*);

 private:
  void ExtendValue(base::trace_event::TracedValue* value) const override;
};

}  // namespace cc

#endif  // CC_QUADS_SOLID_COLOR_DRAW_QUAD_H_
