// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_COCOA_TAB_CONTENTS_WEB_DRAG_BOOKMARK_HANDLER_MAC_H_
#define CHROME_BROWSER_UI_COCOA_TAB_CONTENTS_WEB_DRAG_BOOKMARK_HANDLER_MAC_H_

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "components/bookmarks/browser/bookmark_node_data.h"
#include "content/public/browser/web_drag_dest_delegate.h"

class BookmarkTabHelper;

namespace content {
class WebContents;
}

// Chrome needs to intercept content drag events so it can dispatch them to the
// bookmarks and extensions system.
class WebDragBookmarkHandlerMac : public content::WebDragDestDelegate {
 public:
  WebDragBookmarkHandlerMac();
  ~WebDragBookmarkHandlerMac() override;

  // Overridden from content::WebDragDestDelegate:
  void DragInitialize(content::WebContents* contents) override;
  void OnDragOver() override;
  void OnDragEnter() override;
  void OnDrop() override;
  void OnDragLeave() override;

 private:
  // The BookmarkTabHelper.
  // Weak reference; may be NULL if the contents don't have a
  // BookmarkTabHelper (e.g. WebUI dialogs).
  BookmarkTabHelper* bookmark_tab_helper_;

  content::WebContents* web_contents_;

  // The bookmark data for the current tab. This will be empty if there is not
  // a native bookmark drag.
  bookmarks::BookmarkNodeData bookmark_drag_data_;

  DISALLOW_COPY_AND_ASSIGN(WebDragBookmarkHandlerMac);
};

#endif  // CHROME_BROWSER_UI_COCOA_TAB_CONTENTS_WEB_DRAG_BOOKMARK_HANDLER_MAC_H_
