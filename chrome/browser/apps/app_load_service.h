// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_APPS_APP_LOAD_SERVICE_H_
#define CHROME_BROWSER_APPS_APP_LOAD_SERVICE_H_

#include <map>
#include <string>

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "components/keyed_service/core/keyed_service.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "extensions/browser/extension_registry_observer.h"

namespace content {
class BrowserContext;
}

namespace apps {

// Monitors apps being reloaded and performs app specific actions (like launch
// or restart) on them. Also provides an interface to schedule these actions.
class AppLoadService : public KeyedService,
                       public content::NotificationObserver,
                       public extensions::ExtensionRegistryObserver {
 public:
  enum PostReloadActionType {
    LAUNCH_FOR_RELOAD,
    RESTART,
    LAUNCH_FOR_LOAD_AND_LAUNCH,
  };

  struct PostReloadAction {
    PostReloadAction();

    PostReloadActionType action_type;
    base::CommandLine command_line;
    base::FilePath current_dir;
  };

  explicit AppLoadService(content::BrowserContext* context);
  ~AppLoadService() override;

  // KeyedService support:
  void Shutdown() override;

  // Reload the application with the given id and then send it the OnRestarted
  // event.
  void RestartApplication(const std::string& extension_id);

  // Reload the application with the given id if it is currently running.
  void RestartApplicationIfRunning(const std::string& extension_id);

  // Loads (or reloads) the app with |extension_path|, then launches it. Any
  // command line parameters from |command_line| will be passed along via
  // launch parameters. Returns true if loading the extension has begun
  // successfully.
  bool LoadAndLaunch(const base::FilePath& extension_path,
                     const base::CommandLine& command_line,
                     const base::FilePath& current_dir);

  // Loads (or reloads) the app with |extension_path|. Returns true if loading
  // the app has begun successfully.
  bool Load(const base::FilePath& extension_path);

  static AppLoadService* Get(content::BrowserContext* context);

 private:
  // content::NotificationObserver.
  void Observe(int type,
               const content::NotificationSource& source,
               const content::NotificationDetails& details) override;

  // extensions::ExtensionRegistryObserver.
  void OnExtensionUnloaded(content::BrowserContext* browser_context,
                           const extensions::Extension* extension,
                           extensions::UnloadedExtensionReason reason) override;

  bool WasUnloadedForReload(const extensions::ExtensionId& extension_id,
                            const extensions::UnloadedExtensionReason reason);
  bool HasPostReloadAction(const std::string& extension_id);

  // Map of extension id to reload action. Absence from the map implies
  // no action.
  std::map<std::string, PostReloadAction> post_reload_actions_;
  content::NotificationRegistrar registrar_;
  content::BrowserContext* context_;

  DISALLOW_COPY_AND_ASSIGN(AppLoadService);
};

}  // namespace apps

#endif  // CHROME_BROWSER_APPS_APP_LOAD_SERVICE_H_
