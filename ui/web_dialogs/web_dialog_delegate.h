// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_WEB_DIALOGS_WEB_DIALOG_DELEGATE_H_
#define UI_WEB_DIALOGS_WEB_DIALOG_DELEGATE_H_

#include <string>
#include <vector>

#include "base/strings/string16.h"
#include "content/public/browser/web_contents_delegate.h"
#include "ui/base/ui_base_types.h"
#include "ui/base/window_open_disposition.h"
#include "ui/web_dialogs/web_dialogs_export.h"

class GURL;

namespace content {
class RenderViewHost;
class WebContents;
class WebUI;
class WebUIMessageHandler;
struct ContextMenuParams;
struct OpenURLParams;
}

namespace gfx {
class Rect;
class Size;
}

namespace ui {
class Accelerator;

// Implement this class to receive notifications.
class WEB_DIALOGS_EXPORT WebDialogDelegate {
 public:
  // Returns true if the contents needs to be run in a modal dialog.
  virtual ModalType GetDialogModalType() const = 0;

  // Returns the title of the dialog.
  virtual base::string16 GetDialogTitle() const = 0;

  // Returns the dialog's name identifier. Used to identify this dialog for
  // state restoration.
  virtual std::string GetDialogName() const;

  // Get the HTML file path for the content to load in the dialog.
  virtual GURL GetDialogContentURL() const = 0;

  // Get WebUIMessageHandler objects to handle messages from the HTML/JS page.
  // The handlers are used to send and receive messages from the page while it
  // is still open.  Ownership of each handler is taken over by the WebUI
  // hosting the page.
  virtual void GetWebUIMessageHandlers(
      std::vector<content::WebUIMessageHandler*>* handlers) const = 0;

  // Get the size of the dialog.
  virtual void GetDialogSize(gfx::Size* size) const = 0;

  // Get the size of the dialog.
  virtual void GetMinimumDialogSize(gfx::Size* size) const;

  // Gets the JSON string input to use when showing the dialog.
  virtual std::string GetDialogArgs() const = 0;

  // Returns true to signal that the dialog can be closed. Specialized
  // WebDialogDelegate subclasses can override this default behavior to allow
  // the close to be blocked until the user corrects mistakes, accepts an
  // agreement, etc.
  virtual bool CanCloseDialog() const;

  // Returns true if the dialog can ever be resized. Default implementation
  // returns |true|.
  virtual bool CanResizeDialog() const;

  // A callback to notify the delegate that |source|'s loading state has
  // changed.
  virtual void OnLoadingStateChanged(content::WebContents* source) {}

  // A callback to notify the delegate that a web dialog has been shown.
  // |webui| is the WebUI with which the dialog is associated.
  // |render_view_host| is the RenderViewHost for the shown dialog.
  virtual void OnDialogShown(content::WebUI* webui,
                             content::RenderViewHost* render_view_host) {}

  // A callback to notify the delegate that the dialog closed.
  // IMPORTANT: Implementations should delete |this| here (unless they've
  // arranged for the delegate to be deleted in some other way, e.g. by
  // registering it as a message handler in the WebUI object).
  virtual void OnDialogClosed(const std::string& json_retval) = 0;

  // A callback to notify the delegate that the dialog is being closed in
  // response to a "dialogClose" message from WebUI.
  virtual void OnDialogCloseFromWebUI(const std::string& json_retval);

  // A callback to notify the delegate that the contents have gone
  // away. Only relevant if your dialog hosts code that calls
  // windows.close() and you've allowed that.  If the output parameter
  // is set to true, then the dialog is closed.  The default is false.
  // |out_close_dialog| is never NULL.
  virtual void OnCloseContents(content::WebContents* source,
                               bool* out_close_dialog) = 0;

  // A callback to allow the delegate to dictate that the window should not
  // have a title bar.  This is useful when presenting branded interfaces.
  virtual bool ShouldShowDialogTitle() const = 0;

  // A callback to allow the delegate to inhibit context menu or show
  // customized menu.
  // Returns true iff you do NOT want the standard context menu to be
  // shown (because you want to handle it yourself).
  virtual bool HandleContextMenu(const content::ContextMenuParams& params);

  // A callback to allow the delegate to open a new URL inside |source|.
  // On return |out_new_contents| should contain the WebContents the URL
  // is opened in. Return false to use the default handler.
  virtual bool HandleOpenURLFromTab(content::WebContents* source,
                                    const content::OpenURLParams& params,
                                    content::WebContents** out_new_contents);

  // A callback to create a new tab with |new_contents|. |source| is the
  // WebContent where the operation originated. |disposition| controls how the
  // new tab should be opened. |initial_rect| is the position and size of the
  // window if a new window is created. |user_gesture| is true if the operation
  // was started by a user gesture. Return false to use the default handler.
  virtual bool HandleAddNewContents(content::WebContents* source,
                                    content::WebContents* new_contents,
                                    WindowOpenDisposition disposition,
                                    const gfx::Rect& initial_rect,
                                    bool user_gesture);

  // A callback to control whether a WebContents will be created. Returns
  // false to disallow the creation. Return true to use the default handler.
  virtual bool HandleShouldCreateWebContents();

  // Stores the dialog bounds.
  virtual void StoreDialogSize(const gfx::Size& dialog_size) {}

  // Returns the accelerators handled by the delegate.
  virtual std::vector<Accelerator> GetAccelerators();

  // Returns true if |accelerator| is processed, otherwise false.
  virtual bool AcceleratorPressed(const Accelerator& accelerator);

  virtual ~WebDialogDelegate() {}
};

}  // namespace ui

#endif  // UI_WEB_DIALOGS_WEB_DIALOG_DELEGATE_H_
