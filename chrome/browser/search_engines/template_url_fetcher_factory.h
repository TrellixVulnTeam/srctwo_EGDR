// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SEARCH_ENGINES_TEMPLATE_URL_FETCHER_FACTORY_H_
#define CHROME_BROWSER_SEARCH_ENGINES_TEMPLATE_URL_FETCHER_FACTORY_H_

#include "base/macros.h"
#include "base/memory/singleton.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

class Profile;
class TemplateURLFetcher;

// Singleton that owns all TemplateURLFetcher and associates them with
// Profiles.
class TemplateURLFetcherFactory : public BrowserContextKeyedServiceFactory {
 public:
  static TemplateURLFetcher* GetForProfile(Profile* profile);

  static TemplateURLFetcherFactory* GetInstance();

  // In some tests, the template url fetcher needs to be shutdown to
  // remove any dangling url requests before the io_thread is shutdown
  // to prevent leaks.
  static void ShutdownForProfile(Profile* profile);

 private:
  friend struct base::DefaultSingletonTraits<TemplateURLFetcherFactory>;

  TemplateURLFetcherFactory();
  ~TemplateURLFetcherFactory() override;

  // BrowserContextKeyedServiceFactory:
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* profile) const override;
  content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const override;

  DISALLOW_COPY_AND_ASSIGN(TemplateURLFetcherFactory);
};

#endif  // CHROME_BROWSER_SEARCH_ENGINES_TEMPLATE_URL_FETCHER_FACTORY_H_
