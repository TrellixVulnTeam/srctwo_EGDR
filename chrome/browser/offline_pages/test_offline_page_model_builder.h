// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_OFFLINE_PAGES_TEST_OFFLINE_PAGE_MODEL_BUILDER_H_
#define CHROME_BROWSER_OFFLINE_PAGES_TEST_OFFLINE_PAGE_MODEL_BUILDER_H_

#include <memory>

class KeyedService;

namespace content {
class BrowserContext;
}

namespace offline_pages {

// Helper function to be used with
// BrowserContextKeyedServiceFactory::SetTestingFactory() that returns a
// OfflinePageModel object with mocked store.
std::unique_ptr<KeyedService> BuildTestOfflinePageModel(
    content::BrowserContext* context);

}  // namespace offline_pages

#endif  // CHROME_BROWSER_OFFLINE_PAGES_TEST_OFFLINE_PAGE_MODEL_BUILDER_H_
