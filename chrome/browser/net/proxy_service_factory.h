// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_NET_PROXY_SERVICE_FACTORY_H_
#define CHROME_BROWSER_NET_PROXY_SERVICE_FACTORY_H_

#include <memory>

#include "base/macros.h"

class PrefProxyConfigTracker;
class PrefService;

namespace net {
class ProxyConfigService;
}

class ProxyServiceFactory {
 public:
  // Creates a ProxyConfigService that delivers the system preferences
  // (or the respective ChromeOS equivalent).
  static std::unique_ptr<net::ProxyConfigService> CreateProxyConfigService(
      PrefProxyConfigTracker* tracker);

  // Creates a PrefProxyConfigTracker that tracks preferences of a
  // profile. On ChromeOS it additionaly tracks local state for shared proxy
  // settings. This tracker should be used if the profile's preferences should
  // be respected. On ChromeOS's signin screen this is for example not the case.
  static PrefProxyConfigTracker* CreatePrefProxyConfigTrackerOfProfile(
      PrefService* profile_prefs,
      PrefService* local_state_prefs);

  // Creates a PrefProxyConfigTracker that tracks local state only. This tracker
  // should be used for the system request context and the signin screen
  // (ChromeOS only).
  static PrefProxyConfigTracker* CreatePrefProxyConfigTrackerOfLocalState(
      PrefService* local_state_prefs);

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(ProxyServiceFactory);
};

#endif  // CHROME_BROWSER_NET_PROXY_SERVICE_FACTORY_H_
