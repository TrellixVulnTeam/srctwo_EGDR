// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_WM_SPLIT_VIEW_SPLIT_VIEW_OVERVIEW_OVERLAY_H_
#define ASH_WM_SPLIT_VIEW_SPLIT_VIEW_OVERVIEW_OVERLAY_H_

#include <memory>

#include "ash/ash_export.h"
#include "base/gtest_prod_util.h"
#include "base/macros.h"
#include "ui/gfx/geometry/point.h"

namespace views {
class Widget;
}  // namespace views

namespace ash {

// An overlay in overview mode which guides users while they are attempting to
// enter splitview.
class ASH_EXPORT SplitViewOverviewOverlay {
 public:
  SplitViewOverviewOverlay();
  ~SplitViewOverviewOverlay();

  // Sets visiblity. The correct indicators will become visible based on the
  // split view controllers state. If |event_location| is located on a different
  // root window than |widget_|, |widget_| will reparent.
  void SetVisible(bool visible, const gfx::Point& event_location);
  bool visible() const;

 private:
  FRIEND_TEST_ALL_PREFIXES(WindowSelectorTest,
                           SplitViewOverviewOverlayWidgetReparenting);
  class SplitViewOverviewOverlayView;

  // The root content view of |widget_|.
  SplitViewOverviewOverlayView* overlay_view_ = nullptr;

  // The SplitViewOverviewOverlay widget. It covers the entire root window
  // and displays regions and text indicating where users should drag windows
  // enter split view.
  std::unique_ptr<views::Widget> widget_;

  DISALLOW_COPY_AND_ASSIGN(SplitViewOverviewOverlay);
};

}  // namespace ash

#endif  // ASH_WM_SPLIT_VIEW_SPLIT_VIEW_OVERVIEW_OVERLAY_H_
