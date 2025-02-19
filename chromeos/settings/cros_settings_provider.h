// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_SETTINGS_CROS_SETTINGS_PROVIDER_H_
#define CHROMEOS_SETTINGS_CROS_SETTINGS_PROVIDER_H_

#include <string>

#include "base/callback.h"
#include "chromeos/chromeos_export.h"

namespace base {
class Value;
}

namespace chromeos {

class CHROMEOS_EXPORT CrosSettingsProvider {
 public:
  // The callback type that is called to notify the CrosSettings observers
  // about a setting change.
  typedef base::Callback<void(const std::string&)> NotifyObserversCallback;

  // Possible results of a trusted check.
  enum TrustedStatus {
    // The trusted values were populated in the cache and can be accessed
    // until the next iteration of the message loop.
    TRUSTED,
    // Either a store or a load operation is in progress. The provided
    // callback will be invoked once the verification has finished.
    TEMPORARILY_UNTRUSTED,
    // The verification of the trusted store has failed permanently. The
    // client should assume this state final and further checks for
    // trustedness will fail at least until the browser restarts.
    PERMANENTLY_UNTRUSTED,
  };

  // Creates a new provider instance. |notify_cb| will be used to notify
  // about setting changes.
  explicit CrosSettingsProvider(const NotifyObserversCallback& notify_cb);
  virtual ~CrosSettingsProvider();

  // Sets |in_value| to given |path| in cros settings.
  void Set(const std::string& path, const base::Value& in_value);

  // Gets settings value of given |path| to |out_value|.
  virtual const base::Value* Get(const std::string& path) const = 0;

  // Requests the provider to fetch its values from a trusted store, if it
  // hasn't done so yet. Returns TRUSTED if the values returned by this provider
  // are trusted during the current loop cycle. Otherwise returns
  // TEMPORARILY_UNTRUSTED, and |callback| will be invoked later when trusted
  // values become available, PrepareTrustedValues() should be tried again in
  // that case. Returns PERMANENTLY_UNTRUSTED if a permanent error has occurred.
  virtual TrustedStatus PrepareTrustedValues(
      const base::Closure& callback) = 0;

  // Gets the namespace prefix provided by this provider.
  virtual bool HandlesSetting(const std::string& path) const = 0;

  void SetNotifyObserversCallback(const NotifyObserversCallback& notify_cb);

 protected:
  // Notifies the observers about a setting change.
  void NotifyObservers(const std::string& path);

 private:
  // Does the real job for Set().
  virtual void DoSet(const std::string& path,
                     const base::Value& in_value) = 0;

  // Callback used to notify about setting changes.
  NotifyObserversCallback notify_cb_;
};

}  // namespace chromeos

#endif  // CHROMEOS_SETTINGS_CROS_SETTINGS_PROVIDER_H_
