// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <map>
#include <string>
#include <vector>

#include "chrome/browser/media/router/media_sinks_observer.h"

#ifndef CHROME_TEST_MEDIA_ROUTER_TEST_MEDIA_SINKS_OBSERVER_H_
#define CHROME_TEST_MEDIA_ROUTER_TEST_MEDIA_SINKS_OBSERVER_H_

namespace media_router {

class MediaRouter;

// Test class to implement MediaSinksObserver that receives SinkQueryResults
// from Media Router and is used for verification.
class TestMediaSinksObserver : public MediaSinksObserver {
 public:
  TestMediaSinksObserver(MediaRouter* router,
                         const MediaSource& source,
                         const url::Origin& origin);
  ~TestMediaSinksObserver() override;

  // MediaSinksObserver implementation.
  void OnSinksReceived(const std::vector<MediaSink>& result) override;

  // Map of <sink_name, media_sink_object>
  std::map<std::string, const MediaSink> sink_map;
};

}  // namespace media_router

#endif  // CHROME_TEST_MEDIA_ROUTER_TEST_MEDIA_SINKS_OBSERVER_H_
