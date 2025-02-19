// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_BROWSER_EVENT_ROUTER_FACTORY_H_
#define EXTENSIONS_BROWSER_EVENT_ROUTER_FACTORY_H_

#include "base/macros.h"
#include "base/memory/singleton.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

namespace extensions {

class EventRouter;

class EventRouterFactory : public BrowserContextKeyedServiceFactory {
 public:
  static EventRouter* GetForBrowserContext(content::BrowserContext* context);
  static EventRouterFactory* GetInstance();

 private:
  friend struct base::DefaultSingletonTraits<EventRouterFactory>;

  EventRouterFactory();
  ~EventRouterFactory() override;

  // BrowserContextKeyedServiceFactory implementation
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
  content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const override;

  DISALLOW_COPY_AND_ASSIGN(EventRouterFactory);
};

}  // namespace extensions

#endif  // EXTENSIONS_BROWSER_EVENT_ROUTER_FACTORY_H_
