// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/device/generic_sensor/orientation_quaternion_fusion_algorithm_using_euler_angles.h"
#include "base/memory/ptr_util.h"
#include "base/memory/ref_counted.h"
#include "services/device/device_service_test_base.h"
#include "services/device/generic_sensor/fake_platform_sensor_fusion.h"
#include "services/device/generic_sensor/orientation_test_data.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace device {

namespace {

constexpr double kEpsilon = 1e-8;

}  // namespace

class OrientationQuaternionFusionAlgorithmUsingEulerAnglesTest
    : public DeviceServiceTestBase {
 public:
  OrientationQuaternionFusionAlgorithmUsingEulerAnglesTest() {
    auto fusion_algorithm =
        base::MakeUnique<OrientationQuaternionFusionAlgorithmUsingEulerAngles>(
            true /* absolute */);
    fusion_algorithm_ = fusion_algorithm.get();
    fake_fusion_sensor_ = base::MakeRefCounted<FakePlatformSensorFusion>(
        std::move(fusion_algorithm));
    fusion_algorithm_->set_fusion_sensor(fake_fusion_sensor_.get());
  }

 protected:
  scoped_refptr<FakePlatformSensorFusion> fake_fusion_sensor_;
  OrientationQuaternionFusionAlgorithmUsingEulerAngles* fusion_algorithm_;
};

TEST_F(OrientationQuaternionFusionAlgorithmUsingEulerAnglesTest,
       ReadSourceSensorFailed) {
  ASSERT_EQ(1UL, fusion_algorithm_->source_types().size());

  mojom::SensorType source_type = fusion_algorithm_->source_types()[0];
  SensorReading reading;
  SensorReading fused_reading;
  fake_fusion_sensor_->SetSensorReading(source_type, reading,
                                        false /* sensor_reading_success */);
  EXPECT_FALSE(fusion_algorithm_->GetFusedData(source_type, &fused_reading));
}

TEST_F(OrientationQuaternionFusionAlgorithmUsingEulerAnglesTest,
       CheckSampleValues) {
  ASSERT_EQ(euler_angles_in_degrees_test_values.size(),
            quaternions_test_values.size());
  ASSERT_EQ(1UL, fusion_algorithm_->source_types().size());

  mojom::SensorType source_type = fusion_algorithm_->source_types()[0];
  SensorReading reading;
  SensorReading fused_reading;

  for (size_t i = 0; i < euler_angles_in_degrees_test_values.size(); ++i) {
    // alpha
    reading.orientation_euler.z = euler_angles_in_degrees_test_values[i][0];
    // beta
    reading.orientation_euler.x = euler_angles_in_degrees_test_values[i][1];
    // gamma
    reading.orientation_euler.y = euler_angles_in_degrees_test_values[i][2];

    fake_fusion_sensor_->SetSensorReading(source_type, reading,
                                          true /* sensor_reading_success */);
    EXPECT_TRUE(fusion_algorithm_->GetFusedData(source_type, &fused_reading));

    double x = fused_reading.orientation_quat.x;
    double y = fused_reading.orientation_quat.y;
    double z = fused_reading.orientation_quat.z;
    double w = fused_reading.orientation_quat.w;

    EXPECT_NEAR(quaternions_test_values[i][0], x, kEpsilon)
        << "on test value " << i;
    EXPECT_NEAR(quaternions_test_values[i][1], y, kEpsilon)
        << "on test value " << i;
    EXPECT_NEAR(quaternions_test_values[i][2], z, kEpsilon)
        << "on test value " << i;
    EXPECT_NEAR(quaternions_test_values[i][3], w, kEpsilon)
        << "on test value " << i;

    EXPECT_NEAR(1.0, x * x + y * y + z * z + w * w, kEpsilon)
        << "on test value " << i;
  }
}

}  // namespace device
