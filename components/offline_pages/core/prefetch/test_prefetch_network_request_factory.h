// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_OFFLINE_PAGES_CORE_PREFETCH_TEST_PREFETCH_NETWORK_REQUEST_FACTORY_H_
#define COMPONENTS_OFFLINE_PAGES_CORE_PREFETCH_TEST_PREFETCH_NETWORK_REQUEST_FACTORY_H_

#include <memory>
#include <string>
#include <vector>

#include "base/memory/ref_counted.h"
#include "components/offline_pages/core/prefetch/prefetch_network_request_factory_impl.h"
#include "components/offline_pages/core/prefetch/prefetch_types.h"
#include "components/version_info/channel.h"
#include "net/url_request/url_request_test_util.h"

namespace offline_pages {

// Test factory that uses a TestURLRequestContextGetter.
// manipulation.
class TestPrefetchNetworkRequestFactory
    : public PrefetchNetworkRequestFactoryImpl {
 public:
  TestPrefetchNetworkRequestFactory();
  explicit TestPrefetchNetworkRequestFactory(
      net::TestURLRequestContextGetter* request_context);
  ~TestPrefetchNetworkRequestFactory() override;

  scoped_refptr<net::TestURLRequestContextGetter> request_context;
};

}  // namespace offline_pages

#endif  // COMPONENTS_OFFLINE_PAGES_CORE_PREFETCH_TEST_PREFETCH_NETWORK_REQUEST_FACTORY_H_
