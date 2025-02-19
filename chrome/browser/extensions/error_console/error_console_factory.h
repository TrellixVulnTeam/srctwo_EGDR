// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_ERROR_CONSOLE_ERROR_CONSOLE_FACTORY_H_
#define CHROME_BROWSER_EXTENSIONS_ERROR_CONSOLE_ERROR_CONSOLE_FACTORY_H_

#include "base/macros.h"
#include "base/memory/singleton.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

namespace extensions {

class ErrorConsole;

class ErrorConsoleFactory : public BrowserContextKeyedServiceFactory {
 public:
  static ErrorConsole* GetForBrowserContext(content::BrowserContext* context);
  static ErrorConsoleFactory* GetInstance();

 private:
  friend struct base::DefaultSingletonTraits<ErrorConsoleFactory>;

  ErrorConsoleFactory();
  ~ErrorConsoleFactory() override;

  // BrowserContextKeyedServiceFactory implementation
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
  content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const override;

  DISALLOW_COPY_AND_ASSIGN(ErrorConsoleFactory);
};

}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_ERROR_CONSOLE_ERROR_CONSOLE_FACTORY_H_
