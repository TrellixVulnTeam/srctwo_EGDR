// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/app_modal/app_modal_dialog_queue.h"

#include "base/memory/singleton.h"
#include "components/app_modal/javascript_app_modal_dialog.h"

namespace app_modal {

// static
AppModalDialogQueue* AppModalDialogQueue::GetInstance() {
  return base::Singleton<AppModalDialogQueue>::get();
}

void AppModalDialogQueue::AddDialog(JavaScriptAppModalDialog* dialog) {
  if (!active_dialog_) {
    ShowModalDialog(dialog);
    return;
  }
  app_modal_dialog_queue_.push_back(dialog);
}

void AppModalDialogQueue::ShowNextDialog() {
  JavaScriptAppModalDialog* dialog = GetNextDialog();
  if (dialog)
    ShowModalDialog(dialog);
  else
    active_dialog_ = nullptr;
}

void AppModalDialogQueue::ActivateModalDialog() {
  if (showing_modal_dialog_) {
    // As part of showing a modal dialog we may end up back in this method
    // (showing a dialog activates the WebContents, which can trigger a call
    // to ActivateModalDialog). We ignore such a request as after the call to
    // activate the tab contents the dialog is shown.
    return;
  }
  if (active_dialog_)
    active_dialog_->ActivateModalDialog();
}

bool AppModalDialogQueue::HasActiveDialog() const {
  return active_dialog_ != nullptr;
}

AppModalDialogQueue::AppModalDialogQueue()
    : active_dialog_(NULL),
      showing_modal_dialog_(false) {
}

AppModalDialogQueue::~AppModalDialogQueue() {
}

void AppModalDialogQueue::ShowModalDialog(JavaScriptAppModalDialog* dialog) {
  // Be sure and set the active_dialog_ field first, otherwise if
  // ShowModalDialog triggers a call back to the queue they'll get the old
  // dialog. Also, if the dialog calls |ShowNextDialog()| before returning, that
  // would write nullptr into |active_dialog_| and this function would then undo
  // that.
  active_dialog_ = dialog;
  showing_modal_dialog_ = true;
  dialog->ShowModalDialog();
  showing_modal_dialog_ = false;
}

JavaScriptAppModalDialog* AppModalDialogQueue::GetNextDialog() {
  while (!app_modal_dialog_queue_.empty()) {
    JavaScriptAppModalDialog* dialog = app_modal_dialog_queue_.front();
    app_modal_dialog_queue_.pop_front();
    if (dialog->IsValid())
      return dialog;
    delete dialog;
  }
  return nullptr;
}

}  // namespace app_modal
