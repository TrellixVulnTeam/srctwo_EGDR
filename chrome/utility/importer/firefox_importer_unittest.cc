// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/files/file_path.h"
#include "base/files/scoped_temp_dir.h"
#include "base/path_service.h"
#include "base/strings/utf_string_conversions.h"
#include "build/build_config.h"
#include "chrome/common/chrome_paths.h"
#include "chrome/common/importer/imported_bookmark_entry.h"
#include "chrome/common/importer/importer_data_types.h"
#include "chrome/common/importer/importer_url_row.h"
#include "chrome/common/importer/mock_importer_bridge.h"
#include "chrome/utility/importer/firefox_importer.h"
#include "chrome/utility/importer/firefox_importer_unittest_utils.h"
#include "chrome/utility/importer/nss_decryptor.h"
#include "components/favicon_base/favicon_usage_data.h"
#include "sql/connection.h"
#include "testing/gtest/include/gtest/gtest.h"

// TODO(jschuh): Disabled on Win64 build. http://crbug.com/179688
#if defined(OS_WIN) && defined(ARCH_CPU_X86_64)
#define MAYBE_NSS(x) DISABLED_##x
#else
#define MAYBE_NSS(x) x
#endif

// The following test requires the use of the NSSDecryptor, on OSX this needs
// to run in a separate process, so we use a proxy object so we can share the
// same test between platforms.
TEST(FirefoxImporterTest, MAYBE_NSS(Firefox3NSS3Decryptor)) {
  base::FilePath nss_path;
  ASSERT_TRUE(PathService::Get(chrome::DIR_TEST_DATA, &nss_path));
#if defined(OS_MACOSX)
  nss_path = nss_path.AppendASCII("firefox3_nss_mac");
#else
  nss_path = nss_path.AppendASCII("firefox3_nss");
#endif  // !OS_MACOSX
  base::FilePath db_path;
  ASSERT_TRUE(PathService::Get(chrome::DIR_TEST_DATA, &db_path));
  db_path = db_path.AppendASCII("firefox3_profile");

  FFUnitTestDecryptorProxy decryptor_proxy;
  ASSERT_TRUE(decryptor_proxy.Setup(nss_path));

  ASSERT_TRUE(decryptor_proxy.DecryptorInit(nss_path, db_path));
  EXPECT_EQ(
      base::ASCIIToUTF16("hello"),
      decryptor_proxy.Decrypt("MDIEEPgAAAAAAAAAAAAAAAAAAAEwFAYIKoZIhvcNAwcECKa"
                              "jtRg4qFSHBAhv9luFkXgDJA=="));
  // Test UTF-16 encoding.
  EXPECT_EQ(
      base::WideToUTF16(L"\x4E2D"),
      decryptor_proxy.Decrypt("MDIEEPgAAAAAAAAAAAAAAAAAAAEwFAYIKoZIhvcNAwcECLW"
                              "qqiccfQHWBAie74hxnULxlw=="));

  // Test empty string edge case.
  EXPECT_EQ(base::string16(), decryptor_proxy.Decrypt(std::string()));

  // Test invalid base64.
  EXPECT_EQ(base::string16(), decryptor_proxy.Decrypt("Not! Valid! Base64!"));
}

// The following test verifies proper detection of authentication scheme in
// firefox's signons db. We insert two entries into moz_logins table. The first
// has httpRealm column filled with non-empty string, therefore resulting
// PasswordForm should have SCHEME_BASIC in scheme. The second entry has NULL
// httpRealm, so it should produce a SCHEME_HTML PasswordForm.
TEST(FirefoxImporterTest, MAYBE_NSS(FirefoxNSSDecryptorDeduceAuthScheme)) {
  base::ScopedTempDir temp_dir;
  ASSERT_TRUE(temp_dir.CreateUniqueTempDir());
  base::FilePath signons_path =
      temp_dir.GetPath().AppendASCII("signons.sqlite");
  sql::Connection db_conn;

  ASSERT_TRUE(db_conn.Open(signons_path));

  ASSERT_TRUE(db_conn.Execute(
      "CREATE TABLE moz_logins (id INTEGER PRIMARY KEY, hostname TEXT NOT "
      "NULL, httpRealm TEXT, formSubmitURL TEXT, usernameField TEXT NOT NULL,"
      "passwordField TEXT NOT NULL, encryptedUsername TEXT NOT NULL,"
      "encryptedPassword TEXT NOT NULL, guid TEXT, encType INTEGER,"
      "timeCreated INTEGER, timeLastUsed INTEGER, timePasswordChanged "
      "INTEGER, timesUsed INTEGER)"));

  ASSERT_TRUE(db_conn.Execute(
      "CREATE TABLE moz_disabledHosts (id INTEGER PRIMARY KEY, hostname TEXT "
      "UNIQUE ON CONFLICT REPLACE)"));

  ASSERT_TRUE(db_conn.Execute(
      "INSERT INTO 'moz_logins' VALUES(1,'http://server.com:1234',"
      "'http_realm',NULL,'','','','','{bfa37106-a4dc-0a47-abb4-dafd507bf2db}',"
      "1,1401883410959,1401883410959,1401883410959,1)"));

  ASSERT_TRUE(db_conn.Execute(
      "INSERT INTO 'moz_logins' VALUES(2,'http://server.com:1234',NULL,"
      "'http://test2.server.com:1234/action','username','password','','',"
      "'{71ad64fa-b5d4-cf4d-b390-2e4d56fe2aff}',1,1401883939239,"
      "1401883939239, 1401883939239,1)"));

  db_conn.Close();

  base::FilePath nss_path;
  ASSERT_TRUE(PathService::Get(chrome::DIR_TEST_DATA, &nss_path));
#if defined(OS_MACOSX)
  nss_path = nss_path.AppendASCII("firefox3_nss_mac");
#else
  nss_path = nss_path.AppendASCII("firefox3_nss");
#endif  // !OS_MACOSX
  base::FilePath db_path;
  ASSERT_TRUE(PathService::Get(chrome::DIR_TEST_DATA, &db_path));
  db_path = db_path.AppendASCII("firefox3_profile");

  FFUnitTestDecryptorProxy decryptor_proxy;
  ASSERT_TRUE(decryptor_proxy.Setup(nss_path));

  ASSERT_TRUE(decryptor_proxy.DecryptorInit(nss_path, db_path));
  std::vector<autofill::PasswordForm> forms =
      decryptor_proxy.ParseSignons(signons_path);

  ASSERT_EQ(2u, forms.size());
  EXPECT_EQ(autofill::PasswordForm::SCHEME_BASIC, forms[0].scheme);
  EXPECT_EQ(autofill::PasswordForm::SCHEME_HTML, forms[1].scheme);
}

TEST(FirefoxImporterTest, ImportBookmarks) {
  using ::testing::_;
  base::FilePath places_path;
  ASSERT_TRUE(PathService::Get(chrome::DIR_TEST_DATA, &places_path));
  places_path =
      places_path.AppendASCII("import").AppendASCII("firefox").AppendASCII(
          "48.0.2");
  scoped_refptr<FirefoxImporter> importer = new FirefoxImporter;
  importer::SourceProfile profile;
  profile.source_path = places_path;
  scoped_refptr<MockImporterBridge> bridge = new MockImporterBridge;
  std::vector<ImportedBookmarkEntry> bookmarks;
  favicon_base::FaviconUsageDataList favicons;
  EXPECT_CALL(*bridge, NotifyStarted());
  EXPECT_CALL(*bridge, NotifyItemStarted(importer::FAVORITES));
  EXPECT_CALL(*bridge, AddBookmarks(_, _))
      .WillOnce(::testing::SaveArg<0>(&bookmarks));
  EXPECT_CALL(*bridge, SetFavicons(_))
      .WillOnce(::testing::SaveArg<0>(&favicons));
  EXPECT_CALL(*bridge, NotifyItemEnded(importer::FAVORITES));
  EXPECT_CALL(*bridge, NotifyEnded());
  importer->StartImport(profile, importer::FAVORITES, bridge.get());
  ASSERT_EQ(6u, bookmarks.size());
  EXPECT_EQ("https://www.mozilla.org/en-US/firefox/central/",
            bookmarks[0].url.spec());
  EXPECT_EQ("https://www.mozilla.org/en-US/firefox/help/",
            bookmarks[1].url.spec());
  EXPECT_EQ("https://www.mozilla.org/en-US/firefox/customize/",
            bookmarks[2].url.spec());
  EXPECT_EQ("https://www.mozilla.org/en-US/contribute/",
            bookmarks[3].url.spec());
  EXPECT_EQ("https://www.mozilla.org/en-US/about/", bookmarks[4].url.spec());
  EXPECT_EQ("https://www.google.com/", bookmarks[5].url.spec());
  ASSERT_EQ(5u, favicons.size());
  EXPECT_EQ("http://www.mozilla.org/2005/made-up-favicon/0-1473403921346",
            favicons[0].favicon_url.spec());
  EXPECT_EQ("http://www.mozilla.org/2005/made-up-favicon/1-1473403921347",
            favicons[1].favicon_url.spec());
  EXPECT_EQ("http://www.mozilla.org/2005/made-up-favicon/2-1473403921348",
            favicons[2].favicon_url.spec());
  EXPECT_EQ("http://www.mozilla.org/2005/made-up-favicon/3-1473403921349",
            favicons[3].favicon_url.spec());
  EXPECT_EQ("http://www.mozilla.org/2005/made-up-favicon/4-1473403921349",
            favicons[4].favicon_url.spec());
}

TEST(FirefoxImporterTest, ImportHistorySchema) {
  using ::testing::_;
  base::FilePath places_path;
  ASSERT_TRUE(PathService::Get(chrome::DIR_TEST_DATA, &places_path));
  places_path =
      places_path.AppendASCII("import").AppendASCII("firefox").AppendASCII(
          "48.0.2");
  scoped_refptr<FirefoxImporter> ff_importer = new FirefoxImporter;
  importer::SourceProfile profile;
  profile.source_path = places_path;
  scoped_refptr<MockImporterBridge> bridge = new MockImporterBridge;
  std::vector<ImporterURLRow> history;
  EXPECT_CALL(*bridge, NotifyStarted());
  EXPECT_CALL(*bridge, NotifyItemStarted(importer::HISTORY));
  EXPECT_CALL(*bridge, SetHistoryItems(_, _))
      .WillOnce(::testing::SaveArg<0>(&history));
  EXPECT_CALL(*bridge, NotifyItemEnded(importer::HISTORY));
  EXPECT_CALL(*bridge, NotifyEnded());
  ff_importer->StartImport(profile, importer::HISTORY, bridge.get());
  ASSERT_EQ(3u, history.size());
  EXPECT_EQ("https://www.mozilla.org/en-US/firefox/48.0.2/firstrun/learnmore/",
            history[0].url.spec());
  EXPECT_EQ("https://www.mozilla.org/en-US/firefox/48.0.2/firstrun/",
            history[1].url.spec());
  EXPECT_EQ("http://google.com/", history[2].url.spec());
}
