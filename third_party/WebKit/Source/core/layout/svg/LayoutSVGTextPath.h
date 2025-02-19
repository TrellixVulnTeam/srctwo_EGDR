/*
 * Copyright (C) 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2009 Apple Inc. All rights reserved.
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

#ifndef LayoutSVGTextPath_h
#define LayoutSVGTextPath_h

#include <memory>
#include "core/layout/svg/LayoutSVGInline.h"
#include "platform/wtf/PtrUtil.h"

namespace blink {

// This class maps a 1D location in the "path space"; [0, path length] to a
// (2D) point on the path and provides the normal (angle from the x-axis) for
// said point.
class PathPositionMapper {
  USING_FAST_MALLOC(PathPositionMapper);

 public:
  static std::unique_ptr<PathPositionMapper> Create(const Path& path) {
    return WTF::WrapUnique(new PathPositionMapper(path));
  }

  enum PositionType {
    kOnPath,
    kBeforePath,
    kAfterPath,
  };
  PositionType PointAndNormalAtLength(float length, FloatPoint&, float& angle);
  float length() const { return path_length_; }

 private:
  explicit PathPositionMapper(const Path&);

  Path::PositionCalculator position_calculator_;
  float path_length_;
};

class LayoutSVGTextPath final : public LayoutSVGInline {
 public:
  explicit LayoutSVGTextPath(Element*);

  std::unique_ptr<PathPositionMapper> LayoutPath() const;
  float CalculateStartOffset(float) const;

  bool IsChildAllowed(LayoutObject*, const ComputedStyle&) const override;

  bool IsOfType(LayoutObjectType type) const override {
    return type == kLayoutObjectSVGTextPath || LayoutSVGInline::IsOfType(type);
  }

  const char* GetName() const override { return "LayoutSVGTextPath"; }
};

DEFINE_LAYOUT_OBJECT_TYPE_CASTS(LayoutSVGTextPath, IsSVGTextPath());

}  // namespace blink

#endif  // LayoutSVGTextPath_h
