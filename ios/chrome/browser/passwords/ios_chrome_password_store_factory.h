// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_PASSWORDS_IOS_CHROME_PASSWORD_STORE_FACTORY_H_
#define IOS_CHROME_BROWSER_PASSWORDS_IOS_CHROME_PASSWORD_STORE_FACTORY_H_

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "components/keyed_service/ios/refcounted_browser_state_keyed_service_factory.h"

enum class ServiceAccessType;

namespace base {
template <typename T>
struct DefaultSingletonTraits;
}

namespace ios {
class ChromeBrowserState;
}

namespace password_manager {
class PasswordStore;
}

// Singleton that owns all PasswordStores and associates them with
// ios::ChromeBrowserState.
class IOSChromePasswordStoreFactory
    : public RefcountedBrowserStateKeyedServiceFactory {
 public:
  static scoped_refptr<password_manager::PasswordStore> GetForBrowserState(
      ios::ChromeBrowserState* browser_state,
      ServiceAccessType access_type);

  static IOSChromePasswordStoreFactory* GetInstance();

  // Called by the PasswordDataTypeController whenever there is a possibility
  // that syncing passwords has just started or ended for |browser_state|.
  static void OnPasswordsSyncedStatePotentiallyChanged(
      ios::ChromeBrowserState* browser_state);

 private:
  friend struct base::DefaultSingletonTraits<IOSChromePasswordStoreFactory>;

  IOSChromePasswordStoreFactory();
  ~IOSChromePasswordStoreFactory() override;

  // BrowserStateKeyedServiceFactory:
  scoped_refptr<RefcountedKeyedService> BuildServiceInstanceFor(
      web::BrowserState* context) const override;
  web::BrowserState* GetBrowserStateToUse(
      web::BrowserState* context) const override;
  bool ServiceIsNULLWhileTesting() const override;

  DISALLOW_COPY_AND_ASSIGN(IOSChromePasswordStoreFactory);
};

#endif  // IOS_CHROME_BROWSER_PASSWORDS_IOS_CHROME_PASSWORD_STORE_FACTORY_H_
