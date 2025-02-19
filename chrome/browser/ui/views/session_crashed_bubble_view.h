// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_SESSION_CRASHED_BUBBLE_VIEW_H_
#define CHROME_BROWSER_UI_VIEWS_SESSION_CRASHED_BUBBLE_VIEW_H_

#include <memory>

#include "base/macros.h"
#include "chrome/browser/ui/session_crashed_bubble.h"
#include "ui/views/bubble/bubble_dialog_delegate.h"
#include "ui/views/controls/styled_label_listener.h"

namespace views {
class Checkbox;
class Widget;
}

class Browser;

// SessionCrashedBubbleView shows a bubble allowing the user to restore the
// previous session. If metrics reporting is not enabled a checkbox is presented
// allowing the user to turn it on.
class SessionCrashedBubbleView : public SessionCrashedBubble,
                                 public views::BubbleDialogDelegateView,
                                 public views::StyledLabelListener {
 public:
  // A helper class that listens to browser removal event.
  class BrowserRemovalObserver;

  // Creates and shows the session crashed bubble, with |uma_opted_in_already|
  // indicating whether the user has already opted-in to UMA. It will be called
  // by Show. It takes ownership of |browser_observer|.
  static void ShowForReal(
      std::unique_ptr<BrowserRemovalObserver> browser_observer,
      bool uma_opted_in_already);

 private:
  friend class SessionCrashedBubbleViewTest;

  SessionCrashedBubbleView(views::View* anchor_view,
                           Browser* browser,
                           bool offer_uma_optin);
  ~SessionCrashedBubbleView() override;

  // WidgetDelegateView methods.
  base::string16 GetWindowTitle() const override;
  bool ShouldShowWindowTitle() const override;
  bool ShouldShowCloseButton() const override;
  void OnWidgetDestroying(views::Widget* widget) override;
  views::View* CreateFootnoteView() override;
  bool Accept() override;
  bool Cancel() override;
  bool Close() override;
  int GetDialogButtons() const override;
  base::string16 GetDialogButtonLabel(ui::DialogButton button) const override;

  // views::BubbleDialogDelegateView methods.
  void Init() override;

  // views::StyledLabelListener methods.
  void StyledLabelLinkClicked(views::StyledLabel* label,
                              const gfx::Range& range,
                              int event_flags) override;

  // Restore previous session after user selects so.
  void RestorePreviousSession();

  // Open startup pages after user selects so.
  void OpenStartupPages();

  // Enable UMA if the user accepted the offer.
  void MaybeEnableUMA();

  // Close and destroy the bubble.
  void CloseBubble();

  // Used for opening the question mark link as well as access the tab strip.
  Browser* browser_;

  // Checkbox for the user to opt-in to UMA reporting.
  views::Checkbox* uma_option_;

  // Whether or not the UMA opt-in option should be shown.
  bool offer_uma_optin_;

  // Whether or not the user ignored the bubble. It is used to collect bubble
  // usage stats.
  bool ignored_;

  DISALLOW_COPY_AND_ASSIGN(SessionCrashedBubbleView);
};

#endif  // CHROME_BROWSER_UI_VIEWS_SESSION_CRASHED_BUBBLE_VIEW_H_
