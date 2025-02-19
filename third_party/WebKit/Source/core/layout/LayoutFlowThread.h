/*
 * Copyright (C) 2011 Adobe Systems Incorporated. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef LayoutFlowThread_h
#define LayoutFlowThread_h

#include "core/CoreExport.h"
#include "core/layout/LayoutBlockFlow.h"
#include "platform/wtf/ListHashSet.h"

namespace blink {

class LayoutMultiColumnSet;

typedef ListHashSet<LayoutMultiColumnSet*> LayoutMultiColumnSetList;

// Layout state for multicol. To be stored when laying out a block child, so
// that we can roll back to the initial state if we need to re-lay out said
// block child.
class MultiColumnLayoutState {
  friend class LayoutMultiColumnFlowThread;

 public:
  MultiColumnLayoutState() : column_set_(nullptr) {}

 private:
  explicit MultiColumnLayoutState(LayoutMultiColumnSet* column_set)
      : column_set_(column_set) {}
  LayoutMultiColumnSet* ColumnSet() const { return column_set_; }

  LayoutMultiColumnSet* column_set_;
};

// LayoutFlowThread is used to collect all the layout objects that participate
// in a flow thread. It will also help in doing the layout. However, it will not
// layout directly to screen. Instead, LayoutMultiColumnSet objects will
// redirect their paint and nodeAtPoint methods to this object. Each
// LayoutMultiColumnSet will actually be a viewPort of the LayoutFlowThread.
class CORE_EXPORT LayoutFlowThread : public LayoutBlockFlow {
 public:
  LayoutFlowThread();
  ~LayoutFlowThread() override {}

  bool IsLayoutFlowThread() const final { return true; }
  virtual bool IsLayoutMultiColumnFlowThread() const { return false; }
  virtual bool IsLayoutPagedFlowThread() const { return false; }

  // Search mode when looking for an enclosing fragmentation context.
  enum AncestorSearchConstraint {
    // No constraints. When we're not laying out (but rather e.g. painting or
    // hit-testing), we just want to find all enclosing fragmentation contexts,
    // e.g. to calculate the accumulated visual translation.
    kAnyAncestor,

    // Consider fragmentation contexts that are strictly unbreakable (seen from
    // the outside) to be isolated from the rest, so that such fragmentation
    // contexts don't participate in fragmentation of enclosing fragmentation
    // contexts, apart from taking up space and otherwise being completely
    // unbreakable. This is typically what we want to do during layout.
    kIsolateUnbreakableContainers,
  };

  static LayoutFlowThread* LocateFlowThreadContainingBlockOf(
      const LayoutObject&,
      AncestorSearchConstraint);

  void UpdateLayout() override;

  // Always create a Layer for the LayoutFlowThread so that we
  // can easily avoid drawing the children directly.
  PaintLayerType LayerTypeRequired() const final { return kNormalPaintLayer; }

  virtual void FlowThreadDescendantWasInserted(LayoutObject*) {}
  virtual void FlowThreadDescendantWillBeRemoved(LayoutObject*) {}
  virtual void FlowThreadDescendantStyleWillChange(
      LayoutBox*,
      StyleDifference,
      const ComputedStyle& new_style) {}
  virtual void FlowThreadDescendantStyleDidChange(
      LayoutBox*,
      StyleDifference,
      const ComputedStyle& old_style) {}

  void AbsoluteQuadsForDescendant(const LayoutBox& descendant,
                                  Vector<FloatQuad>&,
                                  MapCoordinatesFlags mode = 0);

  void AddOutlineRects(Vector<LayoutRect>&,
                       const LayoutPoint& additional_offset,
                       IncludeBlockVisualOverflowOrNot) const override;

  bool NodeAtPoint(HitTestResult&,
                   const HitTestLocation& location_in_container,
                   const LayoutPoint& accumulated_offset,
                   HitTestAction) final;

  virtual void AddColumnSetToThread(LayoutMultiColumnSet*) = 0;
  virtual void RemoveColumnSetFromThread(LayoutMultiColumnSet*);

  void ComputeLogicalHeight(LayoutUnit logical_height,
                            LayoutUnit logical_top,
                            LogicalExtentComputedValues&) const override;

  bool HasColumnSets() const { return multi_column_set_list_.size(); }

  void ValidateColumnSets();
  void InvalidateColumnSets() { column_sets_invalidated_ = true; }
  bool HasValidColumnSetInfo() const {
    return !column_sets_invalidated_ && !multi_column_set_list_.IsEmpty();
  }

  bool MapToVisualRectInAncestorSpaceInternal(
      const LayoutBoxModelObject* ancestor,
      TransformState&,
      VisualRectFlags = kDefaultVisualRectFlags) const override;

  LayoutUnit PageLogicalHeightForOffset(LayoutUnit);
  LayoutUnit PageRemainingLogicalHeightForOffset(LayoutUnit, PageBoundaryRule);

  virtual void ContentWasLaidOut(
      LayoutUnit logical_bottom_in_flow_thread_after_pagination) = 0;
  virtual bool CanSkipLayout(const LayoutBox&) const = 0;

  virtual MultiColumnLayoutState GetMultiColumnLayoutState() const = 0;
  virtual void RestoreMultiColumnLayoutState(const MultiColumnLayoutState&) = 0;

  // Find and return the next logical top after |flowThreadOffset| that can fit
  // unbreakable content as tall as |contentLogicalHeight|. |flowThreadOffset|
  // is expected to be at the exact top of a column that's known to not have
  // enough space for |contentLogicalHeight|. This method is called when the
  // current column is too short to fit the content, in the hope that there
  // exists one that's tall enough further ahead. If no such column can be
  // found, |flowThreadOffset| will be returned.
  LayoutUnit NextLogicalTopForUnbreakableContent(
      LayoutUnit flow_thread_offset,
      LayoutUnit content_logical_height) const;

  virtual bool IsPageLogicalHeightKnown() const { return true; }
  virtual bool MayHaveNonUniformPageLogicalHeight() const = 0;
  bool PageLogicalSizeChanged() const { return page_logical_size_changed_; }

  // Return the visual bounding box based on the supplied flow-thread bounding
  // box. Both rectangles are completely physical in terms of writing mode.
  LayoutRect FragmentsBoundingBox(const LayoutRect& layer_bounding_box) const;

  // Convert a logical position in the flow thread coordinate space to a logical
  // position in the containing coordinate space.
  void FlowThreadToContainingCoordinateSpace(LayoutUnit& block_position,
                                             LayoutUnit& inline_position) const;

  virtual LayoutPoint FlowThreadPointToVisualPoint(
      const LayoutPoint& flow_thread_point) const = 0;
  virtual LayoutPoint VisualPointToFlowThreadPoint(
      const LayoutPoint& visual_point) const = 0;

  virtual LayoutMultiColumnSet* ColumnSetAtBlockOffset(
      LayoutUnit,
      PageBoundaryRule) const = 0;

  virtual const char* GetName() const = 0;

 protected:
  void GenerateColumnSetIntervalTree();

  LayoutMultiColumnSetList multi_column_set_list_;

  typedef PODInterval<LayoutUnit, LayoutMultiColumnSet*> MultiColumnSetInterval;
  typedef PODIntervalTree<LayoutUnit, LayoutMultiColumnSet*>
      MultiColumnSetIntervalTree;

  class MultiColumnSetSearchAdapter {
   public:
    MultiColumnSetSearchAdapter(LayoutUnit offset)
        : offset_(offset), result_(nullptr) {}

    const LayoutUnit& LowValue() const { return offset_; }
    const LayoutUnit& HighValue() const { return offset_; }
    void CollectIfNeeded(const MultiColumnSetInterval&);

    LayoutMultiColumnSet* Result() const { return result_; }

   private:
    LayoutUnit offset_;
    LayoutMultiColumnSet* result_;
  };

  MultiColumnSetIntervalTree multi_column_set_interval_tree_;

  bool column_sets_invalidated_ : 1;
  bool page_logical_size_changed_ : 1;
};

DEFINE_LAYOUT_OBJECT_TYPE_CASTS(LayoutFlowThread, IsLayoutFlowThread());

// These structures are used by PODIntervalTree for debugging.
#ifndef NDEBUG
template <>
struct ValueToString<LayoutMultiColumnSet*> {
  static String ToString(const LayoutMultiColumnSet* value) {
    return String::Format("%p", value);
  }
};
#endif

}  // namespace blink

#endif  // LayoutFlowThread_h
