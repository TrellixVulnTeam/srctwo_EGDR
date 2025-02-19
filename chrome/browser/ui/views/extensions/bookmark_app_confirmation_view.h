// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_EXTENSIONS_BOOKMARK_APP_CONFIRMATION_VIEW_H_
#define CHROME_BROWSER_UI_VIEWS_EXTENSIONS_BOOKMARK_APP_CONFIRMATION_VIEW_H_

#include "base/macros.h"
#include "base/strings/string16.h"
#include "chrome/browser/ui/browser_dialogs.h"
#include "chrome/common/web_application_info.h"
#include "ui/views/controls/textfield/textfield_controller.h"
#include "ui/views/window/dialog_delegate.h"

namespace views {
class Checkbox;
class Textfield;
}

// BookmarkAppConfirmationView provides views for editing the details to
// create a bookmark app with. (More tools > Add to desktop)
class BookmarkAppConfirmationView : public views::DialogDelegateView,
                                    public views::TextfieldController {
 public:
  BookmarkAppConfirmationView(const WebApplicationInfo& web_app_info,
                              chrome::ShowBookmarkAppDialogCallback callback);
  ~BookmarkAppConfirmationView() override;

 private:
  // Overridden from views::WidgetDelegate:
  views::View* GetInitiallyFocusedView() override;
  ui::ModalType GetModalType() const override;
  base::string16 GetWindowTitle() const override;
  bool ShouldShowCloseButton() const override;
  void WindowClosing() override;

  // Overriden from views::DialogDelegateView:
  bool Accept() override;
  base::string16 GetDialogButtonLabel(ui::DialogButton button) const override;
  bool IsDialogButtonEnabled(ui::DialogButton button) const override;

  // Overridden from views::View:
  gfx::Size GetMinimumSize() const override;

  // Overridden from views::TextfieldController:
  void ContentsChanged(views::Textfield* sender,
                       const base::string16& new_contents) override;

  // Update the state of the Add button.
  void UpdateAddButtonState();

  // Get the trimmed contents of the title text field.
  base::string16 GetTrimmedTitle() const;

  // The WebApplicationInfo that the user is editing.
  WebApplicationInfo web_app_info_;

  // The callback to be invoked when the dialog is completed.
  chrome::ShowBookmarkAppDialogCallback callback_;

  // Checkbox to launch as a window.
  views::Checkbox* open_as_window_checkbox_;

  // Textfield showing the title of the app.
  views::Textfield* title_tf_;

  DISALLOW_COPY_AND_ASSIGN(BookmarkAppConfirmationView);
};

#endif  // CHROME_BROWSER_UI_VIEWS_EXTENSIONS_BOOKMARK_APP_CONFIRMATION_VIEW_H_
