// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ForwardGraphemeBoundaryStateMachine_h
#define ForwardGraphemeBoundaryStateMachine_h

#include <iosfwd>
#include "core/CoreExport.h"
#include "core/editing/state_machines/TextSegmentationMachineState.h"
#include "platform/wtf/Allocator.h"
#include "platform/wtf/Noncopyable.h"
#include "platform/wtf/text/Unicode.h"

namespace blink {

class CORE_EXPORT ForwardGraphemeBoundaryStateMachine {
  STACK_ALLOCATED();
  WTF_MAKE_NONCOPYABLE(ForwardGraphemeBoundaryStateMachine);

 public:
  ForwardGraphemeBoundaryStateMachine();

  // Find boundary offset by feeding preceding text.
  // This method must not be called after feedFollowingCodeUnit().
  TextSegmentationMachineState FeedPrecedingCodeUnit(UChar code_unit);

  // Tells the end of preceding text to the state machine.
  TextSegmentationMachineState TellEndOfPrecedingText();

  // Find boundary offset by feeding following text.
  // This method must be called after feedPrecedingCodeUnit() returns
  // NeedsFollowingCodeUnit.
  TextSegmentationMachineState FeedFollowingCodeUnit(UChar code_unit);

  // Returns the next boundary offset. This method finalizes the state machine
  // if it is not finished.
  int FinalizeAndGetBoundaryOffset();

  // Resets the internal state to the initial state.
  void Reset();

 private:
  enum class InternalState;
  friend std::ostream& operator<<(std::ostream&, InternalState);

  TextSegmentationMachineState MoveToNextState(InternalState);

  TextSegmentationMachineState StaySameState();

  // Updates the internal state to InternalState::Finished then
  // returnsTextSegmentationMachineState::Finished.
  TextSegmentationMachineState Finish();

  // Handles end of text. This method always finishes the state machine.
  void FinishWithEndOfText();

  // Used for composing supplementary code point with surrogate pairs.
  UChar pending_code_unit_ = 0;

  // The code point immediately before the m_BoundaryOffset.
  UChar32 prev_code_point_;

  // The relative offset from the begging of this state machine.
  int boundary_offset_ = 0;

  // The number of regional indicator symbols preceding to the begging offset.
  int preceding_ris_count_ = 0;

  // The internal state.
  InternalState internal_state_;
};

}  // namespace blink

#endif
