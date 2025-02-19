/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
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

#ifndef LayoutSVGResourceClipper_h
#define LayoutSVGResourceClipper_h

#include "core/layout/svg/LayoutSVGResourceContainer.h"
#include "core/svg/SVGClipPathElement.h"
#include "third_party/skia/include/core/SkRefCnt.h"

namespace blink {

class LayoutSVGResourceClipper final : public LayoutSVGResourceContainer {
 public:
  explicit LayoutSVGResourceClipper(SVGClipPathElement*);
  ~LayoutSVGResourceClipper() override;

  const char* GetName() const override { return "LayoutSVGResourceClipper"; }

  void RemoveAllClientsFromCache(bool mark_for_invalidation = true) override;
  void RemoveClientFromCache(LayoutObject*,
                             bool mark_for_invalidation = true) override;

  FloatRect ResourceBoundingBox(const FloatRect& reference_box);

  static const LayoutSVGResourceType kResourceType = kClipperResourceType;
  LayoutSVGResourceType ResourceType() const override { return kResourceType; }

  bool HitTestClipContent(const FloatRect&, const FloatPoint&);

  SVGUnitTypes::SVGUnitType ClipPathUnits() const {
    return toSVGClipPathElement(GetElement())
        ->clipPathUnits()
        ->CurrentValue()
        ->EnumValue();
  }

  bool AsPath(const AffineTransform&, const FloatRect& reference_box, Path&);
  sk_sp<const PaintRecord> CreatePaintRecord();

  bool HasCycle() { return in_clip_expansion_; }
  void BeginClipExpansion() {
    DCHECK(!in_clip_expansion_);
    in_clip_expansion_ = true;
  }
  void EndClipExpansion() {
    DCHECK(in_clip_expansion_);
    in_clip_expansion_ = false;
  }

 private:
  void CalculateLocalClipBounds();

  // Return true if the clip path was calculated or a cached value is available.
  bool CalculateClipContentPathIfNeeded();

  // Cache of the clip path when using path clipping.
  Path clip_content_path_;

  // Cache of the clip path paint record when falling back to masking for
  // clipping.
  sk_sp<const PaintRecord> cached_paint_record_;

  FloatRect local_clip_bounds_;

  // Reference cycle detection.
  bool in_clip_expansion_;
};

DEFINE_LAYOUT_SVG_RESOURCE_TYPE_CASTS(LayoutSVGResourceClipper,
                                      kClipperResourceType);

}  // namespace blink

#endif
