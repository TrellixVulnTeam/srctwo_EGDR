/*
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2009 Dirk Schulze <krit@webkit.org>
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
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

#include "platform/graphics/filters/FEOffset.h"

#include "SkOffsetImageFilter.h"
#include "platform/graphics/filters/Filter.h"
#include "platform/graphics/filters/SkiaImageFilterBuilder.h"
#include "platform/text/TextStream.h"

namespace blink {

FEOffset::FEOffset(Filter* filter, float dx, float dy)
    : FilterEffect(filter), dx_(dx), dy_(dy) {}

FEOffset* FEOffset::Create(Filter* filter, float dx, float dy) {
  return new FEOffset(filter, dx, dy);
}

float FEOffset::Dx() const {
  return dx_;
}

void FEOffset::SetDx(float dx) {
  dx_ = dx;
}

float FEOffset::Dy() const {
  return dy_;
}

void FEOffset::SetDy(float dy) {
  dy_ = dy;
}

FloatRect FEOffset::MapEffect(const FloatRect& rect) const {
  FloatRect result = rect;
  result.Move(GetFilter()->ApplyHorizontalScale(dx_),
              GetFilter()->ApplyVerticalScale(dy_));
  return result;
}

sk_sp<SkImageFilter> FEOffset::CreateImageFilter() {
  Filter* filter = this->GetFilter();
  SkImageFilter::CropRect crop_rect = GetCropRect();
  return SkOffsetImageFilter::Make(
      SkFloatToScalar(filter->ApplyHorizontalScale(dx_)),
      SkFloatToScalar(filter->ApplyVerticalScale(dy_)),
      SkiaImageFilterBuilder::Build(InputEffect(0),
                                    OperatingInterpolationSpace()),
      &crop_rect);
}

TextStream& FEOffset::ExternalRepresentation(TextStream& ts, int indent) const {
  WriteIndent(ts, indent);
  ts << "[feOffset";
  FilterEffect::ExternalRepresentation(ts);
  ts << " dx=\"" << Dx() << "\" dy=\"" << Dy() << "\"]\n";
  InputEffect(0)->ExternalRepresentation(ts, indent + 1);
  return ts;
}

}  // namespace blink
