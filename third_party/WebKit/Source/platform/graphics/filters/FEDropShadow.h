/*
 * Copyright (C) Research In Motion Limited 2011. All rights reserved.
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef FEDropShadow_h
#define FEDropShadow_h

#include "platform/graphics/Color.h"
#include "platform/graphics/filters/FilterEffect.h"

namespace blink {

class PLATFORM_EXPORT FEDropShadow final : public FilterEffect {
 public:
  static FEDropShadow*
  Create(Filter*, float, float, float, float, const Color&, float);

  // Compute which destination area will be affected when applying a drop
  // shadow effect with |stdDeviation| and |offset| to an area |rect|.
  static FloatRect MapEffect(const FloatSize& std_deviation,
                             const FloatPoint& offset,
                             const FloatRect&);

  void SetShadowColor(const Color& color) { shadow_color_ = color; }
  void SetShadowOpacity(float opacity) { shadow_opacity_ = opacity; }

  TextStream& ExternalRepresentation(TextStream&, int indention) const override;

 private:
  FEDropShadow(Filter*, float, float, float, float, const Color&, float);

  FloatRect MapEffect(const FloatRect&) const override;

  sk_sp<SkImageFilter> CreateImageFilter() override;

  float std_x_;
  float std_y_;
  float dx_;
  float dy_;
  Color shadow_color_;
  float shadow_opacity_;
};

}  // namespace blink

#endif  // FEDropShadow_h
