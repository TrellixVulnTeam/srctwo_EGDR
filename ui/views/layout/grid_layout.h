// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_VIEWS_LAYOUT_GRID_LAYOUT_H_
#define UI_VIEWS_LAYOUT_GRID_LAYOUT_H_

#include <stddef.h>

#include <memory>
#include <vector>

#include "base/macros.h"
#include "ui/gfx/geometry/size.h"
#include "ui/views/layout/layout_manager.h"

// GridLayout is a LayoutManager that positions child Views in a grid. You
// define the structure of the Grid first, then add the Views.
// The following creates a trivial grid with two columns separated by
// a column with padding:
// ColumnSet* columns = layout->AddColumnSet(0); // Give this column set an
//                                               // identifier of 0.
// columns->AddColumn(FILL, // Views are horizontally resized to fill column.
//                    FILL, // Views starting in this column are vertically
//                          // resized.
//                    1,    // This column has a resize weight of 1.
//                    USE_PREF, // Use the preferred size of the view.
//                    0,   // Ignored for USE_PREF.
//                    0);  // A minimum width of 0.
// columns->AddPaddingColumn(0,   // The padding column is not resizable.
//                           10); // And has a width of 10 pixels.
// columns->AddColumn(FILL, FILL, 0, USE_PREF, 0, 0);
// Now add the views:
// // First start a row.
// layout->StartRow(0,  // This row isn't vertically resizable.
//                  0); // The column set to use for this row.
// layout->AddView(v1);
// Notice you need not skip over padding columns, that's done for you.
// layout->AddView(v2);
//
// When adding a Column you give it the default alignment for all views
// originating in that column. You can override this for specific views
// when adding them. For example, the following forces a View to have
// a horizontal and vertical alignment of leading regardless of that defined
// for the column:
// layout->AddView(v1, 1, 1, LEADING, LEADING);
//
// If the View using GridLayout is given a size bigger than the preferred,
// columns and rows with a resize percent > 0 are resized. Each column/row
// is given resize_percent / total_resize_percent * extra_pixels extra
// pixels. Only Views with an Alignment of FILL are given extra space, others
// are aligned in the provided space.
//
// GridLayout allows you to define multiple column sets. When you start a
// new row you specify the id of the column set the row is to use.
//
// GridLayout allows you to force columns to have the same width. This is
// done using the LinkColumnSizes method.
//
// AddView takes care of adding the View to the View the GridLayout was
// created with.
namespace views {

class Column;
class ColumnSet;
class Row;
class View;

struct ViewState;

class VIEWS_EXPORT GridLayout : public LayoutManager {
 public:
  // An enumeration of the possible alignments supported by GridLayout.
  enum Alignment {
    // Leading equates to left along the horizontal axis, and top along the
    // vertical axis.
    LEADING,

    // Centers the view along the axis.
    CENTER,

    // Trailing equals to right along the horizontal axis, and bottom along
    // the vertical axis.
    TRAILING,

    // The view is resized to fill the space.
    FILL,

    // The view is aligned along the baseline. This is only valid for the
    // vertical axis.
    BASELINE
  };

  // An enumeration of the possible ways the size of a column may be obtained.
  enum SizeType {
    // The column size is fixed.
    FIXED,

    // The preferred size of the view is used to determine the column size.
    USE_PREF
  };

  // Creates a new GridLayout and installs it as the LayoutManager for |host|.
  static GridLayout* CreateAndInstall(View* host);

  ~GridLayout() override;

  // Creates a new column set with the specified id and returns it.
  // The id is later used when starting a new row.
  // GridLayout takes ownership of the ColumnSet and will delete it when
  // the GridLayout is deleted.
  ColumnSet* AddColumnSet(int id);

  // Returns the column set for the specified id, or NULL if one doesn't exist.
  ColumnSet* GetColumnSet(int id);

  // Adds a padding row. Padding rows typically don't have any views, and
  // but are used to provide vertical white space between views.
  // Size specifies the height of the row.
  void AddPaddingRow(float vertical_resize, int size);

  // A convenience for AddPaddingRow followed by StartRow.
  void StartRowWithPadding(float vertical_resize, int column_set_id,
                           float padding_resize, int padding);

  // Starts a new row with the specified column set and height (0 for
  // unspecified height).
  void StartRow(float vertical_resize, int column_set_id, int height = 0);

  // Advances past columns. Use this when the current column should not
  // contain any views.
  void SkipColumns(int col_count);

  // Adds a view using the default alignment from the column. The added
  // view has a column and row span of 1.
  // As a convenience this adds the view to the host. The view becomes owned
  // by the host, and NOT this GridLayout.
  void AddView(View* view);

  // Adds a view using the default alignment from the column.
  // As a convenience this adds the view to the host. The view becomes owned
  // by the host, and NOT this GridLayout.
  void AddView(View* view, int col_span, int row_span);

  // Adds a view with the specified alignment and spans.
  // As a convenience this adds the view to the host. The view becomes owned
  // by the host, and NOT this GridLayout.
  void AddView(View* view, int col_span, int row_span, Alignment h_align,
               Alignment v_align);

  // Adds a view with the specified alignment and spans. If
  // pref_width/pref_height is > 0 then the preferred width/height of the view
  // is fixed to the specified value.
  // As a convenience this adds the view to the host. The view becomes owned
  // by the host, and NOT this GridLayout.
  void AddView(View* view, int col_span, int row_span,
               Alignment h_align, Alignment v_align,
               int pref_width, int pref_height);

  // Notification we've been installed on a particular host. Checks that host
  // is the same as the View supplied in the constructor.
  void Installed(View* host) override;

  // Notification that a view has been added.
  void ViewAdded(View* host, View* view) override;

  // Notification that a view has been removed.
  void ViewRemoved(View* host, View* view) override;

  // Layouts out the components.
  void Layout(View* host) override;

  // Returns the preferred size for the GridLayout.
  gfx::Size GetPreferredSize(const View* host) const override;

  int GetPreferredHeightForWidth(const View* host, int width) const override;

  void set_minimum_size(const gfx::Size& size) { minimum_size_ = size; }

 private:
  explicit GridLayout(View* host);

  // As both Layout and GetPreferredSize need to do nearly the same thing,
  // they both call into this method. This sizes the Columns/Rows as
  // appropriate. If layout is true, width/height give the width/height the
  // of the host, otherwise they are ignored.
  void SizeRowsAndColumns(bool layout,
                          int width,
                          int height,
                          gfx::Size* pref) const;

  // Calculates the master columns of all the column sets. See Column for
  // a description of what a master column is.
  void CalculateMasterColumnsIfNecessary() const;

  // This is called internally from AddView. It adds the ViewState to the
  // appropriate structures, and updates internal fields such as next_column_.
  void AddViewState(std::unique_ptr<ViewState> view_state);

  // Adds the Row to rows_, as well as updating next_column_,
  // current_row_col_set ...
  void AddRow(std::unique_ptr<Row> row);

  // As the name says, updates the remaining_height of the ViewState for
  // all Rows the supplied ViewState touches.
  void UpdateRemainingHeightFromRows(ViewState* state) const;

  // If the view state's remaining height is > 0, it is distributed among
  // the rows the view state touches. This is used during layout to make
  // sure the Rows can accommodate a view.
  void DistributeRemainingHeight(ViewState* state) const;

  // Advances next_column_ past any padding columns.
  void SkipPaddingColumns();

  // Returns the column set of the last non-padding row.
  ColumnSet* GetLastValidColumnSet();

  // The view we were created with. We don't own this.
  View* const host_;

  // Whether or not we've calculated the master/linked columns.
  mutable bool calculated_master_columns_;

  // Used to verify a view isn't added with a row span that expands into
  // another column structure.
  int remaining_row_span_;

  // Current row.
  int current_row_;

  // Current column.
  int next_column_;

  // Column set for the current row. This is null for padding rows.
  ColumnSet* current_row_col_set_;

  // Set to true when adding a View.
  bool adding_view_;

  // ViewStates. This is ordered by row_span in ascending order.
  mutable std::vector<std::unique_ptr<ViewState>> view_states_;

  // ColumnSets.
  mutable std::vector<std::unique_ptr<ColumnSet>> column_sets_;

  // Rows.
  mutable std::vector<std::unique_ptr<Row>> rows_;

  // Minimum preferred size.
  gfx::Size minimum_size_;

  DISALLOW_COPY_AND_ASSIGN(GridLayout);
};

// ColumnSet is used to define a set of columns. GridLayout may have any
// number of ColumnSets. You don't create a ColumnSet directly, instead
// use the AddColumnSet method of GridLayout.
class VIEWS_EXPORT ColumnSet {
 public:
  ~ColumnSet();

  // Adds a column for padding. When adding views, padding columns are
  // automatically skipped. For example, if you create a column set with
  // two columns separated by a padding column, the second AddView automatically
  // skips past the padding column. That is, to add two views, do:
  // layout->AddView(v1); layout->AddView(v2);, not:
  // layout->AddView(v1); layout->SkipColumns(1); layout->AddView(v2);
  // See class description for details on |resize_percent|.
  void AddPaddingColumn(float resize_percent, int width);

  // Adds a column. The alignment gives the default alignment for views added
  // with no explicit alignment. fixed_width gives a specific width for the
  // column, and is only used if size_type == FIXED. min_width gives the
  // minimum width for the column.
  //
  // If none of the columns in a columnset are resizable, the views are only
  // made as wide as the widest views in each column, even if extra space is
  // provided. In other words, GridLayout does not automatically resize views
  // unless the column is marked as resizable.
  // See class description for details on |resize_percent|.
  void AddColumn(GridLayout::Alignment h_align,
                 GridLayout::Alignment v_align,
                 float resize_percent,
                 GridLayout::SizeType size_type,
                 int fixed_width,
                 int min_width);

  // Forces the specified columns to have the same size. The size of
  // linked columns is that of the max of the specified columns. This
  // must end with -1. For example, the following forces the first and
  // second column to have the same size:
  // LinkColumnSizes(0, 1, -1);
  void LinkColumnSizes(int first, ...);

  // When sizing linked columns, columns wider than |size_limit| are ignored.
  void set_linked_column_size_limit(int size_limit) {
    linked_column_size_limit_ = size_limit;
  }

  // ID of this ColumnSet.
  int id() const { return id_; }

  int num_columns() const { return static_cast<int>(columns_.size()); }

 private:
  friend class GridLayout;

  explicit ColumnSet(int id);

  void AddColumn(GridLayout::Alignment h_align,
                 GridLayout::Alignment v_align,
                 float resize_percent,
                 GridLayout::SizeType size_type,
                 int fixed_width,
                 int min_width,
                 bool is_padding);

  void AddViewState(ViewState* view_state);

  // Set description of these.
  void CalculateMasterColumns();
  void AccumulateMasterColumns();

  // Sets the size of each linked column to be the same.
  void UnifyLinkedColumnSizes();

  // Updates the remaining width field of the ViewState from that of the
  // columns the view spans.
  void UpdateRemainingWidth(ViewState* view_state);

  // Makes sure the columns touched by view state are big enough for the
  // view.
  void DistributeRemainingWidth(ViewState* view_state);

  // Returns the total size needed for this ColumnSet.
  int LayoutWidth();

  // Returns the width of the specified columns.
  int GetColumnWidth(int start_col, int col_span);

  // Updates the x coordinate of each column from the previous ones.
  // NOTE: this doesn't include the insets.
  void ResetColumnXCoordinates();

  // Calculate the preferred width of each view in this column set, as well
  // as updating the remaining_width.
  void CalculateSize();

  // Distributes delta among the resizable columns.
  void Resize(int delta);

  // ID for this columnset.
  const int id_;

  // Columns wider than this limit will be ignored when computing linked
  // columns' sizes.
  int linked_column_size_limit_;

  // The columns.
  std::vector<std::unique_ptr<Column>> columns_;

  // The ViewStates. This is sorted based on column_span in ascending
  // order.
  std::vector<ViewState*> view_states_;

  // The master column of those columns that are linked. See Column
  // for a description of what the master column is.
  std::vector<Column*> master_columns_;

  DISALLOW_COPY_AND_ASSIGN(ColumnSet);
};

}  // namespace views

#endif  // UI_VIEWS_LAYOUT_GRID_LAYOUT_H_
