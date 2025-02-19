// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/chrome/browser/web/mailto_handler.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/gtest_mac.h"
#include "url/gurl.h"

// Tests constructor.
TEST(MailtoHandlerTest, TestConstructor) {
  MailtoHandler* handler =
      [[MailtoHandler alloc] initWithName:@"Some App" appStoreID:@"12345"];
  EXPECT_NSEQ(@"Some App", [handler appName]);
  EXPECT_NSEQ(@"12345", [handler appStoreID]);
  EXPECT_NSEQ(@"mailtohandler:/co?", [handler beginningScheme]);
}

// Tests mailto URL with and without a subject.
TEST(MailtoHandlerTest, TestRewriteGood) {
  MailtoHandler* handler =
      [[MailtoHandler alloc] initWithName:@"Some App" appStoreID:@"12345"];
  NSString* result = [handler rewriteMailtoURL:GURL("mailto:user@domain.com")];
  EXPECT_NSEQ(@"mailtohandler:/co?to=user@domain.com", result);
  // Tests mailto URL with a subject.
  result =
      [handler rewriteMailtoURL:GURL("mailto:user@domain.com?subject=hello")];
  EXPECT_NSEQ(@"mailtohandler:/co?to=user@domain.com&subject=hello", result);
}

// Tests mailto URL with unrecognized query parameters.
TEST(MailtoHandlerTest, TestRewriteUnrecognizedParams) {
  MailtoHandler* handler =
      [[MailtoHandler alloc] initWithName:@"Some App" appStoreID:@"12345"];
  NSString* result = [handler
      rewriteMailtoURL:
          GURL("mailto:someone@there.com?garbage=in&garbageOut&subject=trash")];
  EXPECT_NSEQ(@"mailtohandler:/co?to=someone@there.com&subject=trash", result);
}

// Tests mailto URL with a body that includes a = sign.
TEST(MailtoHandlerTest, TestRewriteBodyWithUrl) {
  MailtoHandler* handler =
      [[MailtoHandler alloc] initWithName:@"Some App" appStoreID:@"12345"];
  NSString* result = [handler
      rewriteMailtoURL:GURL("mailto:user@domain.com?body=http://foo.bar?x=y")];
  EXPECT_NSEQ(@"mailtohandler:/co?to=user@domain.com&body=http://foo.bar?x=y",
              result);
}

// Tests mailto URL with parameters that are mixed upper/lower cases.
TEST(MailtoHandlerTest, TestRewriteWithMixedCase) {
  MailtoHandler* handler =
      [[MailtoHandler alloc] initWithName:@"Some App" appStoreID:@"12345"];
  NSString* result =
      [handler rewriteMailtoURL:GURL("mailto:?Subject=Blah&BODY=stuff")];
  EXPECT_NSEQ(@"mailtohandler:/co?subject=Blah&body=stuff", result);
}

// Tests that non-mailto URLs returns nil.
TEST(MailtoHandlerTest, TestRewriteNotMailto) {
  MailtoHandler* handler =
      [[MailtoHandler alloc] initWithName:@"Some App" appStoreID:@"12345"];
  NSString* result = [handler rewriteMailtoURL:GURL("http://www.google.com")];
  EXPECT_FALSE(result);
}
