// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IntersectionGeometry_h
#define IntersectionGeometry_h

#include "platform/Length.h"
#include "platform/geometry/LayoutRect.h"
#include "platform/heap/Handle.h"
#include "platform/wtf/Vector.h"

namespace blink {

class Element;
class LayoutObject;

// Computes the intersection between an ancestor (root) element and a
// descendant (target) element, with overflow and CSS clipping applied, but not
// paint occlusion.
//
// If the root argument to the constructor is null, computes the intersection
// of the target with the top-level frame viewport (AKA the "implicit root").
class IntersectionGeometry {
  STACK_ALLOCATED();

 public:
  IntersectionGeometry(Element* root,
                       Element& target,
                       const Vector<Length>& root_margin,
                       bool should_report_root_bounds);
  ~IntersectionGeometry();

  void ComputeGeometry();

  LayoutObject* Root() const { return root_; }
  LayoutObject* Target() const { return target_; }

  // Client rect in the coordinate system of the frame containing target.
  LayoutRect TargetRect() const { return target_rect_; }

  // Client rect in the coordinate system of the frame containing target.
  LayoutRect IntersectionRect() const { return intersection_rect_; }

  // Client rect in the coordinate system of the frame containing root.
  LayoutRect RootRect() const { return root_rect_; }

  bool DoesIntersect() const { return does_intersect_; }

  IntRect IntersectionIntRect() const {
    return PixelSnappedIntRect(intersection_rect_);
  }

  IntRect TargetIntRect() const { return PixelSnappedIntRect(target_rect_); }

  IntRect RootIntRect() const { return PixelSnappedIntRect(root_rect_); }

 private:
  bool InitializeCanComputeGeometry(Element* root, Element& target) const;
  void InitializeGeometry();
  void InitializeTargetRect();
  void InitializeRootRect();
  void ClipToRoot();
  void MapTargetRectToTargetFrameCoordinates();
  void MapRootRectToRootFrameCoordinates();
  void MapIntersectionRectToTargetFrameCoordinates();
  void ApplyRootMargin();

  // Returns true iff it's possible to compute an intersection between root
  // and target.
  bool CanComputeGeometry() const { return can_compute_geometry_; }
  bool RootIsImplicit() const { return root_is_implicit_; }
  bool ShouldReportRootBounds() const { return should_report_root_bounds_; }

  LayoutObject* root_;
  LayoutObject* target_;
  const Vector<Length> root_margin_;
  LayoutRect target_rect_;
  LayoutRect intersection_rect_;
  LayoutRect root_rect_;
  unsigned does_intersect_ : 1;
  const unsigned should_report_root_bounds_ : 1;
  const unsigned root_is_implicit_ : 1;
  const unsigned can_compute_geometry_ : 1;
};

}  // namespace blink

#endif  // IntersectionGeometry_h
