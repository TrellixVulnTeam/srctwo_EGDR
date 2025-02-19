// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_HOST_IT2ME_IT2ME_CONFIRMATION_DIALOG_PROXY_H_
#define REMOTING_HOST_IT2ME_IT2ME_CONFIRMATION_DIALOG_PROXY_H_

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/single_thread_task_runner.h"
#include "remoting/host/it2me/it2me_confirmation_dialog.h"

namespace remoting {

// A helper class to use an It2MeConfirmationDialog from a non-UI thread.
class It2MeConfirmationDialogProxy : public It2MeConfirmationDialog {
 public:
  // |ui_task_runner| must be the UI thread. It will be used to call into the
  // wrapped dialog.
  // |dialog| is the dialog being wrapped.
  It2MeConfirmationDialogProxy(
      scoped_refptr<base::SingleThreadTaskRunner> ui_task_runner,
      std::unique_ptr<It2MeConfirmationDialog> dialog);

  ~It2MeConfirmationDialogProxy() override;

  // It2MeConfirmationDialog implementation.
  void Show(const std::string& remote_user_email,
            const It2MeConfirmationDialog::ResultCallback& callback) override;

 private:
  class Core;

  void ReportResult(It2MeConfirmationDialog::Result result);

  std::unique_ptr<Core> core_;
  It2MeConfirmationDialog::ResultCallback callback_;
  base::WeakPtrFactory<It2MeConfirmationDialogProxy> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(It2MeConfirmationDialogProxy);
};

}  // namespace remoting

#endif  // REMOTING_HOST_IT2ME_IT2ME_CONFIRMATION_DIALOG_PROXY_H_
