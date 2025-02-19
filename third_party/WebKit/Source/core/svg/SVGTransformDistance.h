/*
 * Copyright (C) 2007 Eric Seidel <eric@webkit.org>
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

#ifndef SVGTransformDistance_h
#define SVGTransformDistance_h

#include "core/svg/SVGTransform.h"
#include "platform/wtf/Allocator.h"

namespace blink {

class AffineTransform;

class SVGTransformDistance {
  STACK_ALLOCATED();

 public:
  SVGTransformDistance();
  SVGTransformDistance(SVGTransform* from_transform,
                       SVGTransform* to_transform);

  SVGTransformDistance ScaledDistance(float scale_factor) const;
  SVGTransform* AddToSVGTransform(SVGTransform*) const;

  static SVGTransform* AddSVGTransforms(SVGTransform*,
                                        SVGTransform*,
                                        unsigned repeat_count = 1);

  float Distance() const;

 private:
  SVGTransformDistance(SVGTransformType,
                       float angle,
                       float cx,
                       float cy,
                       const AffineTransform&);

  SVGTransformType transform_type_;
  float angle_;
  float cx_;
  float cy_;
  AffineTransform
      transform_;  // for storing scale, translation or matrix transforms
};
}  // namespace blink

#endif  // SVGTransformDistance_h
