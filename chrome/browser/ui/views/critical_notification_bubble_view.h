// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_CRITICAL_NOTIFICATION_BUBBLE_VIEW_H_
#define CHROME_BROWSER_UI_VIEWS_CRITICAL_NOTIFICATION_BUBBLE_VIEW_H_

#include "base/macros.h"
#include "base/timer/timer.h"
#include "ui/views/bubble/bubble_dialog_delegate.h"

class CriticalNotificationBubbleView : public views::BubbleDialogDelegateView {
 public:
  explicit CriticalNotificationBubbleView(views::View* anchor_view);
  ~CriticalNotificationBubbleView() override;

  // views::BubbleDialogDelegateView overrides:
  base::string16 GetWindowTitle() const override;
  void WindowClosing() override;
  bool Cancel() override;
  bool Accept() override;
  void Init() override;
  base::string16 GetDialogButtonLabel(ui::DialogButton button) const override;
  void GetAccessibleNodeData(ui::AXNodeData* node_data) override;
  void ViewHierarchyChanged(
      const ViewHierarchyChangedDetails& details) override;

 private:
  // Helper function to calculate the remaining time (in seconds) until
  // spontaneous reboot.
  int GetRemainingTime() const;

  // Called when the timer fires each time the clock ticks.
  void OnCountdown();

  // A timer to refresh the bubble to show new countdown value.
  base::RepeatingTimer refresh_timer_;

  // When the bubble was created.
  base::TimeTicks bubble_created_;

  DISALLOW_COPY_AND_ASSIGN(CriticalNotificationBubbleView);
};

#endif  // CHROME_BROWSER_UI_VIEWS_CRITICAL_NOTIFICATION_BUBBLE_VIEW_H_
