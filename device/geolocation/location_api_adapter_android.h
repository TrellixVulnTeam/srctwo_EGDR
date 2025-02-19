// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_GEOLOCATION_LOCATION_API_ADAPTER_ANDROID_H_
#define DEVICE_GEOLOCATION_LOCATION_API_ADAPTER_ANDROID_H_

#include "base/android/jni_weak_ref.h"
#include "base/android/scoped_java_ref.h"
#include "base/memory/ref_counted.h"
#include "base/memory/singleton.h"
#include "base/synchronization/lock.h"
#include "base/threading/thread_checker.h"

namespace base {
class SingleThreadTaskRunner;
}

namespace device {
struct Geoposition;

// Interacts with JNI and reports back to |on_geoposition_callback_|. This class
// creates a LocationProvider java object and listens for updates.
// The simplified flow is:
//   - GeolocationProvider runs in a Geolocation |task_runner_| and fetches
//     geolocation data from a LocationProvider.
//   - LocationApiAdapterAndroid calls via JNI and uses the main thread Looper
//     in the java side to listen for location updates. We then bounce these
//     updates to the Geolocation |task_runner_|.
//
// Note that LocationApiAdapterAndroid is a singleton and there's at most only
// one call to Start().
class LocationApiAdapterAndroid {
 public:
  using OnGeopositionCB = base::Callback<void(const Geoposition&)>;

  // Starts the underlying location provider, returns true if successful.
  // Called on |task_runner_|.
  bool Start(OnGeopositionCB on_geoposition_callback, bool high_accuracy);

  // Stops the underlying location provider. Called on |task_runner_|.
  void Stop();

  // Called by JNI on its thread looper.
  static void OnNewLocationAvailable(double latitude,
                                     double longitude,
                                     double time_stamp,
                                     bool has_altitude,
                                     double altitude,
                                     bool has_accuracy,
                                     double accuracy,
                                     bool has_heading,
                                     double heading,
                                     bool has_speed,
                                     double speed);
  static void OnNewErrorAvailable(JNIEnv* env, jstring message);

  // Returns our singleton.
  static LocationApiAdapterAndroid* GetInstance();

 private:
  friend struct base::DefaultSingletonTraits<LocationApiAdapterAndroid>;
  LocationApiAdapterAndroid();
  ~LocationApiAdapterAndroid();

  // Calls |on_geoposition_callback_| with the new location.
  void NotifyNewGeoposition(const Geoposition& geoposition);

  base::android::ScopedJavaGlobalRef<jobject> java_location_provider_adapter_;

  // Valid between Start() and Stop().
  OnGeopositionCB on_geoposition_callback_;

  const scoped_refptr<base::SingleThreadTaskRunner> task_runner_;

  base::ThreadChecker thread_checker_;
};

}  // namespace device

#endif  // DEVICE_GEOLOCATION_LOCATION_API_ADAPTER_ANDROID_H_
