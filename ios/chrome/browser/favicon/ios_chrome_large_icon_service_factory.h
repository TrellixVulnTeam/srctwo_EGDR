// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_FAVICON_IOS_CHROME_LARGE_ICON_SERVICE_FACTORY_H_
#define IOS_CHROME_BROWSER_FAVICON_IOS_CHROME_LARGE_ICON_SERVICE_FACTORY_H_

#include <memory>

#include "base/macros.h"
#include "components/keyed_service/ios/browser_state_keyed_service_factory.h"

class KeyedService;

namespace base {
template <typename T>
struct DefaultSingletonTraits;
}

namespace favicon {
class LargeIconService;
};

namespace ios {
class ChromeBrowserState;
}

// Singleton that owns all LargeIconService and associates them with
// ChromeBrowserState.
class IOSChromeLargeIconServiceFactory
    : public BrowserStateKeyedServiceFactory {
 public:
  static favicon::LargeIconService* GetForBrowserState(
      ios::ChromeBrowserState* browser_state);

  static IOSChromeLargeIconServiceFactory* GetInstance();

 private:
  friend struct base::DefaultSingletonTraits<IOSChromeLargeIconServiceFactory>;

  IOSChromeLargeIconServiceFactory();
  ~IOSChromeLargeIconServiceFactory() override;

  // BrowserStateKeyedServiceFactory implementation.
  std::unique_ptr<KeyedService> BuildServiceInstanceFor(
      web::BrowserState* context) const override;
  web::BrowserState* GetBrowserStateToUse(
      web::BrowserState* context) const override;
  bool ServiceIsNULLWhileTesting() const override;

  DISALLOW_COPY_AND_ASSIGN(IOSChromeLargeIconServiceFactory);
};

#endif  // IOS_CHROME_BROWSER_FAVICON_IOS_CHROME_LARGE_ICON_SERVICE_FACTORY_H_
