// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_BROWSER_UI_PREFS_H_
#define CHROME_BROWSER_UI_BROWSER_UI_PREFS_H_

#include <string>

class PrefRegistrySimple;

namespace user_prefs {
class PrefRegistrySyncable;
}

namespace chrome {

void RegisterBrowserPrefs(PrefRegistrySimple* registry);
void RegisterBrowserUserPrefs(user_prefs::PrefRegistrySyncable* registry);

}  // namespace chrome

#endif  // CHROME_BROWSER_UI_BROWSER_UI_PREFS_H_
