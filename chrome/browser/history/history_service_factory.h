// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_HISTORY_HISTORY_SERVICE_FACTORY_H_
#define CHROME_BROWSER_HISTORY_HISTORY_SERVICE_FACTORY_H_

#include "base/memory/singleton.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"
#include "components/keyed_service/core/service_access_type.h"

class Profile;

namespace history {
class HistoryService;
}

// Singleton that owns all HistoryService and associates them with
// Profiles.
class HistoryServiceFactory : public BrowserContextKeyedServiceFactory {
 public:
  static history::HistoryService* GetForProfile(Profile* profile,
                                                ServiceAccessType sat);

  static history::HistoryService* GetForProfileIfExists(Profile* profile,
                                                        ServiceAccessType sat);

  static history::HistoryService* GetForProfileWithoutCreating(
      Profile* profile);

  static HistoryServiceFactory* GetInstance();

  // In the testing profile, we often clear the history before making a new
  // one. This takes care of that work. It should only be used in tests.
  // Note: This does not do any cleanup; it only destroys the service. The
  // calling test is expected to do the cleanup before calling this function.
  static void ShutdownForProfile(Profile* profile);

 private:
  friend struct base::DefaultSingletonTraits<HistoryServiceFactory>;

  HistoryServiceFactory();
  ~HistoryServiceFactory() override;

  // BrowserContextKeyedServiceFactory:
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
  content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const override;
  bool ServiceIsNULLWhileTesting() const override;
};

#endif  // CHROME_BROWSER_HISTORY_HISTORY_SERVICE_FACTORY_H_
