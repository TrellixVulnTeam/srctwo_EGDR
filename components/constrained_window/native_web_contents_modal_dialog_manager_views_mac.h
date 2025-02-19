// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/constrained_window/native_web_contents_modal_dialog_manager_views.h"

namespace web_modal {
class SingleWebContentsDialogManagerDelegate;
}

namespace constrained_window {

// Class for parenting a Mac Cocoa sheet on a views tab modal dialog off of a
// views browser, e.g. for tab-modal Cocoa sheets. Since Cocoa sheets are modal
// to the parent window, the sheet is instead parented to an invisible views
// overlay window which is tab-modal.
class NativeWebContentsModalDialogManagerViewsMac
    : public NativeWebContentsModalDialogManagerViews {
 public:
  NativeWebContentsModalDialogManagerViewsMac(
      gfx::NativeWindow dialog,
      web_modal::SingleWebContentsDialogManagerDelegate* native_delegate);

  // NativeWebContentsModalDialogManagerViews:
  void OnPositionRequiresUpdate() override;
  void ShowWidget(views::Widget* widget) override;
  void HideWidget(views::Widget* widget) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(NativeWebContentsModalDialogManagerViewsMac);
};

}  // namespace constrained_window
