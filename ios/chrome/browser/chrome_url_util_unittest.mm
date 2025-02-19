// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/chrome_url_util.h"

#include "base/macros.h"
#include "base/strings/sys_string_conversions.h"
#include "ios/chrome/browser/chrome_url_constants.h"
#import "net/base/mac/url_conversions.h"
#import "testing/gtest_mac.h"
#include "url/gurl.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

namespace {

TEST(ChromeURLUtilTest, TestIsExternalFileReference) {
  GURL external_url("chrome://external-file/foo/bar");
  GURL not_external_url("chrome://foo/bar");
  GURL still_not_external_url("http://external-file/foo/bar");
  EXPECT_TRUE(UrlIsExternalFileReference(external_url));
  EXPECT_FALSE(UrlIsExternalFileReference(not_external_url));
  EXPECT_FALSE(UrlIsExternalFileReference(still_not_external_url));
}

const char* kSchemeTestData[] = {
    "http://foo.com",
    "https://foo.com",
    "data:text/html;charset=utf-8,Hello",
    "about:blank",
    "chrome://settings",
};

// Tests UrlHasChromeScheme with NSURL* parameter.
TEST(ChromeURLUtilTest, NSURLHasChromeScheme) {
  for (unsigned int i = 0; i < arraysize(kSchemeTestData); ++i) {
    const char* url = kSchemeTestData[i];
    NSURL* nsurl = [NSURL URLWithString:base::SysUTF8ToNSString(url)];
    bool nsurl_result = UrlHasChromeScheme(nsurl);
    EXPECT_EQ(GURL(url).SchemeIs(kChromeUIScheme), nsurl_result)
        << "Scheme check failed for " << url;
  }
}

// Tests UrlHasChromeScheme with const GURL& paramter.
TEST(ChromeURLUtilTest, GURLHasChromeScheme) {
  for (unsigned int i = 0; i < arraysize(kSchemeTestData); ++i) {
    GURL gurl(kSchemeTestData[i]);
    bool result = UrlHasChromeScheme(gurl);
    EXPECT_EQ(gurl.SchemeIs(kChromeUIScheme), result)
        << "Scheme check failed for " << gurl.spec();
  }
}

TEST(ChromeURLUtilTest, GetBundleURLScheme) {
  // Verifies that there is some default values.
  ChromeAppConstants* constants = [ChromeAppConstants sharedInstance];
  NSString* originalScheme = [constants getBundleURLScheme];
  EXPECT_GT([originalScheme length], 0U);

  // Verifies that Chrome scheme can be reset for testing.
  [constants setCallbackSchemeForTesting:@"blah"];
  EXPECT_NSEQ(@"blah", [constants getBundleURLScheme]);

  // Resets state in case of further tests.
  [constants setCallbackSchemeForTesting:originalScheme];
}

}  // namespace
