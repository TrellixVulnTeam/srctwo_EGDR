// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/chrome/browser/app_startup_parameters.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"

namespace {

struct UniversalLinkDecodeTestCase {
  GURL universal_link;
  std::map<std::string, std::string> expected_external_url_params;
  bool expected_complete_payment_request;
};

TEST(AppStartupParameters, QueryParametersPaymentRequest) {
  const UniversalLinkDecodeTestCase test_cases[] = {
      {
          GURL("https://goo.gl/ioschrome/"), {}, false,
      },
      {
          GURL("https://goo.gl/ioschrome?payment-request-id"), {}, false,
      },
      {
          GURL("https://goo.gl/ioschrome?payment-request-id=092831-af18l-324"
               "&payment-request-data="),
          {{"payment-request-id", "092831-af18l-324"},
           {"payment-request-data", ""}},
          true,
      },
      {
          GURL("https://goo.gl/ioschrome?payment-request-id=092831-af18l-324"
               "&payment-request-data=JNLSKFknrlwe80dkzrnLEWR"),
          {{"payment-request-id", "092831-af18l-324"},
           {"payment-request-data", "JNLSKFknrlwe80dkzrnLEWR"}},
          true,
      },
      {
          GURL("https://goo.gl/ioschrome?payment-request-id=092831-af18l-324"
               "&payment-request-data=JNLSKFknrlwe80dkzrnLEWR"
               "&unecessary-parameter=0"),
          {{"payment-request-id", "092831-af18l-324"},
           {"payment-request-data", "JNLSKFknrlwe80dkzrnLEWR"},
           {"unecessary-parameter", "0"}},
          true,
      },
  };

  for (size_t i = 0; i < arraysize(test_cases); ++i) {
    const UniversalLinkDecodeTestCase& test_case = test_cases[i];
    AppStartupParameters* params = [[AppStartupParameters alloc]
        initWithUniversalLink:test_case.universal_link];
    EXPECT_EQ(test_case.expected_external_url_params, params.externalURLParams);
    EXPECT_EQ(test_case.expected_complete_payment_request,
              params.completePaymentRequest);
  }
}

}  // namespace
