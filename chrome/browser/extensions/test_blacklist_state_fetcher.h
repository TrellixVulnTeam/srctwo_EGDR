// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_TEST_BLACKLIST_STATE_FETCHER_H_
#define CHROME_BROWSER_EXTENSIONS_TEST_BLACKLIST_STATE_FETCHER_H_

#include <string>

#include "base/macros.h"
#include "chrome/browser/extensions/blacklist_state_fetcher.h"
#include "chrome/common/safe_browsing/crx_info.pb.h"
#include "net/url_request/test_url_fetcher_factory.h"

namespace extensions {

// A wrapper for extensions::BlacklistStateFetcher, emulating server responses.
class TestBlacklistStateFetcher {
 public:
  explicit TestBlacklistStateFetcher(BlacklistStateFetcher* fetcher);

  ~TestBlacklistStateFetcher();

  void SetBlacklistVerdict(const std::string& id,
                           ClientCRXListInfoResponse_Verdict state);

  // Get URLFetcher by its id from factory, send the appropriate response.
  // Return false, if fetcher with fiven id doesn't exist or in case of
  // incorrect request. Otherwise return true.
  bool HandleFetcher(int id);

 private:
  BlacklistStateFetcher* fetcher_;

  std::map<std::string, ClientCRXListInfoResponse_Verdict> verdicts_;

  net::TestURLFetcherFactory url_fetcher_factory_;

  DISALLOW_COPY_AND_ASSIGN(TestBlacklistStateFetcher);
};

}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_TEST_BLACKLIST_STATE_FETCHER_H_
