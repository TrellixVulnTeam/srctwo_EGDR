/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "core/editing/PositionIterator.h"

#include "core/editing/EditingUtilities.h"

namespace blink {

namespace {

// TODO(editing-dev): We should replace usages of |hasChildren()| in
// |PositionIterator| to |shouldTraverseChildren()|.
template <typename Strategy>
bool ShouldTraverseChildren(const Node& node) {
  return Strategy::HasChildren(node) && !IsUserSelectContain(node);
}

// TODO(editing-dev): We should replace usages of |parent()| in
// |PositionIterator| to |selectableParentOf()|.
template <typename Strategy>
ContainerNode* SelectableParentOf(const Node& node) {
  ContainerNode* const parent = Strategy::Parent(node);
  return parent && !IsUserSelectContain(*parent) ? parent : nullptr;
}

}  // namespace

static const int kInvalidOffset = -1;

template <typename Strategy>
PositionIteratorAlgorithm<Strategy>::PositionIteratorAlgorithm(
    Node* anchor_node,
    int offset_in_anchor)
    : anchor_node_(anchor_node),
      node_after_position_in_anchor_(
          Strategy::ChildAt(*anchor_node, offset_in_anchor)),
      offset_in_anchor_(node_after_position_in_anchor_ ? 0 : offset_in_anchor),
      depth_to_anchor_node_(0),
      dom_tree_version_(anchor_node->GetDocument().DomTreeVersion()) {
  for (Node* node = SelectableParentOf<Strategy>(*anchor_node); node;
       node = SelectableParentOf<Strategy>(*node)) {
    // Each m_offsetsInAnchorNode[offset] should be an index of node in
    // parent, but delay to calculate the index until it is needed for
    // performance.
    offsets_in_anchor_node_.push_back(kInvalidOffset);
    ++depth_to_anchor_node_;
  }
  if (node_after_position_in_anchor_)
    offsets_in_anchor_node_.push_back(offset_in_anchor);
}
template <typename Strategy>
PositionIteratorAlgorithm<Strategy>::PositionIteratorAlgorithm(
    const PositionTemplate<Strategy>& pos)
    : PositionIteratorAlgorithm(pos.AnchorNode(), pos.ComputeEditingOffset()) {}

template <typename Strategy>
PositionIteratorAlgorithm<Strategy>::PositionIteratorAlgorithm()
    : anchor_node_(nullptr),
      node_after_position_in_anchor_(nullptr),
      offset_in_anchor_(0),
      depth_to_anchor_node_(0),
      dom_tree_version_(0) {}

template <typename Strategy>
PositionTemplate<Strategy>
PositionIteratorAlgorithm<Strategy>::DeprecatedComputePosition() const {
  // TODO(yoichio): Share code to check domTreeVersion with EphemeralRange.
  DCHECK(IsValid());
  if (node_after_position_in_anchor_) {
    DCHECK_EQ(Strategy::Parent(*node_after_position_in_anchor_), anchor_node_);
    DCHECK_NE(offsets_in_anchor_node_[depth_to_anchor_node_], kInvalidOffset);
    // FIXME: This check is inadaquete because any ancestor could be ignored by
    // editing
    if (EditingIgnoresContent(
            *Strategy::Parent(*node_after_position_in_anchor_)))
      return PositionTemplate<Strategy>::BeforeNode(*anchor_node_);
    return PositionTemplate<Strategy>(
        anchor_node_, offsets_in_anchor_node_[depth_to_anchor_node_]);
  }
  if (Strategy::HasChildren(*anchor_node_))
    return PositionTemplate<Strategy>::LastPositionInOrAfterNode(anchor_node_);
  return PositionTemplate<Strategy>::EditingPositionOf(anchor_node_,
                                                       offset_in_anchor_);
}

template <typename Strategy>
PositionTemplate<Strategy>
PositionIteratorAlgorithm<Strategy>::ComputePosition() const {
  DCHECK(IsValid());
  // Assume that we have the following DOM tree:
  // A
  // |-B
  // | |-E
  // | +-F
  // |
  // |-C
  // +-D
  //   |-G
  //   +-H
  if (node_after_position_in_anchor_) {
    // For example, position is before E, F.
    DCHECK_EQ(Strategy::Parent(*node_after_position_in_anchor_), anchor_node_);
    DCHECK_NE(offsets_in_anchor_node_[depth_to_anchor_node_], kInvalidOffset);
    // TODO(yoichio): This should be equivalent to
    // PositionTemplate<Strategy>(m_anchorNode,
    // PositionAnchorType::BeforeAnchor);
    return PositionTemplate<Strategy>(
        anchor_node_, offsets_in_anchor_node_[depth_to_anchor_node_]);
  }
  if (ShouldTraverseChildren<Strategy>(*anchor_node_))
    // For example, position is the end of B.
    return PositionTemplate<Strategy>::LastPositionInOrAfterNode(anchor_node_);
  if (anchor_node_->IsTextNode())
    return PositionTemplate<Strategy>(anchor_node_, offset_in_anchor_);
  if (offset_in_anchor_)
    // For example, position is after G.
    return PositionTemplate<Strategy>(anchor_node_,
                                      PositionAnchorType::kAfterAnchor);
  // For example, position is before G.
  return PositionTemplate<Strategy>(anchor_node_,
                                    PositionAnchorType::kBeforeAnchor);
}

template <typename Strategy>
void PositionIteratorAlgorithm<Strategy>::Increment() {
  DCHECK(IsValid());
  if (!anchor_node_)
    return;

  // Assume that we have the following DOM tree:
  // A
  // |-B
  // | |-E
  // | +-F
  // |
  // |-C
  // +-D
  //   |-G
  //   +-H
  // Let |anchor| as |m_anchorNode| and
  // |child| as |m_nodeAfterPositionInAnchor|.
  if (node_after_position_in_anchor_) {
    // Case #1: Move to position before the first child of
    // |m_nodeAfterPositionInAnchor|.
    // This is a point just before |child|.
    // Let |anchor| is A and |child| is B,
    // then next |anchor| is B and |child| is E.
    anchor_node_ = node_after_position_in_anchor_;
    node_after_position_in_anchor_ =
        ShouldTraverseChildren<Strategy>(*anchor_node_)
            ? Strategy::FirstChild(*anchor_node_)
            : nullptr;
    offset_in_anchor_ = 0;
    // Increment depth intializing with 0.
    ++depth_to_anchor_node_;
    if (depth_to_anchor_node_ == offsets_in_anchor_node_.size())
      offsets_in_anchor_node_.push_back(0);
    else
      offsets_in_anchor_node_[depth_to_anchor_node_] = 0;
    return;
  }

  if (anchor_node_->GetLayoutObject() &&
      !ShouldTraverseChildren<Strategy>(*anchor_node_) &&
      offset_in_anchor_ < Strategy::LastOffsetForEditing(anchor_node_)) {
    // Case #2. This is the next of Case #1 or #2 itself.
    // Position is (|anchor|, |m_offsetInAchor|).
    // In this case |anchor| is a leaf(E,F,C,G or H) and
    // |m_offsetInAnchor| is not on the end of |anchor|.
    // Then just increment |m_offsetInAnchor|.
    offset_in_anchor_ = NextGraphemeBoundaryOf(anchor_node_, offset_in_anchor_);
  } else {
    // Case #3. This is the next of Case #2 or #3.
    // Position is the end of |anchor|.
    // 3-a. If |anchor| has next sibling (let E),
    //      next |anchor| is B and |child| is F (next is Case #1.)
    // 3-b. If |anchor| doesn't have next sibling (let F),
    //      next |anchor| is B and |child| is null. (next is Case #3.)
    node_after_position_in_anchor_ = anchor_node_;
    anchor_node_ =
        SelectableParentOf<Strategy>(*node_after_position_in_anchor_);
    if (!anchor_node_)
      return;
    DCHECK_GT(depth_to_anchor_node_, 0u);
    --depth_to_anchor_node_;
    // Increment offset of |child| or initialize if it have never been
    // used.
    if (offsets_in_anchor_node_[depth_to_anchor_node_] == kInvalidOffset)
      offsets_in_anchor_node_[depth_to_anchor_node_] =
          Strategy::Index(*node_after_position_in_anchor_) + 1;
    else
      ++offsets_in_anchor_node_[depth_to_anchor_node_];
    node_after_position_in_anchor_ =
        Strategy::NextSibling(*node_after_position_in_anchor_);
    offset_in_anchor_ = 0;
  }
}

template <typename Strategy>
void PositionIteratorAlgorithm<Strategy>::Decrement() {
  DCHECK(IsValid());
  if (!anchor_node_)
    return;

  // Assume that we have the following DOM tree:
  // A
  // |-B
  // | |-E
  // | +-F
  // |
  // |-C
  // +-D
  //   |-G
  //   +-H
  // Let |anchor| as |m_anchorNode| and
  // |child| as |m_nodeAfterPositionInAnchor|.
  // decrement() is complex but logically reverse of increment(), of course:)
  if (node_after_position_in_anchor_) {
    anchor_node_ = Strategy::PreviousSibling(*node_after_position_in_anchor_);
    if (anchor_node_) {
      // Case #1-a. This is a revese of increment()::Case#3-a.
      // |child| has a previous sibling.
      // Let |anchor| is B and |child| is F,
      // next |anchor| is E and |child| is null.
      node_after_position_in_anchor_ = nullptr;
      offset_in_anchor_ = ShouldTraverseChildren<Strategy>(*anchor_node_)
                              ? 0
                              : Strategy::LastOffsetForEditing(anchor_node_);
      // Decrement offset of |child| or initialize if it have never been
      // used.
      if (offsets_in_anchor_node_[depth_to_anchor_node_] == kInvalidOffset)
        offsets_in_anchor_node_[depth_to_anchor_node_] =
            Strategy::Index(*node_after_position_in_anchor_);
      else
        --offsets_in_anchor_node_[depth_to_anchor_node_];
      DCHECK_GE(offsets_in_anchor_node_[depth_to_anchor_node_], 0);
      // Increment depth intializing with last offset.
      ++depth_to_anchor_node_;
      if (depth_to_anchor_node_ >= offsets_in_anchor_node_.size())
        offsets_in_anchor_node_.push_back(offset_in_anchor_);
      else
        offsets_in_anchor_node_[depth_to_anchor_node_] = offset_in_anchor_;
      return;
    } else {
      // Case #1-b. This is a revese of increment()::Case#1.
      // |child| doesn't have a previous sibling.
      // Let |anchor| is B and |child| is E,
      // next |anchor| is A and |child| is B.
      node_after_position_in_anchor_ =
          Strategy::Parent(*node_after_position_in_anchor_);
      anchor_node_ =
          SelectableParentOf<Strategy>(*node_after_position_in_anchor_);
      if (!anchor_node_)
        return;
      offset_in_anchor_ = 0;
      // Decrement depth and intialize if needs.
      DCHECK_GT(depth_to_anchor_node_, 0u);
      --depth_to_anchor_node_;
      if (offsets_in_anchor_node_[depth_to_anchor_node_] == kInvalidOffset)
        offsets_in_anchor_node_[depth_to_anchor_node_] =
            Strategy::Index(*node_after_position_in_anchor_);
    }
    return;
  }

  if (ShouldTraverseChildren<Strategy>(*anchor_node_)) {
    // Case #2. This is a reverse of increment()::Case3-b.
    // Let |anchor| is B, next |anchor| is F.
    anchor_node_ = Strategy::LastChild(*anchor_node_);
    offset_in_anchor_ = ShouldTraverseChildren<Strategy>(*anchor_node_)
                            ? 0
                            : Strategy::LastOffsetForEditing(anchor_node_);
    // Decrement depth initializing with -1 because
    // |m_nodeAfterPositionInAnchor| is null so still unneeded.
    if (depth_to_anchor_node_ >= offsets_in_anchor_node_.size())
      offsets_in_anchor_node_.push_back(kInvalidOffset);
    else
      offsets_in_anchor_node_[depth_to_anchor_node_] = kInvalidOffset;
    ++depth_to_anchor_node_;
    return;
  }
  if (offset_in_anchor_ && anchor_node_->GetLayoutObject()) {
    // Case #3-a. This is a reverse of increment()::Case#2.
    // In this case |anchor| is a leaf(E,F,C,G or H) and
    // |m_offsetInAnchor| is not on the beginning of |anchor|.
    // Then just decrement |m_offsetInAnchor|.
    offset_in_anchor_ =
        PreviousGraphemeBoundaryOf(anchor_node_, offset_in_anchor_);
    return;
  }
  // Case #3-b. This is a reverse of increment()::Case#1.
  // In this case |anchor| is a leaf(E,F,C,G or H) and
  // |m_offsetInAnchor| is on the beginning of |anchor|.
  // Let |anchor| is E,
  // next |anchor| is B and |child| is E.
  node_after_position_in_anchor_ = anchor_node_;
  anchor_node_ = SelectableParentOf<Strategy>(*anchor_node_);
  if (!anchor_node_)
    return;
  DCHECK_GT(depth_to_anchor_node_, 0u);
  --depth_to_anchor_node_;
  if (offsets_in_anchor_node_[depth_to_anchor_node_] != kInvalidOffset)
    return;
  offsets_in_anchor_node_[depth_to_anchor_node_] =
      Strategy::Index(*node_after_position_in_anchor_);
}

template <typename Strategy>
bool PositionIteratorAlgorithm<Strategy>::AtStart() const {
  DCHECK(IsValid());
  if (!anchor_node_)
    return true;
  if (Strategy::Parent(*anchor_node_))
    return false;
  return (!Strategy::HasChildren(*anchor_node_) && !offset_in_anchor_) ||
         (node_after_position_in_anchor_ &&
          !Strategy::PreviousSibling(*node_after_position_in_anchor_));
}

template <typename Strategy>
bool PositionIteratorAlgorithm<Strategy>::AtEnd() const {
  DCHECK(IsValid());
  if (!anchor_node_)
    return true;
  if (node_after_position_in_anchor_)
    return false;
  return !Strategy::Parent(*anchor_node_) &&
         (Strategy::HasChildren(*anchor_node_) ||
          offset_in_anchor_ >= Strategy::LastOffsetForEditing(anchor_node_));
}

template <typename Strategy>
bool PositionIteratorAlgorithm<Strategy>::AtStartOfNode() const {
  DCHECK(IsValid());
  if (!anchor_node_)
    return true;
  if (!node_after_position_in_anchor_)
    return !Strategy::HasChildren(*anchor_node_) && !offset_in_anchor_;
  return !Strategy::PreviousSibling(*node_after_position_in_anchor_);
}

template <typename Strategy>
bool PositionIteratorAlgorithm<Strategy>::AtEndOfNode() const {
  DCHECK(IsValid());
  if (!anchor_node_)
    return true;
  if (node_after_position_in_anchor_)
    return false;
  return Strategy::HasChildren(*anchor_node_) ||
         offset_in_anchor_ >= Strategy::LastOffsetForEditing(anchor_node_);
}

template class CORE_TEMPLATE_EXPORT PositionIteratorAlgorithm<EditingStrategy>;
template class CORE_TEMPLATE_EXPORT
    PositionIteratorAlgorithm<EditingInFlatTreeStrategy>;

}  // namespace blink
