// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_SYNC_IOS_USER_EVENT_SERVICE_FACTORY_H_
#define IOS_CHROME_BROWSER_SYNC_IOS_USER_EVENT_SERVICE_FACTORY_H_

#include <memory>

#include "base/macros.h"
#include "components/keyed_service/ios/browser_state_keyed_service_factory.h"

namespace base {
template <typename T>
struct DefaultSingletonTraits;
}  // namespace base

namespace ios {
class ChromeBrowserState;
}  // namespace ios

namespace syncer {
class UserEventService;
}  // namespace syncer

// Singleton that associates UserEventServices to ChromeBrowserStates.
class IOSUserEventServiceFactory : public BrowserStateKeyedServiceFactory {
 public:
  static syncer::UserEventService* GetForBrowserState(
      ios::ChromeBrowserState* browser_state);

  static IOSUserEventServiceFactory* GetInstance();

 private:
  friend struct base::DefaultSingletonTraits<IOSUserEventServiceFactory>;

  IOSUserEventServiceFactory();
  ~IOSUserEventServiceFactory() override;

  // BrowserStateKeyedServiceFactory implementation.
  std::unique_ptr<KeyedService> BuildServiceInstanceFor(
      web::BrowserState* context) const override;
  web::BrowserState* GetBrowserStateToUse(
      web::BrowserState* context) const override;
};

#endif  // IOS_CHROME_BROWSER_SYNC_IOS_USER_EVENT_SERVICE_FACTORY_H_
