// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LayoutMultiColumnSpannerPlaceholder_h
#define LayoutMultiColumnSpannerPlaceholder_h

#include "core/layout/LayoutBlockFlow.h"

namespace blink {

// Placeholder layoutObject for column-span:all elements. The column-span:all
// layoutObject itself is a descendant of the flow thread, but due to its
// out-of-flow nature, we need something on the outside to take care of its
// positioning and sizing. LayoutMultiColumnSpannerPlaceholder objects are
// siblings of LayoutMultiColumnSet objects, i.e. direct children of the
// multicol container.
class LayoutMultiColumnSpannerPlaceholder final : public LayoutBox {
 public:
  bool IsOfType(LayoutObjectType type) const override {
    return type == kLayoutObjectLayoutMultiColumnSpannerPlaceholder ||
           LayoutBox::IsOfType(type);
  }

  static LayoutMultiColumnSpannerPlaceholder* CreateAnonymous(
      const ComputedStyle& parent_style,
      LayoutBox&);

  LayoutMultiColumnFlowThread* FlowThread() const {
    return ToLayoutBlockFlow(Parent())->MultiColumnFlowThread();
  }

  LayoutBox* LayoutObjectInFlowThread() const {
    return layout_object_in_flow_thread_;
  }
  void MarkForLayoutIfObjectInFlowThreadNeedsLayout() {
    if (!layout_object_in_flow_thread_->NeedsLayout())
      return;
    // The containing block of a spanner is the multicol container (our parent
    // here), but the spanner is laid out via its spanner set (us), so we need
    // to make sure that we enter it.
    SetChildNeedsLayout(kMarkOnlyThis);
  }

  void LayoutObjectInFlowThreadStyleDidChange(const ComputedStyle* old_style);
  void UpdateMarginProperties();

  const char* GetName() const override {
    return "LayoutMultiColumnSpannerPlaceholder";
  }

 protected:
  void InsertedIntoTree() override;
  void WillBeRemovedFromTree() override;
  bool NeedsPreferredWidthsRecalculation() const override;
  LayoutUnit MinPreferredLogicalWidth() const override;
  LayoutUnit MaxPreferredLogicalWidth() const override;
  void UpdateLayout() override;
  void ComputeLogicalHeight(LayoutUnit logical_height,
                            LayoutUnit logical_top,
                            LogicalExtentComputedValues&) const override;
  void Paint(const PaintInfo&, const LayoutPoint& paint_offset) const override;
  bool NodeAtPoint(HitTestResult&,
                   const HitTestLocation& location_in_container,
                   const LayoutPoint& accumulated_offset,
                   HitTestAction) override;

 private:
  LayoutMultiColumnSpannerPlaceholder(LayoutBox*);

  // The actual column-span:all layoutObject inside the flow thread.
  LayoutBox* layout_object_in_flow_thread_;
};

DEFINE_LAYOUT_OBJECT_TYPE_CASTS(LayoutMultiColumnSpannerPlaceholder,
                                IsLayoutMultiColumnSpannerPlaceholder());

}  // namespace blink

#endif
