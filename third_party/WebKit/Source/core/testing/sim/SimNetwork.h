// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SimNetwork_h
#define SimNetwork_h

#include "platform/wtf/HashMap.h"
#include "platform/wtf/text/StringHash.h"
#include "platform/wtf/text/WTFString.h"
#include "public/platform/WebURLLoaderTestDelegate.h"

namespace blink {

class SimRequest;
class WebURLLoaderClient;
class WebURLResponse;

// Simulates a network with precise flow control so you can make requests
// return, write data, and finish in a specific order in a unit test. One of
// these must be created before using the SimRequest to issue requests.
class SimNetwork final : public WebURLLoaderTestDelegate {
 public:
  SimNetwork();
  ~SimNetwork();

 private:
  friend class SimRequest;

  static SimNetwork& Current();

  void ServePendingRequests();
  void AddRequest(SimRequest&);
  void RemoveRequest(SimRequest&);

  // WebURLLoaderTestDelegate
  void DidReceiveResponse(WebURLLoaderClient*, const WebURLResponse&) override;
  void DidReceiveData(WebURLLoaderClient*,
                      const char* data,
                      int data_length) override;
  void DidFail(WebURLLoaderClient*,
               const WebURLError&,
               int64_t total_encoded_data_length,
               int64_t total_encoded_body_length,
               int64_t total_decoded_body_length) override;
  void DidFinishLoading(WebURLLoaderClient*,
                        double finish_time,
                        int64_t total_encoded_data_length,
                        int64_t total_encoded_body_length,
                        int64_t total_decoded_body_length) override;

  SimRequest* current_request_;
  HashMap<String, SimRequest*> requests_;
};

}  // namespace blink

#endif
