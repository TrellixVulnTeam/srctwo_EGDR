// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FragmentainerIterator_h
#define FragmentainerIterator_h

#include "core/layout/MultiColumnFragmentainerGroup.h"
#include "platform/geometry/LayoutRect.h"

namespace blink {

class LayoutFlowThread;
class LayoutMultiColumnSet;

// Used to find the fragmentainers that intersect with a given portion of the
// flow thread. The portion typically corresponds to the bounds of some
// descendant layout object. The iterator walks in block direction order.
class FragmentainerIterator {
 public:
  // Initialize the iterator, and move to the first fragmentainer of interest.
  // The clip rectangle is optional. If it's empty, it means that no clipping
  // will be performed, and that the only thing that can limit the set of
  // fragmentainers to visit is |physicalBoundingBox|.
  FragmentainerIterator(
      const LayoutFlowThread&,
      const LayoutRect& physical_bounding_box_in_flow_thread,
      const LayoutRect& clip_rect_in_multicol_container = LayoutRect());

  // Advance to the next fragmentainer. Not allowed to call this if atEnd() is
  // true.
  void Advance();

  // Return true if we have walked through all relevant fragmentainers.
  bool AtEnd() const { return !current_column_set_; }

  // The physical translation to apply to shift the box when converting from
  // flowthread to visual coordinates.
  LayoutSize PaginationOffset() const;

  // Return the physical clip rectangle of the current fragmentainer, relative
  // to the flow thread.
  LayoutRect ClipRectInFlowThread(
      MultiColumnFragmentainerGroup::ClipRectAxesSelector =
          MultiColumnFragmentainerGroup::kBothAxes) const;

 private:
  const LayoutFlowThread& flow_thread_;
  const LayoutRect clip_rect_in_multicol_container_;

  const LayoutMultiColumnSet* current_column_set_;
  unsigned current_fragmentainer_group_index_;
  unsigned current_fragmentainer_index_;
  unsigned end_fragmentainer_index_;

  LayoutUnit logical_top_in_flow_thread_;
  LayoutUnit logical_bottom_in_flow_thread_;

  const MultiColumnFragmentainerGroup& CurrentGroup() const;
  void MoveToNextFragmentainerGroup();
  bool SetFragmentainersOfInterest();
  void SetAtEnd() { current_column_set_ = nullptr; }
  bool HasClipRect() const {
    return !clip_rect_in_multicol_container_.IsEmpty();
  }
};

}  // namespace blink

#endif  // FragmentainerIterator_h
