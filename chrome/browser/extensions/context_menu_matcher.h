// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_CONTEXT_MENU_MATCHER_H_
#define CHROME_BROWSER_EXTENSIONS_CONTEXT_MENU_MATCHER_H_

#include <stddef.h>

#include <map>
#include <memory>
#include <vector>

#include "base/callback.h"
#include "base/macros.h"
#include "chrome/browser/extensions/menu_manager.h"
#include "ui/base/models/simple_menu_model.h"

class ExtensionContextMenuBrowserTest;

namespace content {
class BrowserContext;
class RenderFrameHost;
}

namespace extensions {

// This class contains code that is shared between the various places where
// context menu items added by the extension or app should be shown.
class ContextMenuMatcher {
 public:
  static const size_t kMaxExtensionItemTitleLength;

  // Convert a command ID so that it fits within the range for
  // extension context menu.
  static int ConvertToExtensionsCustomCommandId(int id);

  // Returns true if the given id is one generated for extension context menu.
  static bool IsExtensionsCustomCommandId(int id);

  // The |filter| will be called on possibly matching menu items, and its
  // result is used to determine which items to actually append to the menu.
  ContextMenuMatcher(content::BrowserContext* context,
                     ui::SimpleMenuModel::Delegate* delegate,
                     ui::SimpleMenuModel* menu_model,
                     const base::Callback<bool(const MenuItem*)>& filter);

  // This is a helper function to append items for one particular extension.
  // The |index| parameter is used for assigning id's, and is incremented for
  // each item actually added. |is_action_menu| is used for browser and page
  // action context menus, in which menu items are not placed in submenus
  // and the extension's icon is not shown.
  void AppendExtensionItems(const MenuItem::ExtensionKey& extension_key,
                            const base::string16& selection_text,
                            int* index,
                            bool is_action_menu);

  void Clear();

  // This function returns the top level context menu title of an extension
  // based on a printable selection text.
  base::string16 GetTopLevelContextMenuTitle(
      const MenuItem::ExtensionKey& extension_key,
      const base::string16& selection_text);

  bool IsCommandIdChecked(int command_id) const;
  bool IsCommandIdVisible(int command_id) const;
  bool IsCommandIdEnabled(int command_id) const;
  void ExecuteCommand(int command_id,
                      content::WebContents* web_contents,
                      content::RenderFrameHost* render_frame_host,
                      const content::ContextMenuParams& params);

 private:
  friend class ::ExtensionContextMenuBrowserTest;
  friend class ExtensionContextMenuApiTest;

  bool GetRelevantExtensionTopLevelItems(
      const MenuItem::ExtensionKey& extension_key,
      const Extension** extension,
      bool* can_cross_incognito,
      MenuItem::List* items);

  MenuItem::List GetRelevantExtensionItems(const MenuItem::OwnedList& items,
                                           bool can_cross_incognito);

  // Used for recursively adding submenus of extension items.
  void RecursivelyAppendExtensionItems(const MenuItem::List& items,
                                       bool can_cross_incognito,
                                       const base::string16& selection_text,
                                       ui::SimpleMenuModel* menu_model,
                                       int* index,
                                       bool is_action_menu_top_level);

  // Attempts to get an MenuItem given the id of a context menu item.
  extensions::MenuItem* GetExtensionMenuItem(int id) const;

  // This will set the icon on the most recently-added item in the menu_model_.
  void SetExtensionIcon(const std::string& extension_id);

  content::BrowserContext* browser_context_;
  ui::SimpleMenuModel* menu_model_;
  ui::SimpleMenuModel::Delegate* delegate_;

  base::Callback<bool(const MenuItem*)> filter_;

  // Maps the id from a context menu item to the MenuItem's internal id.
  std::map<int, extensions::MenuItem::Id> extension_item_map_;

  // Keep track of and clean up menu models for submenus.
  std::vector<std::unique_ptr<ui::SimpleMenuModel>> extension_menu_models_;

  DISALLOW_COPY_AND_ASSIGN(ContextMenuMatcher);
};

}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_CONTEXT_MENU_MATCHER_H_
