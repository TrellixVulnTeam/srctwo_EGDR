// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/chrome/browser/reading_list/offline_url_utils.h"

#include <string>

#include "base/files/file_path.h"
#include "base/memory/ptr_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/test/gtest_util.h"
#include "base/time/default_clock.h"
#include "components/reading_list/core/reading_list_entry.h"
#include "components/reading_list/core/reading_list_model_impl.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

// Checks the distilled URL for the page with an onlineURL is
// chrome://offline/MD5/page.html?entryURL=...&virtualURL=...
TEST(OfflineURLUtilsTest, OfflineURLForPathWithEntryURLAndVirtualURLTest) {
  base::FilePath page_path("MD5/page.html");
  GURL entry_url = GURL("http://foo.bar");
  GURL virtual_url = GURL("http://foo.bar/virtual");
  GURL distilled_url =
      reading_list::OfflineURLForPath(page_path, entry_url, virtual_url);
  EXPECT_EQ(
      "chrome://offline/MD5/page.html?"
      "entryURL=http%3A%2F%2Ffoo.bar%2F&"
      "virtualURL=http%3A%2F%2Ffoo.bar%2Fvirtual",
      distilled_url.spec());
}

// Checks the parsing of offline URL chrome://offline/MD5/page.html.
// As entryURL and virtualURL are absent, they should be invalid.
TEST(OfflineURLUtilsTest, ParseOfflineURLTest) {
  GURL distilled_url("chrome://offline/MD5/page.html");
  GURL entry_url = reading_list::EntryURLForOfflineURL(distilled_url);
  EXPECT_TRUE(entry_url.is_empty());
  GURL virtual_url = reading_list::VirtualURLForOfflineURL(distilled_url);
  EXPECT_TRUE(virtual_url.is_empty());
}

// Checks the parsing of offline URL
// chrome://offline/MD5/page.html?entryURL=encorded%20URL
// As entryURL is present, it should be returned correctly.
// As virtualURL is absent, it should return GURL::EmptyGURL().
TEST(OfflineURLUtilsTest, ParseOfflineURLWithEntryURLTest) {
  GURL offline_url(
      "chrome://offline/MD5/page.html?entryURL=http%3A%2F%2Ffoo.bar%2F");
  GURL entry_url = reading_list::EntryURLForOfflineURL(offline_url);
  EXPECT_EQ("http://foo.bar/", entry_url.spec());
  GURL virtual_url = reading_list::VirtualURLForOfflineURL(offline_url);
  EXPECT_TRUE(virtual_url.is_empty());
}

// Checks the parsing of offline URL
// chrome://offline/MD5/page.html?virtualURL=encorded%20URL
// As entryURL is absent, it should return the offline URL.
// As virtualURL is present, it should be returned correctly.
TEST(OfflineURLUtilsTest, ParseOfflineURLWithVirtualURLTest) {
  GURL offline_url(
      "chrome://offline/MD5/page.html?virtualURL=http%3A%2F%2Ffoo.bar%2F");
  GURL entry_url = reading_list::EntryURLForOfflineURL(offline_url);
  EXPECT_TRUE(entry_url.is_empty());
  GURL virtual_url = reading_list::VirtualURLForOfflineURL(offline_url);
  EXPECT_EQ("http://foo.bar/", virtual_url.spec());
}

// Checks the parsing of offline URL
// chrome://offline/MD5/page.html?entryURL=...&virtualURL=...
// As entryURL is present, it should be returned correctly.
// As virtualURL is present, it should be returned correctly.
TEST(OfflineURLUtilsTest, ParseOfflineURLWithVirtualAndEntryURLTest) {
  GURL offline_url(
      "chrome://offline/MD5/"
      "page.html?virtualURL=http%3A%2F%2Ffoo.bar%2Fvirtual&entryURL=http%3A%2F%"
      "2Ffoo.bar%2Fentry");
  GURL entry_url = reading_list::EntryURLForOfflineURL(offline_url);
  EXPECT_EQ("http://foo.bar/entry", entry_url.spec());
  GURL virtual_url = reading_list::VirtualURLForOfflineURL(offline_url);
  EXPECT_EQ("http://foo.bar/virtual", virtual_url.spec());
}

// Checks the file path for chrome://offline/MD5/page.html is
// file://profile_path/Offline/MD5/page.html.
// Checks the resource root for chrome://offline/MD5/page.html is
// file://profile_path/Offline/MD5
TEST(OfflineURLUtilsTest, FileURLForDistilledURLTest) {
  base::FilePath offline_path("/profile_path/Offline");
  GURL file_url =
      reading_list::FileURLForDistilledURL(GURL(), offline_path, nullptr);
  EXPECT_FALSE(file_url.is_valid());

  GURL distilled_url("chrome://offline/MD5/page.html");
  file_url = reading_list::FileURLForDistilledURL(distilled_url, offline_path,
                                                  nullptr);
  EXPECT_TRUE(file_url.is_valid());
  EXPECT_TRUE(file_url.SchemeIsFile());
  EXPECT_EQ("/profile_path/Offline/MD5/page.html", file_url.path());

  GURL resource_url;
  file_url = reading_list::FileURLForDistilledURL(distilled_url, offline_path,
                                                  &resource_url);
  EXPECT_TRUE(resource_url.is_valid());
  EXPECT_TRUE(resource_url.SchemeIsFile());
  EXPECT_EQ("/profile_path/Offline/MD5/", resource_url.path());
}

// Checks that the offline URLs are correctly detected by |IsOfflineURL|.
TEST(OfflineURLUtilsTest, IsOfflineURL) {
  EXPECT_FALSE(reading_list::IsOfflineURL(GURL()));
  EXPECT_FALSE(reading_list::IsOfflineURL(GURL("chrome://")));
  EXPECT_FALSE(reading_list::IsOfflineURL(GURL("chrome://offline-foobar")));
  EXPECT_FALSE(reading_list::IsOfflineURL(GURL("http://offline/")));
  EXPECT_FALSE(reading_list::IsOfflineURL(GURL("http://chrome://offline/")));
  EXPECT_TRUE(reading_list::IsOfflineURL(GURL("chrome://offline")));
  EXPECT_TRUE(reading_list::IsOfflineURL(GURL("chrome://offline/")));
  EXPECT_TRUE(reading_list::IsOfflineURL(GURL("chrome://offline/foobar")));
  EXPECT_TRUE(
      reading_list::IsOfflineURL(GURL("chrome://offline/foobar?foo=bar")));
}

// Checks that the offline URLs are correctly detected by |IsOfflineURL|.
TEST(OfflineURLUtilsTest, IsOfflineURLValid) {
  auto reading_list_model = base::MakeUnique<ReadingListModelImpl>(
      nullptr, nullptr, base::MakeUnique<base::DefaultClock>());
  GURL entry_url("http://entry_url.com");
  base::FilePath distilled_path("distilled/page.html");
  GURL distilled_url("http://distilled_url.com");
  reading_list_model->AddEntry(entry_url, "title",
                               reading_list::ADDED_VIA_CURRENT_APP);
  reading_list_model->SetEntryDistilledInfo(
      entry_url, distilled_path, distilled_url, 10, base::Time::Now());

  EXPECT_FALSE(
      reading_list::IsOfflineURLValid(GURL(), reading_list_model.get()));
  EXPECT_FALSE(reading_list::IsOfflineURLValid(GURL("chrome://"),
                                               reading_list_model.get()));
  EXPECT_FALSE(reading_list::IsOfflineURLValid(GURL("chrome://offline-foobar"),
                                               reading_list_model.get()));
  EXPECT_FALSE(reading_list::IsOfflineURLValid(GURL("http://offline/"),
                                               reading_list_model.get()));
  EXPECT_FALSE(reading_list::IsOfflineURLValid(GURL("http://chrome://offline/"),
                                               reading_list_model.get()));
  EXPECT_FALSE(reading_list::IsOfflineURLValid(GURL("chrome://offline"),
                                               reading_list_model.get()));
  EXPECT_FALSE(reading_list::IsOfflineURLValid(GURL("chrome://offline/"),
                                               reading_list_model.get()));
  EXPECT_FALSE(reading_list::IsOfflineURLValid(GURL("chrome://offline/foobar"),
                                               reading_list_model.get()));
  EXPECT_FALSE(reading_list::IsOfflineURLValid(
      GURL("chrome://offline/foobar?foo=bar"), reading_list_model.get()));
  EXPECT_TRUE(reading_list::IsOfflineURLValid(
      reading_list::OfflineURLForPath(distilled_path, entry_url, distilled_url),
      reading_list_model.get()));
  EXPECT_FALSE(reading_list::IsOfflineURLValid(
      reading_list::OfflineURLForPath(distilled_path, entry_url, entry_url),
      reading_list_model.get()));
  EXPECT_FALSE(reading_list::IsOfflineURLValid(
      reading_list::OfflineURLForPath(base::FilePath("not_distilled_path"),
                                      entry_url, distilled_url),
      reading_list_model.get()));
  reading_list_model->RemoveEntryByURL(entry_url);
  EXPECT_FALSE(reading_list::IsOfflineURLValid(
      reading_list::OfflineURLForPath(distilled_path, entry_url, distilled_url),
      reading_list_model.get()));
}
