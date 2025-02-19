// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/profiles/incognito_helpers.h"

#include "chrome/browser/profiles/profile.h"

namespace chrome {

content::BrowserContext* GetBrowserContextRedirectedInIncognito(
    content::BrowserContext* context) {
  return static_cast<Profile*>(context)->GetOriginalProfile();
}

content::BrowserContext* GetBrowserContextOwnInstanceInIncognito(
    content::BrowserContext* context) {
  return context;
}

}  // namespace chrome
