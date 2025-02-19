// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_SIGNIN_SIGNIN_EMAIL_CONFIRMATION_DIALOG_H_
#define CHROME_BROWSER_UI_WEBUI_SIGNIN_SIGNIN_EMAIL_CONFIRMATION_DIALOG_H_

#include "base/callback.h"
#include "base/macros.h"
#include "chrome/browser/ui/webui/constrained_web_dialog_ui.h"
#include "ui/web_dialogs/web_dialog_delegate.h"
#include "ui/web_dialogs/web_dialog_ui.h"

class WebUIMessageHandler;
class Profile;

namespace content {
class WebContents;
}

// A tab-modal dialog to ask the user to confirm his email before signing in.
class SigninEmailConfirmationDialog : public ui::WebDialogDelegate {
 public:
  // Actions that can be taken when the user is asked to confirm their account.
  enum Action {
    // The user chose not to sign in to the current profile and wants chrome
    // to create a new profile instead.
    CREATE_NEW_USER,

    // The user chose to sign in and enable sync in the current profile.
    START_SYNC,

    // The user chose abort sign in.
    CLOSE
  };

  // Callback indicating action performed by the user.
  using Callback = base::Callback<void(Action)>;

  // Create and show the dialog, which owns itself.
  // Ask the user for confirmation before starting to sync.
  static void AskForConfirmation(content::WebContents* contents,
                                 Profile* profile,
                                 const std::string& last_email,
                                 const std::string& email,
                                 const Callback& callback);

 private:
  class DialogWebContentsObserver;

  SigninEmailConfirmationDialog(content::WebContents* contents,
                                Profile* profile,
                                const std::string& last_email,
                                const std::string& new_email,
                                const Callback& callback);
  ~SigninEmailConfirmationDialog() override;

  // WebDialogDelegate implementation.
  ui::ModalType GetDialogModalType() const override;
  base::string16 GetDialogTitle() const override;
  GURL GetDialogContentURL() const override;
  void GetWebUIMessageHandlers(
      std::vector<content::WebUIMessageHandler*>* handlers) const override;
  void GetDialogSize(gfx::Size* size) const override;
  std::string GetDialogArgs() const override;
  void OnDialogClosed(const std::string& json_retval) override;
  void OnCloseContents(content::WebContents* source,
                       bool* out_close_dialog) override;
  bool ShouldShowDialogTitle() const override;

  // Shows the dialog and releases ownership of this object. It will
  // delete itself when the dialog is closed.
  void ShowDialog();

  // Closes the dialog.
  void CloseDialog();

  // Resets the dialog observer.
  void ResetDialogObserver();

  // Returns the media router dialog WebContents.
  // Returns nullptr if there is no dialog.
  content::WebContents* GetDialogWebContents() const;

  // Web contents from which the "Learn more" link should be opened.
  content::WebContents* const web_contents_;
  Profile* const profile_;

  std::string last_email_;
  std::string new_email_;
  Callback callback_;

  // Observer for lifecycle events of the web contents of the dialog.
  std::unique_ptr<DialogWebContentsObserver> dialog_observer_;

  DISALLOW_COPY_AND_ASSIGN(SigninEmailConfirmationDialog);
};

#endif  // CHROME_BROWSER_UI_WEBUI_SIGNIN_SIGNIN_EMAIL_CONFIRMATION_DIALOG_H_
