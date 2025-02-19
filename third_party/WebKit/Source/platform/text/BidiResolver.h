/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2003, 2004, 2006, 2007, 2008 Apple Inc.  All right reserved.
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
 *
 */

#ifndef BidiResolver_h
#define BidiResolver_h

#include "platform/text/BidiCharacterRun.h"
#include "platform/text/BidiContext.h"
#include "platform/text/BidiRunList.h"
#include "platform/text/TextDirection.h"
#include "platform/wtf/Allocator.h"
#include "platform/wtf/HashMap.h"
#include "platform/wtf/Noncopyable.h"
#include "platform/wtf/Vector.h"

namespace blink {

template <class Iterator>
class MidpointState final {
  DISALLOW_NEW();

 public:
  MidpointState() { Reset(); }

  void Reset() {
    num_midpoints_ = 0;
    current_midpoint_ = 0;
    between_midpoints_ = false;
  }

  void StartIgnoringSpaces(const Iterator& midpoint) {
    DCHECK(!(num_midpoints_ % 2));
    AddMidpoint(midpoint);
  }

  void StopIgnoringSpaces(const Iterator& midpoint) {
    DCHECK(num_midpoints_ % 2);
    AddMidpoint(midpoint);
  }

  // Adding a pair of midpoints before a character will split it out into a new
  // line box.
  void EnsureCharacterGetsLineBox(Iterator& text_paragraph_separator) {
    StartIgnoringSpaces(Iterator(0,
                                 text_paragraph_separator.GetLineLayoutItem(),
                                 text_paragraph_separator.Offset() - 1));
    StopIgnoringSpaces(Iterator(0, text_paragraph_separator.GetLineLayoutItem(),
                                text_paragraph_separator.Offset()));
  }

  void CheckMidpoints(Iterator& l_break) {
    // Check to see if our last midpoint is a start point beyond the line break.
    // If so, shave it off the list, and shave off a trailing space if the
    // previous end point doesn't preserve whitespace.
    if (l_break.GetLineLayoutItem() && num_midpoints_ &&
        !(num_midpoints_ % 2)) {
      Iterator* midpoints_iterator = midpoints_.data();
      Iterator& endpoint = midpoints_iterator[num_midpoints_ - 2];
      const Iterator& startpoint = midpoints_iterator[num_midpoints_ - 1];
      Iterator currpoint = endpoint;
      while (!currpoint.AtEnd() && currpoint != startpoint &&
             currpoint != l_break)
        currpoint.Increment();
      if (currpoint == l_break) {
        // We hit the line break before the start point. Shave off the start
        // point.
        num_midpoints_--;
        if (endpoint.GetLineLayoutItem().Style()->CollapseWhiteSpace() &&
            endpoint.GetLineLayoutItem().IsText())
          endpoint.SetOffset(endpoint.Offset() - 1);
      }
    }
  }

  Vector<Iterator>& Midpoints() { return midpoints_; }
  const unsigned& NumMidpoints() const { return num_midpoints_; }
  const unsigned& CurrentMidpoint() const { return current_midpoint_; }
  void IncrementCurrentMidpoint() { current_midpoint_++; }
  const bool& BetweenMidpoints() const { return between_midpoints_; }
  void SetBetweenMidpoints(bool between_midpoint) {
    between_midpoints_ = between_midpoint;
  }

 private:
  // The goal is to reuse the line state across multiple
  // lines so we just keep an array around for midpoints and never clear it
  // across multiple lines. We track the number of items and position using the
  // two other variables.
  Vector<Iterator> midpoints_;
  unsigned num_midpoints_;
  unsigned current_midpoint_;
  bool between_midpoints_;

  void AddMidpoint(const Iterator& midpoint) {
    if (midpoints_.size() <= num_midpoints_)
      midpoints_.Grow(num_midpoints_ + 10);

    Iterator* midpoints_iterator = midpoints_.data();
    midpoints_iterator[num_midpoints_++] = midpoint;
  }
};

// The BidiStatus at a given position (typically the end of a line) can
// be cached and then used to restart bidi resolution at that position.
struct BidiStatus final {
  DISALLOW_NEW();
  BidiStatus()
      : eor(WTF::Unicode::kOtherNeutral),
        last_strong(WTF::Unicode::kOtherNeutral),
        last(WTF::Unicode::kOtherNeutral) {}

  // Creates a BidiStatus representing a new paragraph root with a default
  // direction.  Uses TextDirection as it only has two possibilities instead of
  // WTF::Unicode::Direction which has 19.
  BidiStatus(TextDirection text_direction, bool is_override) {
    WTF::Unicode::CharDirection direction =
        text_direction == TextDirection::kLtr ? WTF::Unicode::kLeftToRight
                                              : WTF::Unicode::kRightToLeft;
    eor = last_strong = last = direction;
    context = BidiContext::Create(text_direction == TextDirection::kLtr ? 0 : 1,
                                  direction, is_override);
  }

  BidiStatus(WTF::Unicode::CharDirection eor_dir,
             WTF::Unicode::CharDirection last_strong_dir,
             WTF::Unicode::CharDirection last_dir,
             RefPtr<BidiContext> bidi_context)
      : eor(eor_dir),
        last_strong(last_strong_dir),
        last(last_dir),
        context(std::move(bidi_context)) {}

  // Creates a BidiStatus for Isolates (RLI/LRI).
  // The rule X5a ans X5b of UAX#9: http://unicode.org/reports/tr9/#X5a
  static BidiStatus CreateForIsolate(TextDirection text_direction,
                                     bool is_override,
                                     unsigned char level) {
    WTF::Unicode::CharDirection direction;
    if (text_direction == TextDirection::kRtl) {
      level = NextGreaterOddLevel(level);
      direction = WTF::Unicode::kRightToLeft;
    } else {
      level = NextGreaterEvenLevel(level);
      direction = WTF::Unicode::kLeftToRight;
    }
    RefPtr<BidiContext> context =
        BidiContext::Create(level, direction, is_override, kFromStyleOrDOM);

    // This copies BidiStatus and may churn the ref on BidiContext.
    // I doubt it matters.
    return BidiStatus(direction, direction, direction, std::move(context));
  }

  WTF::Unicode::CharDirection eor;
  WTF::Unicode::CharDirection last_strong;
  WTF::Unicode::CharDirection last;
  RefPtr<BidiContext> context;
};

class BidiEmbedding final {
  DISALLOW_NEW_EXCEPT_PLACEMENT_NEW();

 public:
  BidiEmbedding(WTF::Unicode::CharDirection direction,
                BidiEmbeddingSource source)
      : direction_(direction), source_(source) {}

  WTF::Unicode::CharDirection Direction() const { return direction_; }
  BidiEmbeddingSource Source() const { return source_; }

 private:
  WTF::Unicode::CharDirection direction_;
  BidiEmbeddingSource source_;
};

inline bool operator==(const BidiStatus& status1, const BidiStatus& status2) {
  return status1.eor == status2.eor && status1.last == status2.last &&
         status1.last_strong == status2.last_strong &&
         *(status1.context) == *(status2.context);
}

inline bool operator!=(const BidiStatus& status1, const BidiStatus& status2) {
  return !(status1 == status2);
}

enum VisualDirectionOverride {
  kNoVisualOverride,
  kVisualLeftToRightOverride,
  kVisualRightToLeftOverride
};

class NoIsolatedRun {};

// BidiResolver is WebKit's implementation of the Unicode Bidi Algorithm
// http://unicode.org/reports/tr9
template <class Iterator, class Run, class IsolatedRun = NoIsolatedRun>
class BidiResolver final {
  DISALLOW_NEW();
  WTF_MAKE_NONCOPYABLE(BidiResolver);

 public:
  BidiResolver()
      : direction_(WTF::Unicode::kOtherNeutral),
        reached_end_of_line_(false),
        empty_run_(true),
        nested_isolate_count_(0),
        trailing_space_run_(0),
        needs_trailing_space_(false) {}

#if DCHECK_IS_ON()
  ~BidiResolver();
#endif

  const Iterator& GetPosition() const { return current_; }
  Iterator& GetPosition() { return current_; }
  void SetPositionIgnoringNestedIsolates(const Iterator& position) {
    current_ = position;
  }
  void SetPosition(const Iterator& position, unsigned nested_isolated_count) {
    current_ = position;
    nested_isolate_count_ = nested_isolated_count;
  }

  BidiContext* Context() const { return status_.context.Get(); }
  void SetContext(RefPtr<BidiContext> c) { status_.context = std::move(c); }

  void SetLastDir(WTF::Unicode::CharDirection last_dir) {
    status_.last = last_dir;
  }
  void SetLastStrongDir(WTF::Unicode::CharDirection last_strong_dir) {
    status_.last_strong = last_strong_dir;
  }
  void SetEorDir(WTF::Unicode::CharDirection eor_dir) { status_.eor = eor_dir; }

  WTF::Unicode::CharDirection Dir() const { return direction_; }
  void SetDir(WTF::Unicode::CharDirection d) { direction_ = d; }

  const BidiStatus& Status() const { return status_; }
  void SetStatus(const BidiStatus s) {
    DCHECK(s.context);
    status_ = s;
    paragraph_directionality_ = s.context->Dir() == WTF::Unicode::kLeftToRight
                                    ? TextDirection::kLtr
                                    : TextDirection::kRtl;
  }

  MidpointState<Iterator>& GetMidpointState() { return midpoint_state_; }

  // The current algorithm handles nested isolates one layer of nesting at a
  // time.  But when we layout each isolated span, we will walk into (and
  // ignore) all child isolated spans.
  void EnterIsolate() { nested_isolate_count_++; }
  void ExitIsolate() {
    DCHECK_GE(nested_isolate_count_, 1u);
    nested_isolate_count_--;
  }
  bool InIsolate() const { return nested_isolate_count_; }

  void Embed(WTF::Unicode::CharDirection, BidiEmbeddingSource);
  bool CommitExplicitEmbedding(BidiRunList<Run>&);

  void CreateBidiRunsForLine(const Iterator& end,
                             VisualDirectionOverride = kNoVisualOverride,
                             bool hard_line_break = false,
                             bool reorder_runs = true);

  BidiRunList<Run>& Runs() { return runs_; }

  // FIXME: This used to be part of deleteRuns() but was a layering violation.
  // It's unclear if this is still needed.
  void MarkCurrentRunEmpty() { empty_run_ = true; }

  Vector<IsolatedRun>& IsolatedRuns() { return isolated_runs_; }

  bool IsEndOfLine(const Iterator& end) {
    return current_ == end || current_.AtEnd();
  }

  TextDirection DetermineParagraphDirectionality(
      bool* has_strong_directionality = 0) {
    bool break_on_paragraph = true;
    return DetermineDirectionalityInternal(break_on_paragraph,
                                           has_strong_directionality);
  }
  TextDirection DetermineDirectionality(bool* has_strong_directionality = 0) {
    bool break_on_paragraph = false;
    return DetermineDirectionalityInternal(break_on_paragraph,
                                           has_strong_directionality);
  }

  void SetMidpointStateForIsolatedRun(Run&, const MidpointState<Iterator>&);
  MidpointState<Iterator> MidpointStateForIsolatedRun(Run&);

  Iterator EndOfLine() const { return end_of_line_; }

  void SetNeedsTrailingSpace(bool value) { needs_trailing_space_ = value; }
  Run* TrailingSpaceRun() const { return trailing_space_run_; }

 protected:
  void Increment() { current_.Increment(); }
  // FIXME: Instead of InlineBidiResolvers subclassing this method, we should
  // pass in some sort of Traits object which knows how to create runs for
  // appending.
  void AppendRun(BidiRunList<Run>&);

  Run* AddTrailingRun(BidiRunList<Run>&,
                      int,
                      int,
                      Run*,
                      BidiContext*,
                      TextDirection) const {
    return 0;
  }
  Iterator current_;
  // sor and eor are "start of run" and "end of run" respectively and correpond
  // to abreviations used in UBA spec: http://unicode.org/reports/tr9/#BD7
  Iterator sor_;  // Points to the first character in the current run.
  Iterator eor_;  // Points to the last character in the current run.
  Iterator last_;
  BidiStatus status_;
  WTF::Unicode::CharDirection direction_;
  // m_endOfRunAtEndOfLine is "the position last eor in the end of line"
  Iterator end_of_run_at_end_of_line_;
  Iterator end_of_line_;
  bool reached_end_of_line_;
  Iterator last_before_et_;  // Before a EuropeanNumberTerminator
  bool empty_run_;

  // FIXME: This should not belong to the resolver, but rather be passed
  // into createBidiRunsForLine by the caller.
  BidiRunList<Run> runs_;

  MidpointState<Iterator> midpoint_state_;

  unsigned nested_isolate_count_;
  Vector<IsolatedRun> isolated_runs_;
  Run* trailing_space_run_;
  bool needs_trailing_space_;
  TextDirection paragraph_directionality_;

 private:
  void RaiseExplicitEmbeddingLevel(BidiRunList<Run>&,
                                   WTF::Unicode::CharDirection from,
                                   WTF::Unicode::CharDirection to);
  void LowerExplicitEmbeddingLevel(BidiRunList<Run>&,
                                   WTF::Unicode::CharDirection from);
  void CheckDirectionInLowerRaiseEmbeddingLevel();

  void UpdateStatusLastFromCurrentDirection(WTF::Unicode::CharDirection);
  void ReorderRunsFromLevels(BidiRunList<Run>&) const;

  bool NeedsTrailingSpace(BidiRunList<Run>&) { return needs_trailing_space_; }
  int FindFirstTrailingSpaceAtRun(Run*) { return 0; }
  // http://www.unicode.org/reports/tr9/#L1
  void ComputeTrailingSpace(BidiRunList<Run>&);

  TextDirection DetermineDirectionalityInternal(
      bool break_on_paragraph,
      bool* has_strong_directionality);

  Vector<BidiEmbedding, 8> current_explicit_embedding_sequence_;
  HashMap<Run*, MidpointState<Iterator>> midpoint_state_for_isolated_run_;
};

#if DCHECK_IS_ON()
template <class Iterator, class Run, class IsolatedRun>
BidiResolver<Iterator, Run, IsolatedRun>::~BidiResolver() {
  // The owner of this resolver should have handled the isolated runs.
  DCHECK(isolated_runs_.IsEmpty());
  DCHECK(!runs_.RunCount());
}
#endif

template <class Iterator, class Run, class IsolatedRun>
void BidiResolver<Iterator, Run, IsolatedRun>::AppendRun(
    BidiRunList<Run>& runs) {
  if (!empty_run_ && !eor_.AtEnd()) {
    unsigned start_offset = sor_.Offset();
    unsigned end_offset = eor_.Offset();

    if (!end_of_run_at_end_of_line_.AtEnd() &&
        end_offset >= end_of_run_at_end_of_line_.Offset()) {
      reached_end_of_line_ = true;
      end_offset = end_of_run_at_end_of_line_.Offset();
    }

    // m_eor and m_endOfRunAtEndOfLine are inclusive while BidiRun's stop is
    // exclusive so offset needs to be increased by one.
    end_offset += 1;

    // Append BidiRun objects, at most 64K chars at a time, until all
    // text between |startOffset| and |endOffset| is represented.
    while (start_offset < end_offset) {
      unsigned end = end_offset;
      const int kLimit =
          USHRT_MAX;  // InlineTextBox stores text length as unsigned short.
      if (end - start_offset > kLimit)
        end = start_offset + kLimit;
      runs.AddRun(new Run(Context()->Override(), Context()->Level(),
                          start_offset, end, direction_, Context()->Dir()));
      start_offset = end;
    }

    eor_.Increment();
    sor_ = eor_;
  }

  direction_ = WTF::Unicode::kOtherNeutral;
  status_.eor = WTF::Unicode::kOtherNeutral;
}

template <class Iterator, class Run, class IsolatedRun>
void BidiResolver<Iterator, Run, IsolatedRun>::Embed(
    WTF::Unicode::CharDirection dir,
    BidiEmbeddingSource source) {
  // Isolated spans compute base directionality during their own UBA run.
  // Do not insert fake embed characters once we enter an isolated span.
  DCHECK(!InIsolate());
  using namespace WTF::Unicode;

  DCHECK(dir == kPopDirectionalFormat || dir == kLeftToRightEmbedding ||
         dir == kLeftToRightOverride || dir == kRightToLeftEmbedding ||
         dir == kRightToLeftOverride);
  current_explicit_embedding_sequence_.push_back(BidiEmbedding(dir, source));
}

template <class Iterator, class Run, class IsolatedRun>
void BidiResolver<Iterator, Run, IsolatedRun>::
    CheckDirectionInLowerRaiseEmbeddingLevel() {
  using namespace WTF::Unicode;

  DCHECK(status_.eor != kOtherNeutral || eor_.AtEnd());
  DCHECK_NE(status_.last, kNonSpacingMark);
  DCHECK_NE(status_.last, kBoundaryNeutral);
  DCHECK_NE(status_.last, kRightToLeftEmbedding);
  DCHECK_NE(status_.last, kLeftToRightEmbedding);
  DCHECK_NE(status_.last, kRightToLeftOverride);
  DCHECK_NE(status_.last, kLeftToRightOverride);
  DCHECK_NE(status_.last, kPopDirectionalFormat);
  if (direction_ == kOtherNeutral)
    direction_ =
        status_.last_strong == kLeftToRight ? kLeftToRight : kRightToLeft;
}

template <class Iterator, class Run, class IsolatedRun>
void BidiResolver<Iterator, Run, IsolatedRun>::LowerExplicitEmbeddingLevel(
    BidiRunList<Run>& runs,
    WTF::Unicode::CharDirection from) {
  using namespace WTF::Unicode;

  if (!empty_run_ && eor_ != last_) {
    CheckDirectionInLowerRaiseEmbeddingLevel();
    // bidi.sor ... bidi.eor ... bidi.last eor; need to append the
    // bidi.sor-bidi.eor run or extend it through bidi.last
    if (from == kLeftToRight) {
      // bidi.sor ... bidi.eor ... bidi.last L
      if (status_.eor == kEuropeanNumber) {
        if (status_.last_strong != kLeftToRight) {
          direction_ = kEuropeanNumber;
          AppendRun(runs);
        }
      } else if (status_.eor == kArabicNumber) {
        direction_ = kArabicNumber;
        AppendRun(runs);
      } else if (status_.last_strong != kLeftToRight) {
        AppendRun(runs);
        direction_ = kLeftToRight;
      }
    } else if (status_.eor == kEuropeanNumber || status_.eor == kArabicNumber ||
               status_.last_strong == kLeftToRight) {
      AppendRun(runs);
      direction_ = kRightToLeft;
    }
    eor_ = last_;
  }

  AppendRun(runs);
  empty_run_ = true;

  // sor for the new run is determined by the higher level (rule X10)
  SetLastDir(from);
  SetLastStrongDir(from);
  eor_ = Iterator();
}

template <class Iterator, class Run, class IsolatedRun>
void BidiResolver<Iterator, Run, IsolatedRun>::RaiseExplicitEmbeddingLevel(
    BidiRunList<Run>& runs,
    WTF::Unicode::CharDirection from,
    WTF::Unicode::CharDirection to) {
  using namespace WTF::Unicode;

  if (!empty_run_ && eor_ != last_) {
    CheckDirectionInLowerRaiseEmbeddingLevel();
    // bidi.sor ... bidi.eor ... bidi.last eor; need to append the
    // bidi.sor-bidi.eor run or extend it through bidi.last
    if (to == kLeftToRight) {
      // bidi.sor ... bidi.eor ... bidi.last L
      if (status_.eor == kEuropeanNumber) {
        if (status_.last_strong != kLeftToRight) {
          direction_ = kEuropeanNumber;
          AppendRun(runs);
        }
      } else if (status_.eor == kArabicNumber) {
        direction_ = kArabicNumber;
        AppendRun(runs);
      } else if (status_.last_strong != kLeftToRight && from == kLeftToRight) {
        AppendRun(runs);
        direction_ = kLeftToRight;
      }
    } else if (status_.eor == kArabicNumber ||
               (status_.eor == kEuropeanNumber &&
                (status_.last_strong != kLeftToRight ||
                 from == kRightToLeft)) ||
               (status_.eor != kEuropeanNumber &&
                status_.last_strong == kLeftToRight && from == kRightToLeft)) {
      AppendRun(runs);
      direction_ = kRightToLeft;
    }
    eor_ = last_;
  }

  AppendRun(runs);
  empty_run_ = true;

  SetLastDir(to);
  SetLastStrongDir(to);
  eor_ = Iterator();
}

template <class Iterator, class Run, class IsolatedRun>
void BidiResolver<Iterator, Run, IsolatedRun>::ComputeTrailingSpace(
    BidiRunList<Run>& runs) {
  DCHECK(runs.RunCount());

  Run* trailing_space_run = runs.LogicallyLastRun();

  int first_space = FindFirstTrailingSpaceAtRun(trailing_space_run);
  if (first_space == trailing_space_run->Stop())
    return;

  bool should_reorder =
      trailing_space_run != (paragraph_directionality_ == TextDirection::kLtr
                                 ? runs.LastRun()
                                 : runs.FirstRun());
  if (first_space != trailing_space_run->Start()) {
    BidiContext* base_context = Context();
    while (BidiContext* parent = base_context->Parent())
      base_context = parent;

    trailing_space_run_ = AddTrailingRun(
        runs, first_space, trailing_space_run->stop_, trailing_space_run,
        base_context, paragraph_directionality_);
    DCHECK(trailing_space_run_);
    trailing_space_run->stop_ = first_space;
    return;
  }
  if (!should_reorder) {
    trailing_space_run_ = trailing_space_run;
    return;
  }

  // Apply L1 rule.
  if (paragraph_directionality_ == TextDirection::kLtr) {
    runs.MoveRunToEnd(trailing_space_run);
    trailing_space_run->level_ = 0;
  } else {
    runs.MoveRunToBeginning(trailing_space_run);
    trailing_space_run->level_ = 1;
  }
  trailing_space_run_ = trailing_space_run;
}

template <class Iterator, class Run, class IsolatedRun>
bool BidiResolver<Iterator, Run, IsolatedRun>::CommitExplicitEmbedding(
    BidiRunList<Run>& runs) {
  // When we're "inIsolate()" we're resolving the parent context which
  // ignores (skips over) the isolated content, including embedding levels.
  // We should never accrue embedding levels while skipping over isolated
  // content.
  DCHECK(!InIsolate() || current_explicit_embedding_sequence_.IsEmpty());

  using namespace WTF::Unicode;

  unsigned char from_level = Context()->Level();
  RefPtr<BidiContext> to_context = Context();

  for (size_t i = 0; i < current_explicit_embedding_sequence_.size(); ++i) {
    BidiEmbedding embedding = current_explicit_embedding_sequence_[i];
    if (embedding.Direction() == kPopDirectionalFormat) {
      if (BidiContext* parent_context = to_context->Parent())
        to_context = parent_context;
    } else {
      CharDirection direction =
          (embedding.Direction() == kRightToLeftEmbedding ||
           embedding.Direction() == kRightToLeftOverride)
              ? kRightToLeft
              : kLeftToRight;
      bool override = embedding.Direction() == kLeftToRightOverride ||
                      embedding.Direction() == kRightToLeftOverride;
      unsigned char level = to_context->Level();
      if (direction == kRightToLeft)
        level = NextGreaterOddLevel(level);
      else
        level = NextGreaterEvenLevel(level);
      if (level < BidiContext::kMaxLevel)
        to_context = BidiContext::Create(level, direction, override,
                                         embedding.Source(), to_context.Get());
    }
  }

  unsigned char to_level = to_context->Level();

  if (to_level > from_level)
    RaiseExplicitEmbeddingLevel(runs,
                                from_level % 2 ? kRightToLeft : kLeftToRight,
                                to_level % 2 ? kRightToLeft : kLeftToRight);
  else if (to_level < from_level)
    LowerExplicitEmbeddingLevel(runs,
                                from_level % 2 ? kRightToLeft : kLeftToRight);

  SetContext(to_context);

  current_explicit_embedding_sequence_.clear();

  return from_level != to_level;
}

template <class Iterator, class Run, class IsolatedRun>
inline void
BidiResolver<Iterator, Run, IsolatedRun>::UpdateStatusLastFromCurrentDirection(
    WTF::Unicode::CharDirection dir_current) {
  using namespace WTF::Unicode;
  switch (dir_current) {
    case kEuropeanNumberTerminator:
      if (status_.last != kEuropeanNumber)
        status_.last = kEuropeanNumberTerminator;
      break;
    case kEuropeanNumberSeparator:
    case kCommonNumberSeparator:
    case kSegmentSeparator:
    case kWhiteSpaceNeutral:
    case kOtherNeutral:
      switch (status_.last) {
        case kLeftToRight:
        case kRightToLeft:
        case kRightToLeftArabic:
        case kEuropeanNumber:
        case kArabicNumber:
          status_.last = dir_current;
          break;
        default:
          status_.last = kOtherNeutral;
      }
      break;
    case kNonSpacingMark:
    case kBoundaryNeutral:
    case kRightToLeftEmbedding:
    case kLeftToRightEmbedding:
    case kRightToLeftOverride:
    case kLeftToRightOverride:
    case kPopDirectionalFormat:
      // ignore these
      break;
    case kEuropeanNumber:
    // fall through
    default:
      status_.last = dir_current;
  }
}

template <class Iterator, class Run, class IsolatedRun>
inline void BidiResolver<Iterator, Run, IsolatedRun>::ReorderRunsFromLevels(
    BidiRunList<Run>& runs) const {
  unsigned char level_low = BidiContext::kMaxLevel;
  unsigned char level_high = 0;
  for (Run* run = runs.FirstRun(); run; run = run->Next()) {
    level_high = std::max(run->Level(), level_high);
    level_low = std::min(run->Level(), level_low);
  }

  // This implements reordering of the line (L2 according to Bidi spec):
  // http://unicode.org/reports/tr9/#L2
  // L2. From the highest level found in the text to the lowest odd level on
  // each line, reverse any contiguous sequence of characters that are at that
  // level or higher.

  // Reversing is only done up to the lowest odd level.
  if (!(level_low % 2))
    level_low++;

  unsigned count = runs.RunCount() - 1;

  while (level_high >= level_low) {
    unsigned i = 0;
    Run* run = runs.FirstRun();
    while (i < count) {
      for (; i < count && run && run->Level() < level_high; i++)
        run = run->Next();
      unsigned start = i;
      for (; i <= count && run && run->Level() >= level_high; i++)
        run = run->Next();
      unsigned end = i - 1;
      runs.ReverseRuns(start, end);
    }
    level_high--;
  }
}

template <class Iterator, class Run, class IsolatedRun>
TextDirection
BidiResolver<Iterator, Run, IsolatedRun>::DetermineDirectionalityInternal(
    bool break_on_paragraph,
    bool* has_strong_directionality) {
  while (!current_.AtEnd()) {
    if (InIsolate()) {
      Increment();
      continue;
    }
    if (break_on_paragraph && current_.AtParagraphSeparator())
      break;
    UChar32 current = current_.Current();
    if (UNLIKELY(U16_IS_SURROGATE(current))) {
      Increment();
      // If this not the high part of the surrogate pair, then drop it and move
      // to the next.
      if (!U16_IS_SURROGATE_LEAD(current))
        continue;
      UChar high = static_cast<UChar>(current);
      if (current_.AtEnd())
        continue;
      UChar low = current_.Current();
      // Verify the low part. If invalid, then assume an invalid surrogate pair
      // and retry.
      if (!U16_IS_TRAIL(low))
        continue;
      current = U16_GET_SUPPLEMENTARY(high, low);
    }
    WTF::Unicode::CharDirection char_direction =
        WTF::Unicode::Direction(current);
    if (char_direction == WTF::Unicode::kLeftToRight) {
      if (has_strong_directionality)
        *has_strong_directionality = true;
      return TextDirection::kLtr;
    }
    if (char_direction == WTF::Unicode::kRightToLeft ||
        char_direction == WTF::Unicode::kRightToLeftArabic) {
      if (has_strong_directionality)
        *has_strong_directionality = true;
      return TextDirection::kRtl;
    }
    Increment();
  }
  if (has_strong_directionality)
    *has_strong_directionality = false;
  return TextDirection::kLtr;
}

inline TextDirection DirectionForCharacter(UChar32 character) {
  WTF::Unicode::CharDirection char_direction =
      WTF::Unicode::Direction(character);
  if (char_direction == WTF::Unicode::kRightToLeft ||
      char_direction == WTF::Unicode::kRightToLeftArabic)
    return TextDirection::kRtl;
  return TextDirection::kLtr;
}

template <class Iterator, class Run, class IsolatedRun>
void BidiResolver<Iterator, Run, IsolatedRun>::CreateBidiRunsForLine(
    const Iterator& end,
    VisualDirectionOverride override,
    bool hard_line_break,
    bool reorder_runs) {
  using namespace WTF::Unicode;

  DCHECK_EQ(direction_, kOtherNeutral);
  trailing_space_run_ = 0;

  end_of_line_ = end;

  if (override != kNoVisualOverride) {
    empty_run_ = false;
    sor_ = current_;
    eor_ = Iterator();
    while (current_ != end && !current_.AtEnd()) {
      eor_ = current_;
      Increment();
    }
    direction_ =
        override == kVisualLeftToRightOverride ? kLeftToRight : kRightToLeft;
    AppendRun(runs_);
    runs_.SetLogicallyLastRun(runs_.LastRun());
    if (override == kVisualRightToLeftOverride && runs_.RunCount())
      runs_.ReverseRuns(0, runs_.RunCount() - 1);
    return;
  }

  empty_run_ = true;

  eor_ = Iterator();

  last_ = current_;
  bool last_line_ended = false;
  BidiResolver<Iterator, Run, IsolatedRun> state_at_end;

  while (true) {
    if (InIsolate() && empty_run_) {
      sor_ = current_;
      empty_run_ = false;
    }

    if (!last_line_ended && IsEndOfLine(end)) {
      if (empty_run_)
        break;

      state_at_end.status_ = status_;
      state_at_end.sor_ = sor_;
      state_at_end.eor_ = eor_;
      state_at_end.last_ = last_;
      state_at_end.reached_end_of_line_ = reached_end_of_line_;
      state_at_end.last_before_et_ = last_before_et_;
      state_at_end.empty_run_ = empty_run_;
      end_of_run_at_end_of_line_ = last_;
      last_line_ended = true;
    }
    CharDirection dir_current;
    if (last_line_ended && (hard_line_break || current_.AtEnd())) {
      BidiContext* c = Context();
      if (hard_line_break) {
        // A deviation from the Unicode Bidi Algorithm in order to match
        // WinIE and user expectations: hard line breaks reset bidi state
        // coming from unicode bidi control characters, but not those from
        // DOM nodes with specified directionality
        state_at_end.SetContext(c->CopyStackRemovingUnicodeEmbeddingContexts());

        dir_current = state_at_end.Context()->Dir();
        state_at_end.SetEorDir(dir_current);
        state_at_end.SetLastDir(dir_current);
        state_at_end.SetLastStrongDir(dir_current);
      } else {
        while (c->Parent())
          c = c->Parent();
        dir_current = c->Dir();
      }
    } else {
      dir_current = current_.Direction();
      if (Context()->Override() && dir_current != kRightToLeftEmbedding &&
          dir_current != kLeftToRightEmbedding &&
          dir_current != kRightToLeftOverride &&
          dir_current != kLeftToRightOverride &&
          dir_current != kPopDirectionalFormat)
        dir_current = Context()->Dir();
      else if (dir_current == kNonSpacingMark)
        dir_current = status_.last;
    }

    // We ignore all character directionality while in unicode-bidi: isolate
    // spans.  We'll handle ordering the isolated characters in a second pass.
    if (InIsolate())
      dir_current = kOtherNeutral;

    DCHECK(status_.eor != kOtherNeutral || eor_.AtEnd());
    switch (dir_current) {
      // embedding and overrides (X1-X9 in the Bidi specs)
      case kRightToLeftEmbedding:
      case kLeftToRightEmbedding:
      case kRightToLeftOverride:
      case kLeftToRightOverride:
      case kPopDirectionalFormat:
        Embed(dir_current, kFromUnicode);
        CommitExplicitEmbedding(runs_);
        break;

      // strong types
      case kLeftToRight:
        switch (status_.last) {
          case kRightToLeft:
          case kRightToLeftArabic:
          case kEuropeanNumber:
          case kArabicNumber:
            if (status_.last != kEuropeanNumber ||
                status_.last_strong != kLeftToRight)
              AppendRun(runs_);
            break;
          case kLeftToRight:
            break;
          case kEuropeanNumberSeparator:
          case kEuropeanNumberTerminator:
          case kCommonNumberSeparator:
          case kBoundaryNeutral:
          case kBlockSeparator:
          case kSegmentSeparator:
          case kWhiteSpaceNeutral:
          case kOtherNeutral:
            if (status_.eor == kEuropeanNumber) {
              if (status_.last_strong != kLeftToRight) {
                // the numbers need to be on a higher embedding level, so let's
                // close that run
                direction_ = kEuropeanNumber;
                AppendRun(runs_);
                if (Context()->Dir() != kLeftToRight) {
                  // the neutrals take the embedding direction, which is R
                  eor_ = last_;
                  direction_ = kRightToLeft;
                  AppendRun(runs_);
                }
              }
            } else if (status_.eor == kArabicNumber) {
              // Arabic numbers are always on a higher embedding level, so let's
              // close that run
              direction_ = kArabicNumber;
              AppendRun(runs_);
              if (Context()->Dir() != kLeftToRight) {
                // the neutrals take the embedding direction, which is R
                eor_ = last_;
                direction_ = kRightToLeft;
                AppendRun(runs_);
              }
            } else if (status_.last_strong != kLeftToRight) {
              // last stuff takes embedding dir
              if (Context()->Dir() == kRightToLeft) {
                eor_ = last_;
                direction_ = kRightToLeft;
              }
              AppendRun(runs_);
            }
          default:
            break;
        }
        eor_ = current_;
        status_.eor = kLeftToRight;
        status_.last_strong = kLeftToRight;
        direction_ = kLeftToRight;
        break;
      case kRightToLeftArabic:
      case kRightToLeft:
        switch (status_.last) {
          case kLeftToRight:
          case kEuropeanNumber:
          case kArabicNumber:
            AppendRun(runs_);
          case kRightToLeft:
          case kRightToLeftArabic:
            break;
          case kEuropeanNumberSeparator:
          case kEuropeanNumberTerminator:
          case kCommonNumberSeparator:
          case kBoundaryNeutral:
          case kBlockSeparator:
          case kSegmentSeparator:
          case kWhiteSpaceNeutral:
          case kOtherNeutral:
            if (status_.eor == kEuropeanNumber) {
              if (status_.last_strong == kLeftToRight &&
                  Context()->Dir() == kLeftToRight)
                eor_ = last_;
              AppendRun(runs_);
            } else if (status_.eor == kArabicNumber) {
              AppendRun(runs_);
            } else if (status_.last_strong == kLeftToRight) {
              if (Context()->Dir() == kLeftToRight)
                eor_ = last_;
              AppendRun(runs_);
            }
          default:
            break;
        }
        eor_ = current_;
        status_.eor = kRightToLeft;
        status_.last_strong = dir_current;
        direction_ = kRightToLeft;
        break;

      // weak types:

      case kEuropeanNumber:
        if (status_.last_strong != kRightToLeftArabic) {
          // if last strong was AL change EN to AN
          switch (status_.last) {
            case kEuropeanNumber:
            case kLeftToRight:
              break;
            case kRightToLeft:
            case kRightToLeftArabic:
            case kArabicNumber:
              eor_ = last_;
              AppendRun(runs_);
              direction_ = kEuropeanNumber;
              break;
            case kEuropeanNumberSeparator:
            case kCommonNumberSeparator:
              if (status_.eor == kEuropeanNumber)
                break;
            case kEuropeanNumberTerminator:
            case kBoundaryNeutral:
            case kBlockSeparator:
            case kSegmentSeparator:
            case kWhiteSpaceNeutral:
            case kOtherNeutral:
              if (status_.eor == kEuropeanNumber) {
                if (status_.last_strong == kRightToLeft) {
                  // ENs on both sides behave like Rs, so the neutrals should be
                  // R.  Terminate the EN run.
                  AppendRun(runs_);
                  // Make an R run.
                  eor_ = status_.last == kEuropeanNumberTerminator
                             ? last_before_et_
                             : last_;
                  direction_ = kRightToLeft;
                  AppendRun(runs_);
                  // Begin a new EN run.
                  direction_ = kEuropeanNumber;
                }
              } else if (status_.eor == kArabicNumber) {
                // Terminate the AN run.
                AppendRun(runs_);
                if (status_.last_strong == kRightToLeft ||
                    Context()->Dir() == kRightToLeft) {
                  // Make an R run.
                  eor_ = status_.last == kEuropeanNumberTerminator
                             ? last_before_et_
                             : last_;
                  direction_ = kRightToLeft;
                  AppendRun(runs_);
                  // Begin a new EN run.
                  direction_ = kEuropeanNumber;
                }
              } else if (status_.last_strong == kRightToLeft) {
                // Extend the R run to include the neutrals.
                eor_ = status_.last == kEuropeanNumberTerminator
                           ? last_before_et_
                           : last_;
                direction_ = kRightToLeft;
                AppendRun(runs_);
                // Begin a new EN run.
                direction_ = kEuropeanNumber;
              }
            default:
              break;
          }
          eor_ = current_;
          status_.eor = kEuropeanNumber;
          if (direction_ == kOtherNeutral)
            direction_ = kLeftToRight;
          break;
        }
      case kArabicNumber:
        dir_current = kArabicNumber;
        switch (status_.last) {
          case kLeftToRight:
            if (Context()->Dir() == kLeftToRight)
              AppendRun(runs_);
            break;
          case kArabicNumber:
            break;
          case kRightToLeft:
          case kRightToLeftArabic:
          case kEuropeanNumber:
            eor_ = last_;
            AppendRun(runs_);
            break;
          case kCommonNumberSeparator:
            if (status_.eor == kArabicNumber)
              break;
          case kEuropeanNumberSeparator:
          case kEuropeanNumberTerminator:
          case kBoundaryNeutral:
          case kBlockSeparator:
          case kSegmentSeparator:
          case kWhiteSpaceNeutral:
          case kOtherNeutral:
            if (status_.eor == kArabicNumber ||
                (status_.eor == kEuropeanNumber &&
                 (status_.last_strong == kRightToLeft ||
                  Context()->Dir() == kRightToLeft)) ||
                (status_.eor != kEuropeanNumber &&
                 status_.last_strong == kLeftToRight &&
                 Context()->Dir() == kRightToLeft)) {
              // Terminate the run before the neutrals.
              AppendRun(runs_);
              // Begin an R run for the neutrals.
              direction_ = kRightToLeft;
            } else if (direction_ == kOtherNeutral) {
              direction_ = status_.last_strong == kLeftToRight ? kLeftToRight
                                                               : kRightToLeft;
            }
            eor_ = last_;
            AppendRun(runs_);
          default:
            break;
        }
        eor_ = current_;
        status_.eor = kArabicNumber;
        if (direction_ == kOtherNeutral)
          direction_ = kArabicNumber;
        break;
      case kEuropeanNumberSeparator:
      case kCommonNumberSeparator:
        break;
      case kEuropeanNumberTerminator:
        if (status_.last == kEuropeanNumber) {
          dir_current = kEuropeanNumber;
          eor_ = current_;
          status_.eor = dir_current;
        } else if (status_.last != kEuropeanNumberTerminator) {
          last_before_et_ = empty_run_ ? eor_ : last_;
        }
        break;

      // boundary neutrals should be ignored
      case kBoundaryNeutral:
        if (eor_ == last_)
          eor_ = current_;
        break;
      // neutrals
      case kBlockSeparator:
        // ### what do we do with newline and paragraph seperators that come to
        // here?
        break;
      case kSegmentSeparator:
        // ### implement rule L1
        break;
      case kWhiteSpaceNeutral:
        break;
      case kOtherNeutral:
        break;
      default:
        break;
    }

    if (last_line_ended && eor_ == current_) {
      if (!reached_end_of_line_) {
        eor_ = end_of_run_at_end_of_line_;
        switch (status_.eor) {
          case kLeftToRight:
          case kRightToLeft:
          case kArabicNumber:
            direction_ = status_.eor;
            break;
          case kEuropeanNumber:
            direction_ = status_.last_strong == kLeftToRight ? kLeftToRight
                                                             : kEuropeanNumber;
            break;
          default:
            NOTREACHED();
        }
        AppendRun(runs_);
      }
      current_ = end;
      status_ = state_at_end.status_;
      sor_ = state_at_end.sor_;
      eor_ = state_at_end.eor_;
      last_ = state_at_end.last_;
      reached_end_of_line_ = state_at_end.reached_end_of_line_;
      last_before_et_ = state_at_end.last_before_et_;
      empty_run_ = state_at_end.empty_run_;
      direction_ = kOtherNeutral;
      break;
    }

    UpdateStatusLastFromCurrentDirection(dir_current);
    last_ = current_;

    if (empty_run_) {
      sor_ = current_;
      empty_run_ = false;
    }

    Increment();
    if (!current_explicit_embedding_sequence_.IsEmpty()) {
      bool committed = CommitExplicitEmbedding(runs_);
      if (committed && last_line_ended) {
        current_ = end;
        status_ = state_at_end.status_;
        sor_ = state_at_end.sor_;
        eor_ = state_at_end.eor_;
        last_ = state_at_end.last_;
        reached_end_of_line_ = state_at_end.reached_end_of_line_;
        last_before_et_ = state_at_end.last_before_et_;
        empty_run_ = state_at_end.empty_run_;
        direction_ = kOtherNeutral;
        break;
      }
    }
  }

  runs_.SetLogicallyLastRun(runs_.LastRun());
  if (reorder_runs)
    ReorderRunsFromLevels(runs_);
  end_of_run_at_end_of_line_ = Iterator();
  end_of_line_ = Iterator();

  if (!hard_line_break && runs_.RunCount() && NeedsTrailingSpace(runs_)) {
    ComputeTrailingSpace(runs_);
  }
}

template <class Iterator, class Run, class IsolatedRun>
void BidiResolver<Iterator, Run, IsolatedRun>::SetMidpointStateForIsolatedRun(
    Run& run,
    const MidpointState<Iterator>& midpoint) {
  DCHECK(!midpoint_state_for_isolated_run_.Contains(&run));
  midpoint_state_for_isolated_run_.insert(&run, midpoint);
}

template <class Iterator, class Run, class IsolatedRun>
MidpointState<Iterator>
BidiResolver<Iterator, Run, IsolatedRun>::MidpointStateForIsolatedRun(
    Run& run) {
  return midpoint_state_for_isolated_run_.Take(&run);
}

}  // namespace blink

#endif  // BidiResolver_h
