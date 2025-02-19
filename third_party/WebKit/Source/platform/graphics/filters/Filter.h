/*
 * Copyright (C) 2009 Dirk Schulze <krit@webkit.org>
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

#ifndef Filter_h
#define Filter_h

#include "platform/PlatformExport.h"
#include "platform/geometry/FloatPoint3D.h"
#include "platform/geometry/FloatRect.h"
#include "platform/geometry/IntRect.h"
#include "platform/heap/Handle.h"
#include "platform/wtf/Noncopyable.h"

namespace blink {

class SourceGraphic;
class FilterEffect;

class PLATFORM_EXPORT Filter final : public GarbageCollected<Filter> {
  WTF_MAKE_NONCOPYABLE(Filter);

 public:
  enum UnitScaling { kUserSpace, kBoundingBox };

  static Filter* Create(const FloatRect& reference_box,
                        const FloatRect& filter_region,
                        float scale,
                        UnitScaling);
  static Filter* Create(float scale);

  DECLARE_TRACE();

  float Scale() const { return scale_; }
  FloatRect MapLocalRectToAbsoluteRect(const FloatRect&) const;
  FloatRect MapAbsoluteRectToLocalRect(const FloatRect&) const;

  float ApplyHorizontalScale(float value) const;
  float ApplyVerticalScale(float value) const;

  FloatPoint3D Resolve3dPoint(const FloatPoint3D&) const;

  FloatRect AbsoluteFilterRegion() const {
    return MapLocalRectToAbsoluteRect(filter_region_);
  }

  const FloatRect& FilterRegion() const { return filter_region_; }
  const FloatRect& ReferenceBox() const { return reference_box_; }

  void SetLastEffect(FilterEffect*);
  FilterEffect* LastEffect() const { return last_effect_.Get(); }

  SourceGraphic* GetSourceGraphic() const { return source_graphic_.Get(); }

 private:
  Filter(const FloatRect& reference_box,
         const FloatRect& filter_region,
         float scale,
         UnitScaling);

  FloatRect reference_box_;
  FloatRect filter_region_;
  float scale_;
  UnitScaling unit_scaling_;

  Member<SourceGraphic> source_graphic_;
  Member<FilterEffect> last_effect_;
};

}  // namespace blink

#endif  // Filter_h
