// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_ASH_LAUNCHER_APP_SHORTCUT_LAUNCHER_ITEM_CONTROLLER_H_
#define CHROME_BROWSER_UI_ASH_LAUNCHER_APP_SHORTCUT_LAUNCHER_ITEM_CONTROLLER_H_

#include <string>
#include <vector>

#include "ash/public/cpp/shelf_item_delegate.h"
#include "base/macros.h"
#include "base/time/time.h"
#include "url/gurl.h"

class Browser;
class URLPattern;

namespace content {
class WebContents;
}

namespace extensions {
class Extension;
}

// Item controller for an app shortcut. Shortcuts track app and launcher ids,
// but do not have any associated windows (opening a shortcut will replace the
// item with the appropriate ash::ShelfItemDelegate type).
class AppShortcutLauncherItemController : public ash::ShelfItemDelegate {
 public:
  ~AppShortcutLauncherItemController() override;

  static std::unique_ptr<AppShortcutLauncherItemController> Create(
      const ash::ShelfID& shelf_id);

  std::vector<content::WebContents*> GetRunningApplications();

  // ash::ShelfItemDelegate overrides:
  void ItemSelected(std::unique_ptr<ui::Event> event,
                    int64_t display_id,
                    ash::ShelfLaunchSource source,
                    ItemSelectedCallback callback) override;
  ash::MenuItemList GetAppMenuItems(int event_flags) override;
  std::unique_ptr<ui::MenuModel> GetContextMenu(int64_t display_id) override;
  void ExecuteCommand(bool from_context_menu,
                      int64_t command_id,
                      int32_t event_flags,
                      int64_t display_id) override;
  void Close() override;

  // Get the refocus url pattern, which can be used to identify this application
  // from a URL link.
  const GURL& refocus_url() const { return refocus_url_; }
  // Set the refocus url pattern. Used by unit tests.
  void set_refocus_url(const GURL& refocus_url) { refocus_url_ = refocus_url; }

 protected:
  explicit AppShortcutLauncherItemController(const ash::ShelfID& shelf_id);

 private:
  // Get the last running application.
  content::WebContents* GetLRUApplication();

  // Returns true if this app matches the given |web_contents|. To accelerate
  // the matching, the app managing |extension| as well as the parsed
  // |refocus_pattern| get passed. If |is_app| is true, the application gets
  // first checked against its original URL since a windowed app might have
  // navigated away from its app domain.
  bool WebContentMatchesApp(const extensions::Extension* extension,
                            const URLPattern& refocus_pattern,
                            content::WebContents* web_contents,
                            Browser* browser);

  // Activate the browser with the given |content| and show the associated tab.
  // Returns the action performed by activating the content.
  ash::ShelfAction ActivateContent(content::WebContents* content);

  // Advance to the next item if an owned item is already active. The function
  // will return true if it has successfully advanced.
  bool AdvanceToNextApp();

  // Returns true if the application is a V2 app.
  bool IsV2App();

  // Returns true if it is allowed to try starting a V2 app again.
  bool AllowNextLaunchAttempt();

  GURL refocus_url_;

  // Since V2 applications can be undetectable after launching, this timer is
  // keeping track of the last launch attempt.
  base::Time last_launch_attempt_;

  // The cached list of open app web contents shown in an application menu.
  std::vector<content::WebContents*> app_menu_items_;

  DISALLOW_COPY_AND_ASSIGN(AppShortcutLauncherItemController);
};

#endif  // CHROME_BROWSER_UI_ASH_LAUNCHER_APP_SHORTCUT_LAUNCHER_ITEM_CONTROLLER_H_
