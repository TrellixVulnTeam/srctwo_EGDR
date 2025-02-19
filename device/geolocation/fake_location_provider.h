// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_GEOLOCATION_FAKE_LOCATION_PROVIDER_H_
#define DEVICE_GEOLOCATION_FAKE_LOCATION_PROVIDER_H_

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/thread.h"
#include "device/geolocation/geoposition.h"
#include "device/geolocation/location_provider.h"

namespace device {

// Fake implementation of a location provider for testing.
class FakeLocationProvider : public LocationProvider {
 public:
  enum State { STOPPED, LOW_ACCURACY, HIGH_ACCURACY } state_ = STOPPED;

  FakeLocationProvider();
  ~FakeLocationProvider() override;

  // Updates listeners with the new position.
  void HandlePositionChanged(const Geoposition& position);

  State state() const { return state_; }
  bool is_permission_granted() const { return is_permission_granted_; }

  // LocationProvider implementation.
  void SetUpdateCallback(
      const LocationProviderUpdateCallback& callback) override;
  bool StartProvider(bool high_accuracy) override;
  void StopProvider() override;
  const Geoposition& GetPosition() override;
  void OnPermissionGranted() override;

  scoped_refptr<base::SingleThreadTaskRunner> provider_task_runner_;

 private:
  bool is_permission_granted_ = false;
  Geoposition position_;
  LocationProviderUpdateCallback callback_;

  DISALLOW_COPY_AND_ASSIGN(FakeLocationProvider);
};

}  // namespace device

#endif  // DEVICE_GEOLOCATION_FAKE_LOCATION_PROVIDER_H_
