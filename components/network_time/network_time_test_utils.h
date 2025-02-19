// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_NETWORK_TIME_NETWORK_TIME_TEST_UTILS_H_
#define COMPONENTS_NETWORK_TIME_NETWORK_TIME_TEST_UTILS_H_

#include <map>
#include <memory>

#include "base/macros.h"
#include "components/network_time/network_time_tracker.h"

namespace base {
namespace test {
class ScopedFeatureList;
};  // namespace test

class FieldTrialList;
}  // namespace base

namespace net {
namespace test_server {
struct HttpRequest;
class HttpResponse;
}  // namespace test_server
}  // namespace net

namespace network_time {

// The body of a valid time response. Can be returned, with
// |kGoodTimeResponseServerProofHeader|, in responses from test servers
// to simulate a network time server. This response uses 1 as the key
// version and 123123123 as the nonce. Use
// NetworkTimeTracker::OverrideNonceForTesting() to set the nonce so
// that this response validates.
extern const char kGoodTimeResponseBody[];

// The x-cup-server-proof header value that should be served along with
// |kGoodTimeResponseBody| to make a test server response be accepted by
// NetworkTimeTracker as a valid response.
extern const char kGoodTimeResponseServerProofHeader[];

// The time that |kGoodTimeResponseBody| uses. Can be converted to a
// base::Time with base::Time::FromJsTime.
extern const double kGoodTimeResponseHandlerJsTime;

// Returns a valid network time response using the constants above. See
// comments in the .cc for how to update the time returned in the response.
std::unique_ptr<net::test_server::HttpResponse> GoodTimeResponseHandler(
    const net::test_server::HttpRequest& request);

// Allows unit tests to configure the network time queries field trial.
class FieldTrialTest {
 public:
  FieldTrialTest();
  virtual ~FieldTrialTest();

  void SetNetworkQueriesWithVariationsService(
      bool enable,
      float query_probability,
      NetworkTimeTracker::FetchBehavior fetch_behavior);

 private:
  std::unique_ptr<base::FieldTrialList> field_trial_list_;
  std::unique_ptr<base::test::ScopedFeatureList> scoped_feature_list_;

  DISALLOW_COPY_AND_ASSIGN(FieldTrialTest);
};

}  // namespace network_time

#endif  // COMPONENTS_NETWORK_TIME_NETWORK_TIME_TEST_UTILS_H_
