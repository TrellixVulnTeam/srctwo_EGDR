// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cc/quads/texture_draw_quad.h"

#include <stddef.h>

#include "base/logging.h"
#include "base/trace_event/trace_event_argument.h"
#include "base/values.h"
#include "cc/base/math_util.h"
#include "ui/gfx/geometry/vector2d_f.h"

namespace cc {

TextureDrawQuad::TextureDrawQuad()
    : premultiplied_alpha(false),
      background_color(SK_ColorTRANSPARENT),
      y_flipped(false),
      nearest_neighbor(false) {
  this->vertex_opacity[0] = 0.f;
  this->vertex_opacity[1] = 0.f;
  this->vertex_opacity[2] = 0.f;
  this->vertex_opacity[3] = 0.f;
}

TextureDrawQuad::TextureDrawQuad(const TextureDrawQuad& other) = default;

void TextureDrawQuad::SetNew(const viz::SharedQuadState* shared_quad_state,
                             const gfx::Rect& rect,
                             const gfx::Rect& visible_rect,
                             bool needs_blending,
                             unsigned resource_id,
                             bool premultiplied_alpha,
                             const gfx::PointF& uv_top_left,
                             const gfx::PointF& uv_bottom_right,
                             SkColor background_color,
                             const float vertex_opacity[4],
                             bool y_flipped,
                             bool nearest_neighbor,
                             bool secure_output_only) {
  needs_blending = needs_blending || vertex_opacity[0] != 1.0f ||
                   vertex_opacity[1] != 1.0f || vertex_opacity[2] != 1.0f ||
                   vertex_opacity[3] != 1.0f;
  viz::DrawQuad::SetAll(shared_quad_state, viz::DrawQuad::TEXTURE_CONTENT, rect,
                        visible_rect, needs_blending);
  resources.ids[kResourceIdIndex] = resource_id;
  resources.count = 1;
  this->premultiplied_alpha = premultiplied_alpha;
  this->uv_top_left = uv_top_left;
  this->uv_bottom_right = uv_bottom_right;
  this->background_color = background_color;
  this->vertex_opacity[0] = vertex_opacity[0];
  this->vertex_opacity[1] = vertex_opacity[1];
  this->vertex_opacity[2] = vertex_opacity[2];
  this->vertex_opacity[3] = vertex_opacity[3];
  this->y_flipped = y_flipped;
  this->nearest_neighbor = nearest_neighbor;
  this->secure_output_only = secure_output_only;
}

void TextureDrawQuad::SetAll(const viz::SharedQuadState* shared_quad_state,
                             const gfx::Rect& rect,
                             const gfx::Rect& visible_rect,
                             bool needs_blending,
                             unsigned resource_id,
                             gfx::Size resource_size_in_pixels,
                             bool premultiplied_alpha,
                             const gfx::PointF& uv_top_left,
                             const gfx::PointF& uv_bottom_right,
                             SkColor background_color,
                             const float vertex_opacity[4],
                             bool y_flipped,
                             bool nearest_neighbor,
                             bool secure_output_only) {
  viz::DrawQuad::SetAll(shared_quad_state, viz::DrawQuad::TEXTURE_CONTENT, rect,
                        visible_rect, needs_blending);
  resources.ids[kResourceIdIndex] = resource_id;
  overlay_resources.size_in_pixels[kResourceIdIndex] = resource_size_in_pixels;
  resources.count = 1;
  this->premultiplied_alpha = premultiplied_alpha;
  this->uv_top_left = uv_top_left;
  this->uv_bottom_right = uv_bottom_right;
  this->background_color = background_color;
  this->vertex_opacity[0] = vertex_opacity[0];
  this->vertex_opacity[1] = vertex_opacity[1];
  this->vertex_opacity[2] = vertex_opacity[2];
  this->vertex_opacity[3] = vertex_opacity[3];
  this->y_flipped = y_flipped;
  this->nearest_neighbor = nearest_neighbor;
  this->secure_output_only = secure_output_only;
}

const TextureDrawQuad* TextureDrawQuad::MaterialCast(
    const viz::DrawQuad* quad) {
  DCHECK(quad->material == viz::DrawQuad::TEXTURE_CONTENT);
  return static_cast<const TextureDrawQuad*>(quad);
}

void TextureDrawQuad::ExtendValue(base::trace_event::TracedValue* value) const {
  value->SetInteger("resource_id", resources.ids[kResourceIdIndex]);
  value->SetBoolean("premultiplied_alpha", premultiplied_alpha);

  MathUtil::AddToTracedValue("uv_top_left", uv_top_left, value);
  MathUtil::AddToTracedValue("uv_bottom_right", uv_bottom_right, value);

  value->SetInteger("background_color", background_color);

  value->BeginArray("vertex_opacity");
  for (size_t i = 0; i < 4; ++i)
    value->AppendDouble(vertex_opacity[i]);
  value->EndArray();

  value->SetBoolean("y_flipped", y_flipped);
  value->SetBoolean("nearest_neighbor", nearest_neighbor);
}

TextureDrawQuad::OverlayResources::OverlayResources() {
}

}  // namespace cc
