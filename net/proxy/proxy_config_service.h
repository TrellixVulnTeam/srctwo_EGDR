// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_PROXY_PROXY_CONFIG_SERVICE_H_
#define NET_PROXY_PROXY_CONFIG_SERVICE_H_

#include "net/base/net_export.h"

namespace net {

class ProxyConfig;

// Service for watching when the proxy settings have changed.
class NET_EXPORT ProxyConfigService {
 public:
  // Indicates whether proxy configuration is valid, and if not, why.
  enum ConfigAvailability {
    // Configuration is pending, observers will be notified later.
    CONFIG_PENDING,
    // Configuration is present and valid.
    CONFIG_VALID,
    // No configuration is set.
    CONFIG_UNSET
  };

  // Observer for being notified when the proxy settings have changed.
  class NET_EXPORT Observer {
   public:
    virtual ~Observer() {}
    // Notification callback that should be invoked by ProxyConfigService
    // implementors whenever the configuration changes. |availability| indicates
    // the new availability status and can be CONFIG_UNSET or CONFIG_VALID (in
    // which case |config| contains the configuration). Implementors must not
    // pass CONFIG_PENDING.
    virtual void OnProxyConfigChanged(const ProxyConfig& config,
                                      ConfigAvailability availability) = 0;
  };

  virtual ~ProxyConfigService() {}

  // Adds/Removes an observer that will be called whenever the proxy
  // configuration has changed.
  virtual void AddObserver(Observer* observer) = 0;
  virtual void RemoveObserver(Observer* observer) = 0;

  // Gets the most recent availability status. If a configuration is present,
  // the proxy configuration is written to |config| and CONFIG_VALID is
  // returned. Returns CONFIG_PENDING if it is not available yet. In this case,
  // it is guaranteed that subscribed observers will be notified of a change at
  // some point in the future once the configuration is available.
  // Note that to avoid re-entrancy problems, implementations should not
  // dispatch any change notifications from within this function.
  virtual ConfigAvailability GetLatestProxyConfig(ProxyConfig* config) = 0;

  // ProxyService will call this periodically during periods of activity.
  // It can be used as a signal for polling-based implementations.
  //
  // Note that this is purely used as an optimization -- polling
  // implementations could simply set a global timer that goes off every
  // X seconds at which point they check for changes. However that has
  // the disadvantage of doing continuous work even during idle periods.
  virtual void OnLazyPoll() {}
};

}  // namespace net

#endif  // NET_PROXY_PROXY_CONFIG_SERVICE_H_
