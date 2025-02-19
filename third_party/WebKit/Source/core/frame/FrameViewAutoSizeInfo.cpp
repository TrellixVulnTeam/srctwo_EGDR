// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/frame/FrameViewAutoSizeInfo.h"

#include "core/frame/LocalFrame.h"
#include "core/frame/LocalFrameView.h"
#include "core/layout/LayoutBox.h"
#include "core/layout/LayoutView.h"
#include "core/layout/api/LayoutViewItem.h"
#include "platform/wtf/AutoReset.h"

namespace blink {

FrameViewAutoSizeInfo::FrameViewAutoSizeInfo(LocalFrameView* view)
    : frame_view_(view), in_auto_size_(false), did_run_autosize_(false) {
  DCHECK(frame_view_);
}

DEFINE_TRACE(FrameViewAutoSizeInfo) {
  visitor->Trace(frame_view_);
}

void FrameViewAutoSizeInfo::ConfigureAutoSizeMode(const IntSize& min_size,
                                                  const IntSize& max_size) {
  DCHECK(!min_size.IsEmpty());
  DCHECK_LE(min_size.Width(), max_size.Width());
  DCHECK_LE(min_size.Height(), max_size.Height());

  if (min_auto_size_ == min_size && max_auto_size_ == max_size)
    return;

  min_auto_size_ = min_size;
  max_auto_size_ = max_size;
  did_run_autosize_ = false;
}

void FrameViewAutoSizeInfo::AutoSizeIfNeeded() {
  if (in_auto_size_)
    return;

  AutoReset<bool> change_in_auto_size(&in_auto_size_, true);

  Document* document = frame_view_->GetFrame().GetDocument();
  if (!document || !document->IsActive())
    return;

  Element* document_element = document->documentElement();
  if (!document_element)
    return;

  // If this is the first time we run autosize, start from small height and
  // allow it to grow.
  if (!did_run_autosize_)
    frame_view_->Resize(frame_view_->FrameRect().Width(),
                        min_auto_size_.Height());

  IntSize size = frame_view_->FrameRect().Size();

  // Do the resizing twice. The first time is basically a rough calculation
  // using the preferred width which may result in a height change during the
  // second iteration.
  for (int i = 0; i < 2; i++) {
    // Update various sizes including contentsSize, scrollHeight, etc.
    document->UpdateStyleAndLayoutIgnorePendingStylesheets();

    LayoutViewItem layout_view_item = document->GetLayoutViewItem();
    if (layout_view_item.IsNull())
      return;

    int width = layout_view_item.MinPreferredLogicalWidth().ToInt();

    LayoutBox* document_layout_box = document_element->GetLayoutBox();
    if (!document_layout_box)
      return;

    int height = document_layout_box->ScrollHeight().ToInt();
    IntSize new_size(width, height);

    // Check to see if a scrollbar is needed for a given dimension and
    // if so, increase the other dimension to account for the scrollbar.
    // Since the dimensions are only for the view rectangle, once a
    // dimension exceeds the maximum, there is no need to increase it further.
    if (new_size.Width() > max_auto_size_.Width()) {
      Scrollbar* local_horizontal_scrollbar =
          frame_view_->HorizontalScrollbar();
      if (!local_horizontal_scrollbar)
        local_horizontal_scrollbar =
            frame_view_->CreateScrollbar(kHorizontalScrollbar);
      if (!local_horizontal_scrollbar->IsOverlayScrollbar())
        new_size.SetHeight(new_size.Height() +
                           local_horizontal_scrollbar->Height());

      // Don't bother checking for a vertical scrollbar because the width is at
      // already greater the maximum.
    } else if (new_size.Height() > max_auto_size_.Height()) {
      Scrollbar* local_vertical_scrollbar = frame_view_->VerticalScrollbar();
      if (!local_vertical_scrollbar)
        local_vertical_scrollbar =
            frame_view_->CreateScrollbar(kVerticalScrollbar);
      if (!local_vertical_scrollbar->IsOverlayScrollbar())
        new_size.SetWidth(new_size.Width() + local_vertical_scrollbar->Width());

      // Don't bother checking for a horizontal scrollbar because the height is
      // already greater the maximum.
    }

    // Ensure the size is at least the min bounds.
    new_size = new_size.ExpandedTo(min_auto_size_);

    // Bound the dimensions by the max bounds and determine what scrollbars to
    // show.
    ScrollbarMode horizonal_scrollbar_mode = kScrollbarAlwaysOff;
    if (new_size.Width() > max_auto_size_.Width()) {
      new_size.SetWidth(max_auto_size_.Width());
      horizonal_scrollbar_mode = kScrollbarAlwaysOn;
    }
    ScrollbarMode vertical_scrollbar_mode = kScrollbarAlwaysOff;
    if (new_size.Height() > max_auto_size_.Height()) {
      new_size.SetHeight(max_auto_size_.Height());
      vertical_scrollbar_mode = kScrollbarAlwaysOn;
    }

    if (new_size == size)
      continue;

    // While loading only allow the size to increase (to avoid twitching during
    // intermediate smaller states) unless autoresize has just been turned on or
    // the maximum size is smaller than the current size.
    if (did_run_autosize_ && size.Height() <= max_auto_size_.Height() &&
        size.Width() <= max_auto_size_.Width() &&
        !frame_view_->GetFrame().GetDocument()->LoadEventFinished() &&
        (new_size.Height() < size.Height() || new_size.Width() < size.Width()))
      break;

    frame_view_->Resize(new_size.Width(), new_size.Height());
    // Force the scrollbar state to avoid the scrollbar code adding them and
    // causing them to be needed. For example, a vertical scrollbar may cause
    // text to wrap and thus increase the height (which is the only reason the
    // scollbar is needed).
    frame_view_->SetVerticalScrollbarLock(false);
    frame_view_->SetHorizontalScrollbarLock(false);
    frame_view_->SetScrollbarModes(horizonal_scrollbar_mode,
                                   vertical_scrollbar_mode, true, true);
  }
  did_run_autosize_ = true;
}

}  // namespace blink
