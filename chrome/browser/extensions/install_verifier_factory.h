// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_INSTALL_VERIFIER_FACTORY_H_
#define CHROME_BROWSER_EXTENSIONS_INSTALL_VERIFIER_FACTORY_H_

#include "base/macros.h"
#include "base/memory/singleton.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

namespace extensions {

class InstallVerifier;

class InstallVerifierFactory : public BrowserContextKeyedServiceFactory {
 public:
  static InstallVerifier* GetForBrowserContext(
      content::BrowserContext* context);
  static InstallVerifierFactory* GetInstance();

 private:
  friend struct base::DefaultSingletonTraits<InstallVerifierFactory>;

  InstallVerifierFactory();
  ~InstallVerifierFactory() override;

  // BrowserContextKeyedServiceFactory implementation
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
  content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const override;

  DISALLOW_COPY_AND_ASSIGN(InstallVerifierFactory);
};

}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_INSTALL_VERIFIER_FACTORY_H_
