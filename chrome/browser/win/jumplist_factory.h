// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_WIN_JUMPLIST_FACTORY_H_
#define CHROME_BROWSER_WIN_JUMPLIST_FACTORY_H_

#include "base/memory/singleton.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

class Profile;
class JumpList;

class JumpListFactory : public BrowserContextKeyedServiceFactory {
 public:
  static JumpList* GetForProfile(Profile* profile);

  static JumpListFactory* GetInstance();

 private:
  friend struct base::DefaultSingletonTraits<JumpListFactory>;
  JumpListFactory();
  ~JumpListFactory() override;

  // BrowserContextKeyedServiceFactory:
  KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const override;
};

#endif  // CHROME_BROWSER_WIN_JUMPLIST_FACTORY_H_
