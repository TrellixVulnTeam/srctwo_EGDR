// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GridPositionsResolver_h
#define GridPositionsResolver_h

#include "core/style/GridPosition.h"
#include "platform/wtf/Allocator.h"

namespace blink {

struct GridSpan;
class LayoutBox;
class ComputedStyle;

enum GridPositionSide {
  kColumnStartSide,
  kColumnEndSide,
  kRowStartSide,
  kRowEndSide
};

enum GridTrackSizingDirection { kForColumns, kForRows };

class NamedLineCollection {
  WTF_MAKE_NONCOPYABLE(NamedLineCollection);

 public:
  NamedLineCollection(const ComputedStyle&,
                      const String& named_line,
                      GridTrackSizingDirection,
                      size_t last_line,
                      size_t auto_repeat_tracks_count);

  static bool IsValidNamedLineOrArea(const String& named_line,
                                     const ComputedStyle&,
                                     GridPositionSide);

  bool HasNamedLines();
  size_t FirstPosition();

  bool Contains(size_t line);

 private:
  size_t Find(size_t line);
  const Vector<size_t>* named_lines_indexes_ = nullptr;
  const Vector<size_t>* auto_repeat_named_lines_indexes_ = nullptr;

  size_t insertion_point_;
  size_t last_line_;
  size_t auto_repeat_total_tracks_;
  size_t auto_repeat_track_list_length_;
};

// This is a utility class with all the code related to grid items positions
// resolution.
class GridPositionsResolver {
  DISALLOW_NEW();

 public:
  static size_t ExplicitGridColumnCount(const ComputedStyle&,
                                        size_t auto_repeat_columns_count);
  static size_t ExplicitGridRowCount(const ComputedStyle&,
                                     size_t auto_repeat_rows_count);

  static GridPositionSide InitialPositionSide(GridTrackSizingDirection);
  static GridPositionSide FinalPositionSide(GridTrackSizingDirection);

  static size_t SpanSizeForAutoPlacedItem(const ComputedStyle&,
                                          const LayoutBox&,
                                          GridTrackSizingDirection);
  static GridSpan ResolveGridPositionsFromStyle(
      const ComputedStyle&,
      const LayoutBox&,
      GridTrackSizingDirection,
      size_t auto_repeat_tracks_count);
};

}  // namespace blink

#endif  // GridPositionsResolver_h
