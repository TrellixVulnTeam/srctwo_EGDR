// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMECAST_SERVICE_CAST_SERVICE_H_
#define CHROMECAST_SERVICE_CAST_SERVICE_H_

#include <memory>

#include "base/callback.h"
#include "base/macros.h"

class PrefService;

namespace base {
class ThreadChecker;
}

namespace content {
class BrowserContext;
}

namespace chromecast {

class CastService {
 public:
  virtual ~CastService();

  // Initializes/finalizes the cast service.
  void Initialize();
  void Finalize();

  // Starts/stops the cast service.
  void Start();
  void Stop();

 protected:
  CastService(content::BrowserContext* browser_context,
              PrefService* pref_service);

  // Implementation-specific initialization. Initialization of cast service's
  // sub-components, and anything that requires IO operations should go here.
  // Anything that should happen before cast service is started but doesn't need
  // the sub-components to finish initializing should also go here.
  virtual void InitializeInternal() = 0;

  // Implementation-specific finalization. Any initializations done by
  // InitializeInternal() should be finalized here.
  virtual void FinalizeInternal() = 0;

  // Implementation-specific start behavior. It basically starts the
  // sub-component services and does additional initialization that cannot be
  // done in the InitializationInternal().
  virtual void StartInternal() = 0;

  // Implementation-specific stop behavior. Any initializations done by
  // StartInternal() should be finalized here.
  virtual void StopInternal() = 0;

  content::BrowserContext* browser_context() const { return browser_context_; }
  PrefService* pref_service() const { return pref_service_; }

 private:
  content::BrowserContext* const browser_context_;
  PrefService* const pref_service_;
  bool stopped_;
  const std::unique_ptr<base::ThreadChecker> thread_checker_;

  DISALLOW_COPY_AND_ASSIGN(CastService);
};

}  // namespace chromecast

#endif  // CHROMECAST_SERVICE_CAST_SERVICE_H_
