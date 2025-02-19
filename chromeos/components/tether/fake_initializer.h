// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_COMPONENTS_TETHER_FAKE_INITIALIZER_H_
#define CHROMEOS_COMPONENTS_TETHER_FAKE_INITIALIZER_H_

#include "base/macros.h"
#include "chromeos/components/tether/initializer.h"

namespace chromeos {

namespace tether {

// Test double for Initializer.
class FakeInitializer : public Initializer {
 public:
  explicit FakeInitializer(bool has_asynchronous_shutdown);
  ~FakeInitializer() override;

  void set_has_asynchronous_shutdown(bool has_asynchronous_shutdown) {
    has_asynchronous_shutdown_ = has_asynchronous_shutdown;
  }

  // Should only be called when status() == SHUTTING_DOWN.
  void FinishAsynchronousShutdown();

  // Initializer:
  void RequestShutdown() override;

 private:
  bool has_asynchronous_shutdown_;

  DISALLOW_COPY_AND_ASSIGN(FakeInitializer);
};

}  // namespace tether

}  // namespace chromeos

#endif  // CHROMEOS_COMPONENTS_TETHER_FAKE_INITIALIZER_H_
