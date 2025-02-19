// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SYNC_TEST_INTEGRATION_SESSIONS_HELPER_H_
#define CHROME_BROWSER_SYNC_TEST_INTEGRATION_SESSIONS_HELPER_H_

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "chrome/browser/sync/test/integration/multi_client_status_change_checker.h"
#include "chrome/browser/sync/test/integration/sync_test.h"
#include "components/sessions/core/session_types.h"
#include "components/sync/syncable/nigori_util.h"
#include "components/sync_sessions/synced_session.h"

class GURL;

namespace sessions_helper {

using SyncedSessionVector = std::vector<const sync_sessions::SyncedSession*>;
using SessionWindowMap =
    std::map<SessionID::id_type, sync_sessions::SyncedSessionWindow*>;
using ScopedWindowMap =
    std::map<SessionID::id_type,
             std::unique_ptr<sync_sessions::SyncedSessionWindow>>;

// Copies the local session windows of profile at |index| to |local_windows|.
// Returns true if successful.
bool GetLocalWindows(int index, ScopedWindowMap* local_windows);

// Checks that window count and foreign session count are 0.
bool CheckInitialState(int index);

// Returns number of open windows for a profile.
int GetNumWindows(int index);

// Returns number of foreign sessions for a profile.
int GetNumForeignSessions(int index);

// Fills the sessions vector with the SyncableService's foreign session data.
// Caller owns |sessions|, but not SyncedSessions objects within.
// Returns true if foreign sessions were found, false otherwise.
bool GetSessionData(int index, SyncedSessionVector* sessions);

// Compares a foreign session based on the first session window.
// Returns true based on the comparison of the session windows.
bool CompareSyncedSessions(const sync_sessions::SyncedSession* lhs,
                           const sync_sessions::SyncedSession* rhs);

// Sort a SyncedSession vector using our custom SyncedSession comparator.
void SortSyncedSessions(SyncedSessionVector* sessions);

// Compares two tab navigations base on the parameters we sync.
// (Namely, we don't sync state or type mask)
bool NavigationEquals(const sessions::SerializedNavigationEntry& expected,
                      const sessions::SerializedNavigationEntry& actual);

// Verifies that two SessionWindows match.
// Returns:
//  - true if all the following match:
//    1. number of SessionWindows,
//    2. number of tabs per SessionWindow,
//    3. number of tab navigations per tab,
//    4. actual tab navigations contents
// - false otherwise.
bool WindowsMatch(const ScopedWindowMap& win1, const ScopedWindowMap& win2);
bool WindowsMatch(const SessionWindowMap& win1, const ScopedWindowMap& win2);

// Retrieves the foreign sessions for a particular profile and compares them
// with a reference SessionWindow list.
// Returns true if the session windows of the foreign session matches the
// reference.
bool CheckForeignSessionsAgainst(int index,
                                 const std::vector<ScopedWindowMap>& windows);

// Open a single tab  in the browser at |index| and block until the
// session SyncableService is aware of it. Returns true upon success, false
// otherwise.
bool OpenTab(int index, const GURL& url);

// See OpenTab, except that the tab is opened in position |tab_index|.
// If |tab_index| is -1 or greater than the number of tabs, the tab will be
// appended to the end of the strip. i.e. if tab_index is 3 for a tab strip of
// size 1, the new tab will be in position 1.
bool OpenTabAtIndex(int index, int tab_index, const GURL& url);

// Like OpenTab, but opens |url| from the tab at |index_of_source_tab| using
// |disposition|.
bool OpenTabFromSourceIndex(int index,
                            int index_of_source_tab,
                            const GURL& url,
                            WindowOpenDisposition disposition);

// Open multiple tabs and block until the session SyncableService is aware
// of all of them.  Returns true on success, false on failure.
bool OpenMultipleTabs(int index, const std::vector<GURL>& urls);

// Moves the tab in position |tab_index| in the TabStrip for browser at
// |from_index| to the TabStrip for browser at |to_index|.
void MoveTab(int from_index, int to_index, int tab_index);

// Navigate the active tab for browser in position |index| to the given
// url, and blocks until the session SyncableService is aware of it.
// WARNING: it's dangerous to assume this will return for any arbitrary URL.
// For URLs that don't resolve to a valid server response, this can block
// indefinitely. Use a data uri or the embedded_test_server to ensure that this
// doesn't happen.
bool NavigateTab(int index, const GURL& url);

// Navigate the active tab for browser in position |index| back by one;
// if this isn't possible, does nothing
void NavigateTabBack(int index);

// Navigate the active tab for browser in position |index| forward by
// one; if this isn't possible, does nothing
void NavigateTabForward(int index);

// Wait for a session change to |web_contents| to propagate to the model
// associator. Will return true once |url| has been found, or false if it times
// out while waiting.
bool WaitForTabToLoad(int index,
                      const GURL& url,
                      content::WebContents* web_contents);

// Wait for each url in |urls| to load. The ordering of |urls| is assumed to
// match the ordering of the corresponding tabs.
bool WaitForTabsToLoad(int index, const std::vector<GURL>& urls);

// Check if the session SyncableService knows that the current open tab
// has this url.
bool SessionsSyncManagerHasTabWithURL(int index, const GURL& url);

// Stores a pointer to the local session for a given profile in |session|.
// Returns true on success, false on failure.
bool GetLocalSession(int index, const sync_sessions::SyncedSession** session);

// Deletes the foreign session with tag |session_tag| from the profile specified
// by |index|. This will affect all synced clients.
// Note: We pass the session_tag in by value to ensure it's not a reference
// to the session tag within the SyncedSession we plan to delete.
void DeleteForeignSession(int index, std::string session_tag);

}  // namespace sessions_helper

// Checker to block until the foreign sessions for a particular profile matches
// the reference windows.
class ForeignSessionsMatchChecker : public MultiClientStatusChangeChecker {
 public:
  ForeignSessionsMatchChecker(
      int index,
      const std::vector<sessions_helper::ScopedWindowMap>& windows);

  // StatusChangeChecker implementation.
  bool IsExitConditionSatisfied() override;
  std::string GetDebugMessage() const override;

 private:
  int index_;
  const std::vector<sessions_helper::ScopedWindowMap>& windows_;
};

#endif  // CHROME_BROWSER_SYNC_TEST_INTEGRATION_SESSIONS_HELPER_H_
