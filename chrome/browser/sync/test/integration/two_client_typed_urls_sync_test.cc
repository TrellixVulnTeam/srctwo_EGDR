// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>

#include "base/guid.h"
#include "base/i18n/number_formatting.h"
#include "base/macros.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "build/build_config.h"
#include "chrome/browser/sessions/session_service.h"
#include "chrome/browser/sync/test/integration/bookmarks_helper.h"
#include "chrome/browser/sync/test/integration/profile_sync_service_harness.h"
#include "chrome/browser/sync/test/integration/sync_test.h"
#include "chrome/browser/sync/test/integration/typed_urls_helper.h"
#include "chrome/browser/sync/test/integration/updated_progress_marker_checker.h"
#include "components/history/core/browser/history_types.h"

using base::ASCIIToUTF16;
using bookmarks::BookmarkNode;
using typed_urls_helper::AddUrlToHistory;
using typed_urls_helper::AddUrlToHistoryWithTimestamp;
using typed_urls_helper::AddUrlToHistoryWithTransition;
using typed_urls_helper::AreVisitsEqual;
using typed_urls_helper::AreVisitsUnique;
using typed_urls_helper::CheckURLRowVectorsAreEqual;
using typed_urls_helper::CheckSyncDirectoryHasURL;
using typed_urls_helper::DeleteUrlFromHistory;
using typed_urls_helper::GetTypedUrlsFromClient;
using typed_urls_helper::GetUrlFromClient;
using typed_urls_helper::GetVisitsFromClient;
using typed_urls_helper::RemoveVisitsFromClient;

class TwoClientTypedUrlsSyncTest : public SyncTest {
 public:
  TwoClientTypedUrlsSyncTest() : SyncTest(TWO_CLIENT) {}
  ~TwoClientTypedUrlsSyncTest() override {}

  ::testing::AssertionResult CheckClientsEqual() {
    history::URLRows urls = GetTypedUrlsFromClient(0);
    history::URLRows urls2 = GetTypedUrlsFromClient(1);
    if (!CheckURLRowVectorsAreEqual(urls, urls2))
      return ::testing::AssertionFailure() << "URLVectors are not equal";
    // Now check the visits.
    for (size_t i = 0; i < urls.size() && i < urls2.size(); i++) {
      history::VisitVector visit1 = GetVisitsFromClient(0, urls[i].id());
      history::VisitVector visit2 = GetVisitsFromClient(1, urls2[i].id());
      if (!AreVisitsEqual(visit1, visit2))
        return ::testing::AssertionFailure() << "Visits are not equal";
    }
    return ::testing::AssertionSuccess();
  }

  bool CheckNoDuplicateVisits() {
    for (int i = 0; i < num_clients(); ++i) {
      history::URLRows urls = GetTypedUrlsFromClient(i);
      for (size_t j = 0; j < urls.size(); ++j) {
        history::VisitVector visits = GetVisitsFromClient(i, urls[j].id());
        if (!AreVisitsUnique(visits))
          return false;
      }
    }
    return true;
  }

  int GetVisitCountForFirstURL(int index) {
    history::URLRows urls = GetTypedUrlsFromClient(index);
    if (urls.size() == 0)
      return 0;
    else
      return urls[0].visit_count();
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(TwoClientTypedUrlsSyncTest);
};

IN_PROC_BROWSER_TEST_F(TwoClientTypedUrlsSyncTest, E2E_ENABLED(Add)) {
  // Use a randomized URL to prevent test collisions.
  const base::string16 kHistoryUrl = ASCIIToUTF16(base::StringPrintf(
      "http://www.add-history.google.com/%s", base::GenerateGUID().c_str()));
  ASSERT_TRUE(SetupSync()) << "SetupSync() failed.";

  size_t initial_count = GetTypedUrlsFromClient(0).size();

  // Populate one client with a URL, wait for it to sync to the other.
  GURL new_url(kHistoryUrl);
  AddUrlToHistory(0, new_url);
  ASSERT_TRUE(ProfilesHaveSameURLsChecker().Wait());

  // Assert that the second client has the correct new URL.
  history::URLRows urls = GetTypedUrlsFromClient(1);
  ASSERT_EQ(initial_count + 1, urls.size());
  ASSERT_EQ(new_url, urls.back().url());
}

IN_PROC_BROWSER_TEST_F(TwoClientTypedUrlsSyncTest, AddExpired) {
  const base::string16 kHistoryUrl(
      ASCIIToUTF16("http://www.add-one-history.google.com/"));
  ASSERT_TRUE(SetupSync()) << "SetupSync() failed.";

  // Populate one client with a URL, should sync to the other.
  GURL new_url(kHistoryUrl);
  // Create a URL with a timestamp 1 year before today.
  base::Time timestamp = base::Time::Now() - base::TimeDelta::FromDays(365);
  AddUrlToHistoryWithTimestamp(0,
                               new_url,
                               ui::PAGE_TRANSITION_TYPED,
                               history::SOURCE_BROWSED,
                               timestamp);
  history::URLRows urls = GetTypedUrlsFromClient(0);
  ASSERT_EQ(1U, urls.size());
  ASSERT_EQ(new_url, urls[0].url());

  // Let sync finish.
  ASSERT_TRUE(GetClient(0)->AwaitMutualSyncCycleCompletion(GetClient(1)));

  // Second client should still have no URLs since this one is expired.
  urls = GetTypedUrlsFromClient(1);
  ASSERT_EQ(0U, urls.size());

  // Sync should not receive expired visits.
  EXPECT_FALSE(CheckSyncDirectoryHasURL(0, new_url));
}

IN_PROC_BROWSER_TEST_F(TwoClientTypedUrlsSyncTest, AddExpiredThenUpdate) {
  const base::string16 kHistoryUrl(
      ASCIIToUTF16("http://www.add-one-history.google.com/"));
  ASSERT_TRUE(SetupSync()) << "SetupSync() failed.";

  // Populate one client with a URL, should sync to the other.
  GURL new_url(kHistoryUrl);
  // Create a URL with a timestamp 1 year before today.
  base::Time timestamp = base::Time::Now() - base::TimeDelta::FromDays(365);
  AddUrlToHistoryWithTimestamp(0,
                               new_url,
                               ui::PAGE_TRANSITION_TYPED,
                               history::SOURCE_BROWSED,
                               timestamp);
  std::vector<history::URLRow> urls = GetTypedUrlsFromClient(0);
  ASSERT_EQ(1U, urls.size());
  ASSERT_EQ(new_url, urls[0].url());

  // Let sync finish.
  ASSERT_TRUE(GetClient(0)->AwaitMutualSyncCycleCompletion(GetClient(1)));

  // Second client should still have no URLs since this one is expired.
  urls = GetTypedUrlsFromClient(1);
  ASSERT_EQ(0U, urls.size());

  // Sync should not receive expired visits.
  EXPECT_FALSE(CheckSyncDirectoryHasURL(0, new_url));

  // Now drive an update on the first client.
  AddUrlToHistory(0, new_url);

  // Let sync finish again.
  ASSERT_TRUE(GetClient(0)->AwaitMutualSyncCycleCompletion(GetClient(1)));

  // Second client should have the URL now.
  urls = GetTypedUrlsFromClient(1);
  ASSERT_EQ(1U, urls.size());

  // Sync should receive the new visit.
  EXPECT_TRUE(CheckSyncDirectoryHasURL(0, new_url));
  EXPECT_TRUE(CheckSyncDirectoryHasURL(1, new_url));
}

IN_PROC_BROWSER_TEST_F(TwoClientTypedUrlsSyncTest, E2E_ENABLED(AddThenDelete)) {
  // Use a randomized URL to prevent test collisions.
  const base::string16 kHistoryUrl = ASCIIToUTF16(base::StringPrintf(
      "http://www.add-history.google.com/%s", base::GenerateGUID().c_str()));
  ASSERT_TRUE(SetupSync()) << "SetupSync() failed.";

  size_t initial_count = GetTypedUrlsFromClient(0).size();

  // Populate one client with a URL, wait for it to sync to the other.
  GURL new_url(kHistoryUrl);
  AddUrlToHistory(0, new_url);
  ASSERT_TRUE(ProfilesHaveSameURLsChecker().Wait());

  // Assert that the second client has the correct new URL.
  history::URLRows urls = GetTypedUrlsFromClient(1);
  ASSERT_EQ(initial_count + 1, urls.size());
  ASSERT_EQ(new_url, urls.back().url());

  // Delete from first client, and wait for them to sync.
  DeleteUrlFromHistory(0, new_url);
  ASSERT_TRUE(ProfilesHaveSameURLsChecker().Wait());

  // Assert that it's deleted from the second client.
  ASSERT_EQ(initial_count, GetTypedUrlsFromClient(1).size());
}

IN_PROC_BROWSER_TEST_F(TwoClientTypedUrlsSyncTest,
                       E2E_ENABLED(DisableEnableSync)) {
  const base::string16 kUrl1(ASCIIToUTF16("http://history1.google.com/"));
  const base::string16 kUrl2(ASCIIToUTF16("http://history2.google.com/"));
  ASSERT_TRUE(SetupSync()) << "SetupSync() failed.";

  // Disable typed url sync for one client, leave it active for the other.
  GetClient(0)->DisableSyncForDatatype(syncer::TYPED_URLS);

  // Add one URL to non-syncing client, add a different URL to the other,
  // wait for sync cycle to complete. No data should be exchanged.
  GURL url1(kUrl1);
  GURL url2(kUrl2);
  AddUrlToHistory(0, url1);
  AddUrlToHistory(1, url2);
  ASSERT_TRUE(UpdatedProgressMarkerChecker(GetSyncService(1)).Wait());

  // Make sure that no data was exchanged.
  history::URLRows post_sync_urls = GetTypedUrlsFromClient(0);
  ASSERT_EQ(1U, post_sync_urls.size());
  ASSERT_EQ(url1, post_sync_urls[0].url());
  post_sync_urls = GetTypedUrlsFromClient(1);
  ASSERT_EQ(1U, post_sync_urls.size());
  ASSERT_EQ(url2, post_sync_urls[0].url());

  // Enable typed url sync, make both URLs are synced to each client.
  GetClient(0)->EnableSyncForDatatype(syncer::TYPED_URLS);

  ASSERT_TRUE(ProfilesHaveSameURLsChecker().Wait());
}

// flaky, see crbug.com/108511
IN_PROC_BROWSER_TEST_F(TwoClientTypedUrlsSyncTest, DISABLED_AddOneDeleteOther) {
  const base::string16 kHistoryUrl(
      ASCIIToUTF16("http://www.add-one-delete-history.google.com/"));
  ASSERT_TRUE(SetupSync()) << "SetupSync() failed.";

  // Populate one client with a URL, should sync to the other.
  GURL new_url(kHistoryUrl);
  AddUrlToHistory(0, new_url);
  history::URLRows urls = GetTypedUrlsFromClient(0);
  ASSERT_EQ(1U, urls.size());
  ASSERT_EQ(new_url, urls[0].url());

  // Both clients should have this URL.
  ASSERT_TRUE(ProfilesHaveSameURLsChecker().Wait());

  // Now, delete the URL from the second client.
  DeleteUrlFromHistory(1, new_url);
  urls = GetTypedUrlsFromClient(0);
  ASSERT_EQ(1U, urls.size());

  // Both clients should have this URL removed.
  ASSERT_TRUE(ProfilesHaveSameURLsChecker().Wait());
}

// flaky, see crbug.com/108511
IN_PROC_BROWSER_TEST_F(TwoClientTypedUrlsSyncTest,
                       DISABLED_AddOneDeleteOtherAddAgain) {
  const base::string16 kHistoryUrl(
      ASCIIToUTF16("http://www.add-delete-add-history.google.com/"));
  ASSERT_TRUE(SetupSync()) << "SetupSync() failed.";

  // Populate one client with a URL, should sync to the other.
  GURL new_url(kHistoryUrl);
  AddUrlToHistory(0, new_url);
  history::URLRows urls = GetTypedUrlsFromClient(0);
  ASSERT_EQ(1U, urls.size());
  ASSERT_EQ(new_url, urls[0].url());

  // Both clients should have this URL.
  ASSERT_TRUE(ProfilesHaveSameURLsChecker().Wait());

  // Now, delete the URL from the second client.
  DeleteUrlFromHistory(1, new_url);
  urls = GetTypedUrlsFromClient(0);
  ASSERT_EQ(1U, urls.size());

  // Both clients should have this URL removed.
  ASSERT_TRUE(ProfilesHaveSameURLsChecker().Wait());

  // Add it to the first client again, should succeed (tests that the deletion
  // properly disassociates that URL).
  AddUrlToHistory(0, new_url);

  // Both clients should have this URL added again.
  ASSERT_TRUE(ProfilesHaveSameURLsChecker().Wait());
}

IN_PROC_BROWSER_TEST_F(TwoClientTypedUrlsSyncTest,
                       MergeTypedWithNonTypedDuringAssociation) {
  ASSERT_TRUE(SetupClients());
  GURL new_url("http://history.com");
  base::Time timestamp = base::Time::Now();
  // Put a non-typed URL in both clients with an identical timestamp.
  // Then add a typed URL to the second client - this test makes sure that
  // we properly merge both sets of visits together to end up with the same
  // set of visits on both ends.
  AddUrlToHistoryWithTimestamp(0, new_url, ui::PAGE_TRANSITION_LINK,
                               history::SOURCE_BROWSED, timestamp);
  AddUrlToHistoryWithTimestamp(1, new_url, ui::PAGE_TRANSITION_LINK,
                               history::SOURCE_BROWSED, timestamp);
  AddUrlToHistoryWithTimestamp(1, new_url, ui::PAGE_TRANSITION_TYPED,
                               history::SOURCE_BROWSED,
                               timestamp + base::TimeDelta::FromSeconds(1));

  // Now start up sync - URLs should get merged. Fully sync client 1 first,
  // before syncing client 0, so we have both of client 1's URLs in the sync DB
  // at the time that client 0 does model association.
  ASSERT_TRUE(GetClient(1)->SetupSync()) << "SetupSync() failed";
  ASSERT_TRUE(UpdatedProgressMarkerChecker(GetSyncService(1)).Wait());
  ASSERT_TRUE(GetClient(0)->SetupSync()) << "SetupSync() failed";
  ASSERT_TRUE(GetClient(0)->AwaitMutualSyncCycleCompletion(GetClient(1)));

  ASSERT_TRUE(CheckClientsEqual());
  // At this point, we should have no duplicates (total visit count should be
  // 2). We only need to check client 0 since we already verified that both
  // clients are identical above.
  history::URLRows urls = GetTypedUrlsFromClient(0);
  ASSERT_EQ(1U, urls.size());
  ASSERT_EQ(new_url, urls[0].url());
  ASSERT_TRUE(CheckNoDuplicateVisits());
  ASSERT_EQ(2, GetVisitCountForFirstURL(0));
}

// Tests transitioning a URL from non-typed to typed when both clients
// have already seen that URL (so a merge is required).
IN_PROC_BROWSER_TEST_F(TwoClientTypedUrlsSyncTest,
                       MergeTypedWithNonTypedDuringChangeProcessing) {
  ASSERT_TRUE(SetupClients());
  GURL new_url("http://history.com");
  base::Time timestamp = base::Time::Now();
  // Setup both clients with the identical typed URL visit. This means we can't
  // use the verifier in this test, because this will show up as two distinct
  // visits in the verifier.
  AddUrlToHistoryWithTimestamp(0, new_url, ui::PAGE_TRANSITION_LINK,
                               history::SOURCE_BROWSED, timestamp);
  AddUrlToHistoryWithTimestamp(1, new_url, ui::PAGE_TRANSITION_LINK,
                               history::SOURCE_BROWSED, timestamp);

  // Now start up sync. Neither URL should get synced as they do not look like
  // typed URLs.
  ASSERT_TRUE(SetupSync()) << "SetupSync() failed.";
  ASSERT_TRUE(GetClient(0)->AwaitMutualSyncCycleCompletion(GetClient(1)));
  ASSERT_TRUE(CheckClientsEqual());
  history::URLRows urls = GetTypedUrlsFromClient(0);
  ASSERT_EQ(0U, urls.size());

  // Now, add a typed visit to the first client.
  AddUrlToHistoryWithTimestamp(0, new_url, ui::PAGE_TRANSITION_TYPED,
                               history::SOURCE_BROWSED,
                               timestamp + base::TimeDelta::FromSeconds(1));

  ASSERT_TRUE(GetClient(0)->AwaitMutualSyncCycleCompletion(GetClient(1)));
  ASSERT_TRUE(CheckClientsEqual());
  ASSERT_TRUE(CheckNoDuplicateVisits());
  urls = GetTypedUrlsFromClient(0);
  ASSERT_EQ(1U, urls.size());
  ASSERT_EQ(2, GetVisitCountForFirstURL(0));
  ASSERT_EQ(2, GetVisitCountForFirstURL(1));
}

// Tests transitioning a URL from non-typed to typed when one of the clients
// has never seen that URL before (so no merge is necessary).
IN_PROC_BROWSER_TEST_F(TwoClientTypedUrlsSyncTest, UpdateToNonTypedURL) {
  const base::string16 kHistoryUrl(
      ASCIIToUTF16("http://www.add-delete-add-history.google.com/"));
  ASSERT_TRUE(SetupSync()) << "SetupSync() failed.";

  // Populate one client with a non-typed URL, should not be synced.
  GURL new_url(kHistoryUrl);
  AddUrlToHistoryWithTransition(0, new_url, ui::PAGE_TRANSITION_LINK,
                                history::SOURCE_BROWSED);
  history::URLRows urls = GetTypedUrlsFromClient(0);
  ASSERT_EQ(0U, urls.size());

  // Both clients should have 0 typed URLs.
  ASSERT_TRUE(ProfilesHaveSameURLsChecker().Wait());
  urls = GetTypedUrlsFromClient(0);
  ASSERT_EQ(0U, urls.size());

  // Now, add a typed visit to this URL.
  AddUrlToHistory(0, new_url);

  // Let sync finish.
  ASSERT_TRUE(GetClient(0)->AwaitMutualSyncCycleCompletion(GetClient(1)));

  // Both clients should have this URL as typed and have two visits synced up.
  ASSERT_TRUE(CheckClientsEqual());
  urls = GetTypedUrlsFromClient(0);
  ASSERT_EQ(1U, urls.size());
  ASSERT_EQ(new_url, urls[0].url());
  ASSERT_EQ(2, GetVisitCountForFirstURL(0));
}

IN_PROC_BROWSER_TEST_F(TwoClientTypedUrlsSyncTest,
                       E2E_ENABLED(DontSyncUpdatedNonTypedURLs)) {
  // Checks if a non-typed URL that has been updated (modified) doesn't get
  // synced. This is a regression test after fixing a bug where adding a
  // non-typed URL was guarded against but later modifying it was not. Since
  // "update" is "update or create if missing", non-typed URLs were being
  // created.
  const GURL kNonTypedURL("http://link.google.com/");
  const GURL kTypedURL("http://typed.google.com/");
  ASSERT_TRUE(SetupSync()) << "SetupSync() failed.";
  AddUrlToHistoryWithTransition(0, kNonTypedURL, ui::PAGE_TRANSITION_LINK,
                                history::SOURCE_BROWSED);
  AddUrlToHistoryWithTransition(0, kTypedURL, ui::PAGE_TRANSITION_TYPED,
                                history::SOURCE_BROWSED);

  // Modify the non-typed URL. It should not get synced.
  typed_urls_helper::SetPageTitle(0, kNonTypedURL, "Welcome to Non-Typed URL");
  ASSERT_TRUE(ProfilesHaveSameURLsChecker().Wait());

  history::VisitVector visits;
  // First client has both visits.
  visits = typed_urls_helper::GetVisitsForURLFromClient(0, kNonTypedURL);
  ASSERT_EQ(1U, visits.size());
  EXPECT_TRUE(ui::PageTransitionCoreTypeIs(visits[0].transition,
                                           ui::PAGE_TRANSITION_LINK));
  visits = typed_urls_helper::GetVisitsForURLFromClient(0, kTypedURL);
  ASSERT_EQ(1U, visits.size());
  EXPECT_TRUE(ui::PageTransitionCoreTypeIs(visits[0].transition,
                                           ui::PAGE_TRANSITION_TYPED));
  // Second client has only the typed visit.
  visits = typed_urls_helper::GetVisitsForURLFromClient(1, kNonTypedURL);
  ASSERT_EQ(0U, visits.size());
  visits = typed_urls_helper::GetVisitsForURLFromClient(1, kTypedURL);
  ASSERT_EQ(1U, visits.size());
  EXPECT_TRUE(ui::PageTransitionCoreTypeIs(visits[0].transition,
                                           ui::PAGE_TRANSITION_TYPED));
}

IN_PROC_BROWSER_TEST_F(TwoClientTypedUrlsSyncTest,
                       E2E_ENABLED(SyncTypedRedirects)) {
  const base::string16 kHistoryUrl(ASCIIToUTF16("http://typed.google.com/"));
  const base::string16 kRedirectedHistoryUrl(
      ASCIIToUTF16("http://www.typed.google.com/"));
  ASSERT_TRUE(SetupSync()) << "SetupSync() failed.";

  // Simulate a typed address that gets redirected by the server to a different
  // address.
  GURL initial_url(kHistoryUrl);
  const ui::PageTransition initial_transition = ui::PageTransitionFromInt(
      ui::PAGE_TRANSITION_TYPED | ui::PAGE_TRANSITION_CHAIN_START);
  AddUrlToHistoryWithTransition(0, initial_url, initial_transition,
                                history::SOURCE_BROWSED);

  GURL redirected_url(kRedirectedHistoryUrl);
  const ui::PageTransition redirected_transition = ui::PageTransitionFromInt(
      ui::PAGE_TRANSITION_TYPED | ui::PAGE_TRANSITION_CHAIN_END |
      ui::PAGE_TRANSITION_SERVER_REDIRECT);
  // This address will have a typed_count == 0 because it's a redirection.
  // It should still be synced.
  AddUrlToHistoryWithTransition(0, redirected_url, redirected_transition,
                                history::SOURCE_BROWSED);

  // Both clients should have both URLs.
  ASSERT_TRUE(ProfilesHaveSameURLsChecker().Wait());

  history::VisitVector visits =
      typed_urls_helper::GetVisitsForURLFromClient(0, initial_url);
  ASSERT_EQ(1U, visits.size());
  EXPECT_TRUE(ui::PageTransitionCoreTypeIs(visits[0].transition,
                                           ui::PAGE_TRANSITION_TYPED));
  visits = typed_urls_helper::GetVisitsForURLFromClient(0, redirected_url);
  ASSERT_EQ(1U, visits.size());
  EXPECT_TRUE(ui::PageTransitionCoreTypeIs(visits[0].transition,
                                           ui::PAGE_TRANSITION_TYPED));

  visits = typed_urls_helper::GetVisitsForURLFromClient(1, initial_url);
  ASSERT_EQ(1U, visits.size());
  EXPECT_TRUE(ui::PageTransitionCoreTypeIs(visits[0].transition,
                                           ui::PAGE_TRANSITION_TYPED));
  visits = typed_urls_helper::GetVisitsForURLFromClient(1, redirected_url);
  ASSERT_EQ(1U, visits.size());
  EXPECT_TRUE(ui::PageTransitionCoreTypeIs(visits[0].transition,
                                           ui::PAGE_TRANSITION_TYPED));
}

IN_PROC_BROWSER_TEST_F(TwoClientTypedUrlsSyncTest,
                       SkipImportedVisits) {
  GURL imported_url("http://imported_url.com");
  GURL browsed_url("http://browsed_url.com");
  GURL browsed_and_imported_url("http://browsed_and_imported_url.com");
  ASSERT_TRUE(SetupClients());

  // Create 3 items in our first client - 1 imported, one browsed, one with
  // both imported and browsed entries.
  AddUrlToHistoryWithTransition(0, imported_url,
                                ui::PAGE_TRANSITION_TYPED,
                                history::SOURCE_FIREFOX_IMPORTED);
  AddUrlToHistoryWithTransition(0, browsed_url,
                                ui::PAGE_TRANSITION_TYPED,
                                history::SOURCE_BROWSED);
  AddUrlToHistoryWithTransition(0, browsed_and_imported_url,
                                ui::PAGE_TRANSITION_TYPED,
                                history::SOURCE_FIREFOX_IMPORTED);

  ASSERT_TRUE(SetupSync()) << "SetupSync() failed.";
  ASSERT_TRUE(GetClient(0)->AwaitMutualSyncCycleCompletion(GetClient(1)));
  history::URLRows urls = GetTypedUrlsFromClient(1);
  ASSERT_EQ(1U, urls.size());
  ASSERT_EQ(browsed_url, urls[0].url());

  // Now browse to 3rd URL - this should cause it to be synced, even though it
  // was initially imported.
  AddUrlToHistoryWithTransition(0, browsed_and_imported_url,
                                ui::PAGE_TRANSITION_TYPED,
                                history::SOURCE_BROWSED);
  ASSERT_TRUE(GetClient(0)->AwaitMutualSyncCycleCompletion(GetClient(1)));
  urls = GetTypedUrlsFromClient(1);
  ASSERT_EQ(2U, urls.size());

  // Make sure the imported URL didn't make it over.
  for (size_t i = 0; i < urls.size(); ++i) {
    ASSERT_NE(imported_url, urls[i].url());
  }
}

IN_PROC_BROWSER_TEST_F(TwoClientTypedUrlsSyncTest, BookmarksWithTypedVisit) {
  GURL bookmark_url("http://www.bookmark.google.com/");
  GURL bookmark_icon_url("http://www.bookmark.google.com/favicon.ico");
  ASSERT_TRUE(SetupClients());
  // Create a bookmark.
  const BookmarkNode* node = bookmarks_helper::AddURL(
      0, bookmarks_helper::IndexedURLTitle(0), bookmark_url);
  bookmarks_helper::SetFavicon(0, node, bookmark_icon_url,
      bookmarks_helper::CreateFavicon(SK_ColorWHITE),
      bookmarks_helper::FROM_UI);
  ASSERT_TRUE(SetupSync()) << "SetupSync() failed.";

  ASSERT_TRUE(GetClient(0)->AwaitMutualSyncCycleCompletion(GetClient(1)));
  // A row in the DB for client 1 should have been created as a result of the
  // sync.
  history::URLRow row;
  ASSERT_TRUE(GetUrlFromClient(1, bookmark_url, &row));

  // Now, add a typed visit for client 0 to the bookmark URL and sync it over
  // - this should not cause a crash.
  AddUrlToHistory(0, bookmark_url);
  ASSERT_TRUE(GetClient(0)->AwaitMutualSyncCycleCompletion(GetClient(1)));

  ASSERT_TRUE(ProfilesHaveSameURLsChecker().Wait());
  history::URLRows urls = GetTypedUrlsFromClient(0);
  ASSERT_EQ(1U, urls.size());
  ASSERT_EQ(bookmark_url, urls[0].url());
  ASSERT_EQ(1, GetVisitCountForFirstURL(0));
}
