// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_CONTENT_SETTINGS_CORE_BROWSER_CONTENT_SETTINGS_OBSERVABLE_PROVIDER_H_
#define COMPONENTS_CONTENT_SETTINGS_CORE_BROWSER_CONTENT_SETTINGS_OBSERVABLE_PROVIDER_H_

#include <string>

#include "base/observer_list.h"
#include "base/threading/thread_checker.h"
#include "components/content_settings/core/browser/content_settings_observer.h"
#include "components/content_settings/core/browser/content_settings_provider.h"
#include "components/content_settings/core/common/content_settings_pattern.h"

namespace content_settings {

class ObservableProvider : public ProviderInterface {
 public:
  ObservableProvider();
  ~ObservableProvider() override;

  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);

 protected:
  void NotifyObservers(const ContentSettingsPattern& primary_pattern,
                       const ContentSettingsPattern& secondary_pattern,
                       ContentSettingsType content_type,
                       const std::string& resource_identifier);
  void RemoveAllObservers();
  bool CalledOnValidThread();

 private:
  base::ThreadChecker thread_checker_;
  base::ObserverList<Observer, true> observer_list_;
};

}  // namespace content_settings

#endif  // COMPONENTS_CONTENT_SETTINGS_CORE_BROWSER_CONTENT_SETTINGS_OBSERVABLE_PROVIDER_H_
