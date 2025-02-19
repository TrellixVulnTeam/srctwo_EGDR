// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_RECOVERY_RECOVERY_INSTALL_GLOBAL_ERROR_FACTORY_H_
#define CHROME_BROWSER_RECOVERY_RECOVERY_INSTALL_GLOBAL_ERROR_FACTORY_H_

#include "base/macros.h"
#include "base/memory/singleton.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

class Profile;
class RecoveryInstallGlobalError;

// Singleton that owns all RecoveryInstallGlobalError and associates them with
// Profiles. Listens for the Profile's destruction notification and cleans up
// the associated RecoveryInstallGlobalError.
class RecoveryInstallGlobalErrorFactory
    : public BrowserContextKeyedServiceFactory {
 public:
  // Returns the instance of RecoveryInstallGlobalError associated with this
  // profile, creating one if none exists.
  static RecoveryInstallGlobalError* GetForProfile(Profile* profile);

  // Returns an instance of the RecoveryInstallGlobalErrorFactory singleton.
  static RecoveryInstallGlobalErrorFactory* GetInstance();

 private:
  friend struct base::DefaultSingletonTraits<RecoveryInstallGlobalErrorFactory>;

  RecoveryInstallGlobalErrorFactory();
  ~RecoveryInstallGlobalErrorFactory() override;

  // BrowserContextKeyedServiceFactory:
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* profile) const override;

  DISALLOW_COPY_AND_ASSIGN(RecoveryInstallGlobalErrorFactory);
};

#endif  // CHROME_BROWSER_RECOVERY_RECOVERY_INSTALL_GLOBAL_ERROR_FACTORY_H_
