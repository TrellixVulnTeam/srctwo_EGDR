/*
 * Copyright (C) 2002 Lars Knoll (knoll@kde.org)
 *           (C) 2002 Dirk Mueller (mueller@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License.
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

#ifndef TableLayoutAlgorithmAuto_h
#define TableLayoutAlgorithmAuto_h

#include "core/layout/TableLayoutAlgorithm.h"
#include "platform/LayoutUnit.h"
#include "platform/Length.h"
#include "platform/wtf/Vector.h"

namespace blink {

class LayoutTable;
class LayoutTableCell;

enum CellsToProcess { kAllCells, kNonEmptyCells, kEmptyCells };

enum DistributionMode { kExtraWidth, kInitialWidth, kLeftoverWidth };

enum DistributionDirection { kStartToEnd, kEndToStart };

class TableLayoutAlgorithmAuto final : public TableLayoutAlgorithm {
 public:
  TableLayoutAlgorithmAuto(LayoutTable*);
  ~TableLayoutAlgorithmAuto() override;

  void ComputeIntrinsicLogicalWidths(LayoutUnit& min_width,
                                     LayoutUnit& max_width) override;
  LayoutUnit ScaledWidthFromPercentColumns() override {
    return scaled_width_from_percent_columns_;
  }
  void ApplyPreferredLogicalWidthQuirks(LayoutUnit& min_width,
                                        LayoutUnit& max_width) const override;
  void UpdateLayout() override;
  void WillChangeTableLayout() override {}

 private:
  void FullRecalc();
  void RecalcColumn(unsigned eff_col);

  int CalcEffectiveLogicalWidth();
  void ShrinkColumnWidth(const LengthType&, int& available);
  template <typename Total,
            LengthType,
            CellsToProcess,
            DistributionMode,
            DistributionDirection>
  void DistributeWidthToColumns(int& available, Total);

  void InsertSpanCell(LayoutTableCell*);

  struct Layout {
    Layout()
        : min_logical_width(0),
          max_logical_width(0),
          effective_min_logical_width(0),
          effective_max_logical_width(0),
          computed_logical_width(0),
          empty_cells_only(true),
          column_has_no_cells(true) {}

    Length logical_width;
    Length effective_logical_width;
    int min_logical_width;
    int max_logical_width;
    int effective_min_logical_width;
    int effective_max_logical_width;
    int computed_logical_width;
    bool empty_cells_only;
    bool column_has_no_cells;
    int ClampedEffectiveMaxLogicalWidth() {
      return std::max<int>(1, effective_max_logical_width);
    }
  };

  Vector<Layout, 4> layout_struct_;
  Vector<LayoutTableCell*, 4> span_cells_;
  bool has_percent_ : 1;
  mutable bool effective_logical_width_dirty_ : 1;
  LayoutUnit scaled_width_from_percent_columns_;
};

}  // namespace blink

#endif  // TableLayoutAlgorithmAuto
