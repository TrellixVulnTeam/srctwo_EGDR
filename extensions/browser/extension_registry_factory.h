// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_BROWSER_EXTENSION_REGISTRY_FACTORY_H_
#define EXTENSIONS_BROWSER_EXTENSION_REGISTRY_FACTORY_H_

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/singleton.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

namespace extensions {

class ExtensionRegistry;

// Factory for ExtensionRegistry objects. ExtensionRegistry objects are shared
// between an incognito browser context and its master browser context.
class ExtensionRegistryFactory : public BrowserContextKeyedServiceFactory {
 public:
  static ExtensionRegistry* GetForBrowserContext(
      content::BrowserContext* context);

  static ExtensionRegistryFactory* GetInstance();

 private:
  friend struct base::DefaultSingletonTraits<ExtensionRegistryFactory>;

  ExtensionRegistryFactory();
  ~ExtensionRegistryFactory() override;

  // BrowserContextKeyedServiceFactory implementation:
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
  content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const override;

  DISALLOW_COPY_AND_ASSIGN(ExtensionRegistryFactory);
};

}  // namespace extensions

#endif  // EXTENSIONS_BROWSER_EXTENSION_REGISTRY_FACTORY_H_
