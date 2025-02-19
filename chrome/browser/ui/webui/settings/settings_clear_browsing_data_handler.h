// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_SETTINGS_SETTINGS_CLEAR_BROWSING_DATA_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_SETTINGS_SETTINGS_CLEAR_BROWSING_DATA_HANDLER_H_

#include <memory>
#include <string>
#include <vector>

#include "base/containers/flat_set.h"
#include "base/macros.h"
#include "base/scoped_observer.h"
#include "chrome/browser/engagement/important_sites_util.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/webui/settings/settings_page_ui_handler.h"
#include "components/browser_sync/profile_sync_service.h"
#include "components/browsing_data/core/browsing_data_utils.h"
#include "components/browsing_data/core/counters/browsing_data_counter.h"

namespace base {
class ListValue;
}

namespace content {
class BrowsingDataFilterBuilder;
class WebUI;
}

namespace settings {

// Chrome browser startup settings handler.
class ClearBrowsingDataHandler : public SettingsPageUIHandler,
                                 public syncer::SyncServiceObserver {
 public:
  explicit ClearBrowsingDataHandler(content::WebUI* webui);
  ~ClearBrowsingDataHandler() override;

  // WebUIMessageHandler implementation.
  void RegisterMessages() override;
  void OnJavascriptAllowed() override;
  void OnJavascriptDisallowed() override;

 private:
  // Clears browsing data, called by Javascript.
  void HandleClearBrowsingData(const base::ListValue* value);

  // Parses a ListValue with important site information and creates a
  // BrowsingDataFilterBuilder.
  std::unique_ptr<content::BrowsingDataFilterBuilder> ProcessImportantSites(
      const base::ListValue* important_sites);

  // Called when a clearing task finished. |webui_callback_id| is provided
  // by the WebUI action that initiated it.
  void OnClearingTaskFinished(
      const std::string& webui_callback_id,
      const base::flat_set<browsing_data::BrowsingDataType>& data_types);

  // Get important sites, called by Javascript.
  void HandleGetImportantSites(const base::ListValue* value);

  void OnFetchImportantSitesFinished(
      const std::string& callback_id,
      std::vector<ImportantSitesUtil::ImportantDomainInfo> sites);

  // Initializes the dialog UI. Called by JavaScript when the DOM is ready.
  void HandleInitialize(const base::ListValue* args);

  // Implementation of SyncServiceObserver.
  void OnStateChanged(syncer::SyncService* sync) override;

  // Updates the footer of the dialog when the sync state changes.
  void UpdateSyncState();

  // Finds out whether we should show notice about other forms of history stored
  // in user's account.
  void RefreshHistoryNotice();

  // Called as an asynchronous response to |RefreshHistoryNotice()|. Shows or
  // hides the footer about other forms of history stored in user's account.
  void UpdateHistoryNotice(bool show);

  // Called as an asynchronous response to |RefreshHistoryNotice()|. Enables or
  // disables the dialog about other forms of history stored in user's account
  // that is shown when the history deletion is finished.
  void UpdateHistoryDeletionDialog(bool show);

  // Adds a browsing data |counter|.
  void AddCounter(std::unique_ptr<browsing_data::BrowsingDataCounter> counter);

  // Updates a counter text according to the |result|.
  void UpdateCounterText(
      std::unique_ptr<browsing_data::BrowsingDataCounter::Result> result);

  // Cached profile corresponding to the WebUI of this handler.
  Profile* profile_;

  // Counters that calculate the data volume for individual data types.
  std::vector<std::unique_ptr<browsing_data::BrowsingDataCounter>> counters_;

  // ProfileSyncService to observe sync state changes.
  browser_sync::ProfileSyncService* sync_service_;
  ScopedObserver<browser_sync::ProfileSyncService, syncer::SyncServiceObserver>
      sync_service_observer_;

  // Whether the sentence about other forms of history stored in user's account
  // should be displayed in the footer. This value is retrieved asynchronously,
  // so we cache it here.
  bool show_history_footer_;

  // Whether we should show a dialog informing the user about other forms of
  // history stored in their account after the history deletion is finished.
  bool show_history_deletion_dialog_;

  // A weak pointer factory for asynchronous calls referencing this class.
  // The weak pointers are invalidated in |OnJavascriptDisallowed()| and
  // |HandleInitialize()| to cancel previously initiated tasks.
  base::WeakPtrFactory<ClearBrowsingDataHandler> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(ClearBrowsingDataHandler);
};

}  // namespace settings

#endif  // CHROME_BROWSER_UI_WEBUI_SETTINGS_SETTINGS_CLEAR_BROWSING_DATA_HANDLER_H_
