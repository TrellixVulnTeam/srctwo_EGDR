// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_SERVICES_GCM_IOS_CHROME_GCM_PROFILE_SERVICE_FACTORY_H_
#define IOS_CHROME_BROWSER_SERVICES_GCM_IOS_CHROME_GCM_PROFILE_SERVICE_FACTORY_H_

#include <memory>
#include <string>

#include "base/macros.h"
#include "components/keyed_service/ios/browser_state_keyed_service_factory.h"

namespace base {
template <typename T>
struct DefaultSingletonTraits;
}

namespace gcm {
class GCMProfileService;
}

namespace ios {
class ChromeBrowserState;
}

// Singleton that owns all GCMProfileService and associates them with
// ios::ChromeBrowserState.
class IOSChromeGCMProfileServiceFactory
    : public BrowserStateKeyedServiceFactory {
 public:
  static gcm::GCMProfileService* GetForBrowserState(
      ios::ChromeBrowserState* browser_state);

  static IOSChromeGCMProfileServiceFactory* GetInstance();

  // Returns a string like "com.chrome.ios" that should be used as the GCM
  // category when an app_id is sent as a subtype instead of as a category. This
  // string must never change during the lifetime of a Chrome install, since
  // e.g. to unregister an Instance ID token the same category must be passed to
  // GCM as was originally passed when registering it.
  static std::string GetProductCategoryForSubtypes();

 private:
  friend struct base::DefaultSingletonTraits<IOSChromeGCMProfileServiceFactory>;

  IOSChromeGCMProfileServiceFactory();
  ~IOSChromeGCMProfileServiceFactory() override;

  // BrowserStateKeyedServiceFactory:
  std::unique_ptr<KeyedService> BuildServiceInstanceFor(
      web::BrowserState* context) const override;

  DISALLOW_COPY_AND_ASSIGN(IOSChromeGCMProfileServiceFactory);
};

#endif  // IOS_CHROME_BROWSER_SERVICES_GCM_IOS_CHROME_GCM_PROFILE_SERVICE_FACTORY_H_
