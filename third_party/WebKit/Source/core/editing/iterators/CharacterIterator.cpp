/*
 * Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2012 Apple Inc. All
 * rights reserved.
 * Copyright (C) 2005 Alexey Proskuryakov.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "core/editing/iterators/CharacterIterator.h"

namespace blink {

template <typename Strategy>
CharacterIteratorAlgorithm<Strategy>::CharacterIteratorAlgorithm(
    const PositionTemplate<Strategy>& start,
    const PositionTemplate<Strategy>& end,
    const TextIteratorBehavior& behavior)
    : offset_(0),
      run_offset_(0),
      at_break_(true),
      text_iterator_(start, end, behavior) {
  Initialize();
}

template <typename Strategy>
CharacterIteratorAlgorithm<Strategy>::CharacterIteratorAlgorithm(
    const EphemeralRangeTemplate<Strategy>& range,
    const TextIteratorBehavior& behavior)
    : CharacterIteratorAlgorithm(range.StartPosition(),
                                 range.EndPosition(),
                                 behavior) {}

template <typename Strategy>
void CharacterIteratorAlgorithm<Strategy>::Initialize() {
  while (!AtEnd() && !text_iterator_.length())
    text_iterator_.Advance();
}

template <typename Strategy>
EphemeralRangeTemplate<Strategy> CharacterIteratorAlgorithm<Strategy>::Range()
    const {
  EphemeralRangeTemplate<Strategy> range(text_iterator_.Range());
  if (text_iterator_.AtEnd() || text_iterator_.length() <= 1)
    return range;
  PositionTemplate<Strategy> start_position =
      range.StartPosition().ParentAnchoredEquivalent();
  PositionTemplate<Strategy> end_position =
      range.EndPosition().ParentAnchoredEquivalent();
  Node* node = start_position.ComputeContainerNode();
  DCHECK_EQ(node, end_position.ComputeContainerNode());
  int offset = start_position.OffsetInContainerNode() + run_offset_;
  return EphemeralRangeTemplate<Strategy>(
      PositionTemplate<Strategy>(node, offset),
      PositionTemplate<Strategy>(node, offset + 1));
}

template <typename Strategy>
Document* CharacterIteratorAlgorithm<Strategy>::OwnerDocument() const {
  return text_iterator_.OwnerDocument();
}

template <typename Strategy>
Node* CharacterIteratorAlgorithm<Strategy>::CurrentContainer() const {
  return text_iterator_.CurrentContainer();
}

template <typename Strategy>
int CharacterIteratorAlgorithm<Strategy>::StartOffset() const {
  if (!text_iterator_.AtEnd()) {
    if (text_iterator_.length() > 1)
      return text_iterator_.StartOffsetInCurrentContainer() + run_offset_;
    DCHECK(!run_offset_);
  }
  return text_iterator_.StartOffsetInCurrentContainer();
}

template <typename Strategy>
int CharacterIteratorAlgorithm<Strategy>::EndOffset() const {
  if (!text_iterator_.AtEnd()) {
    if (text_iterator_.length() > 1)
      return text_iterator_.StartOffsetInCurrentContainer() + run_offset_ + 1;
    DCHECK(!run_offset_);
  }
  return text_iterator_.EndOffsetInCurrentContainer();
}

template <typename Strategy>
PositionTemplate<Strategy> CharacterIteratorAlgorithm<Strategy>::StartPosition()
    const {
  if (!text_iterator_.AtEnd()) {
    if (text_iterator_.length() > 1) {
      Node* n = text_iterator_.CurrentContainer();
      int offset = text_iterator_.StartOffsetInCurrentContainer() + run_offset_;
      return PositionTemplate<Strategy>::EditingPositionOf(n, offset);
    }
    DCHECK(!run_offset_);
  }
  return text_iterator_.StartPositionInCurrentContainer();
}

template <typename Strategy>
PositionTemplate<Strategy> CharacterIteratorAlgorithm<Strategy>::EndPosition()
    const {
  if (!text_iterator_.AtEnd()) {
    if (text_iterator_.length() > 1) {
      Node* n = text_iterator_.CurrentContainer();
      int offset = text_iterator_.StartOffsetInCurrentContainer() + run_offset_;
      return PositionTemplate<Strategy>::EditingPositionOf(n, offset + 1);
    }
    DCHECK(!run_offset_);
  }
  return text_iterator_.EndPositionInCurrentContainer();
}

template <typename Strategy>
void CharacterIteratorAlgorithm<Strategy>::Advance(int count) {
  if (count <= 0) {
    DCHECK(!count);
    return;
  }

  at_break_ = false;

  // easy if there is enough left in the current m_textIterator run
  int remaining = text_iterator_.length() - run_offset_;
  if (count < remaining) {
    run_offset_ += count;
    offset_ += count;
    return;
  }

  // exhaust the current m_textIterator run
  count -= remaining;
  offset_ += remaining;

  // move to a subsequent m_textIterator run
  for (text_iterator_.Advance(); !AtEnd(); text_iterator_.Advance()) {
    int run_length = text_iterator_.length();
    if (!run_length) {
      at_break_ = text_iterator_.BreaksAtReplacedElement();
    } else {
      // see whether this is m_textIterator to use
      if (count < run_length) {
        run_offset_ = count;
        offset_ += count;
        return;
      }

      // exhaust this m_textIterator run
      count -= run_length;
      offset_ += run_length;
    }
  }

  // ran to the end of the m_textIterator... no more runs left
  at_break_ = true;
  run_offset_ = 0;
}

template <typename Strategy>
void CharacterIteratorAlgorithm<Strategy>::CopyTextTo(
    ForwardsTextBuffer* output) {
  text_iterator_.CopyTextTo(output, run_offset_);
}

template <typename Strategy>
EphemeralRangeTemplate<Strategy>
CharacterIteratorAlgorithm<Strategy>::CalculateCharacterSubrange(int offset,
                                                                 int length) {
  Advance(offset);
  const PositionTemplate<Strategy> start_pos = StartPosition();

  if (!length)
    return EphemeralRangeTemplate<Strategy>(start_pos, start_pos);
  if (length > 1)
    Advance(length - 1);
  return EphemeralRangeTemplate<Strategy>(start_pos, EndPosition());
}

EphemeralRange CalculateCharacterSubrange(const EphemeralRange& range,
                                          int character_offset,
                                          int character_count) {
  CharacterIterator entire_range_iterator(
      range, TextIteratorBehavior::Builder()
                 .SetEmitsObjectReplacementCharacter(true)
                 .Build());
  return entire_range_iterator.CalculateCharacterSubrange(character_offset,
                                                          character_count);
}

template class CORE_TEMPLATE_EXPORT CharacterIteratorAlgorithm<EditingStrategy>;
template class CORE_TEMPLATE_EXPORT
    CharacterIteratorAlgorithm<EditingInFlatTreeStrategy>;

}  // namespace blink
