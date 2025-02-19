// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/device/generic_sensor/orientation_quaternion_fusion_algorithm_using_euler_angles.h"

#include <cmath>

#include "base/logging.h"
#include "services/device/generic_sensor/orientation_util.h"
#include "services/device/generic_sensor/platform_sensor_fusion.h"

namespace device {

namespace {

void ComputeQuaternionFromEulerAngles(double alpha_in_degrees,
                                      double beta_in_degrees,
                                      double gamma_in_degrees,
                                      double* x,
                                      double* y,
                                      double* z,
                                      double* w) {
  double alpha_in_radians = device::kDegToRad * alpha_in_degrees;
  double beta_in_radians = device::kDegToRad * beta_in_degrees;
  double gamma_in_radians = device::kDegToRad * gamma_in_degrees;

  double cx = std::cos(beta_in_radians / 2);
  double cy = std::cos(gamma_in_radians / 2);
  double cz = std::cos(alpha_in_radians / 2);
  double sx = std::sin(beta_in_radians / 2);
  double sy = std::sin(gamma_in_radians / 2);
  double sz = std::sin(alpha_in_radians / 2);

  *x = sx * cy * cz - cx * sy * sz;
  *y = cx * sy * cz + sx * cy * sz;
  *z = cx * cy * sz + sx * sy * cz;
  *w = cx * cy * cz - sx * sy * sz;
}

constexpr mojom::SensorType GetFusedType(bool absolute) {
  return absolute ? mojom::SensorType::ABSOLUTE_ORIENTATION_QUATERNION
                  : mojom::SensorType::RELATIVE_ORIENTATION_QUATERNION;
}

constexpr mojom::SensorType GetSourceType(bool absolute) {
  return absolute ? mojom::SensorType::ABSOLUTE_ORIENTATION_EULER_ANGLES
                  : mojom::SensorType::RELATIVE_ORIENTATION_EULER_ANGLES;
}

}  // namespace

OrientationQuaternionFusionAlgorithmUsingEulerAngles::
    OrientationQuaternionFusionAlgorithmUsingEulerAngles(bool absolute)
    : PlatformSensorFusionAlgorithm(GetFusedType(absolute),
                                    {GetSourceType(absolute)}) {}

OrientationQuaternionFusionAlgorithmUsingEulerAngles::
    ~OrientationQuaternionFusionAlgorithmUsingEulerAngles() = default;

bool OrientationQuaternionFusionAlgorithmUsingEulerAngles::GetFusedDataInternal(
    mojom::SensorType which_sensor_changed,
    SensorReading* fused_reading) {
  // Transform the *_ORIENTATION_EULER_ANGLES values to
  // *_ORIENTATION_QUATERNION.
  DCHECK(fusion_sensor_);

  SensorReading reading;
  if (!fusion_sensor_->GetSourceReading(which_sensor_changed, &reading))
    return false;

  ComputeQuaternionFromEulerAngles(
      reading.orientation_euler.z /* alpha_in_degrees */,
      reading.orientation_euler.x /* beta_in_degrees */,
      reading.orientation_euler.y /* gamma_in_degrees */,
      &fused_reading->orientation_quat.x.value(),
      &fused_reading->orientation_quat.y.value(),
      &fused_reading->orientation_quat.z.value(),
      &fused_reading->orientation_quat.w.value());

  return true;
}

}  // namespace device
