// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_BROWSER_STATE_CHROME_BROWSER_STATE_MANAGER_H_
#define IOS_CHROME_BROWSER_BROWSER_STATE_CHROME_BROWSER_STATE_MANAGER_H_

#include <vector>

#include "base/compiler_specific.h"
#include "base/macros.h"

namespace base {
class FilePath;
}

class BrowserStateInfoCache;

namespace ios {

class ChromeBrowserState;

// Provides methods that allow for various ways of creating non-incognito
// ChromeBrowserState instances. Owns all instances that it creates.
class ChromeBrowserStateManager {
 public:
  virtual ~ChromeBrowserStateManager() {}

  // Returns the ChromeBrowserState that was last used, creating one if
  // necessary.
  virtual ChromeBrowserState* GetLastUsedBrowserState() = 0;

  // Returns the ChromeBrowserState associated with |path|, creating one if
  // necessary.
  virtual ChromeBrowserState* GetBrowserState(const base::FilePath& path) = 0;

  // Returns the BrowserStateInfoCache associated with this manager.
  virtual BrowserStateInfoCache* GetBrowserStateInfoCache() = 0;

  // Returns the list of loaded ChromeBrowserStates.
  virtual std::vector<ChromeBrowserState*> GetLoadedBrowserStates() = 0;

 protected:
  ChromeBrowserStateManager() {}

 private:
  DISALLOW_COPY_AND_ASSIGN(ChromeBrowserStateManager);
};

}  // namespace ios

#endif  // IOS_CHROME_BROWSER_BROWSER_STATE_CHROME_BROWSER_STATE_MANAGER_H_
