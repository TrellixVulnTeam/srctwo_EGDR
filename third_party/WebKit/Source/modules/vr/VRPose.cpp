// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "modules/vr/VRPose.h"

namespace blink {

namespace {

DOMFloat32Array* MojoArrayToFloat32Array(
    const WTF::Optional<WTF::Vector<float>>& vec) {
  if (!vec)
    return nullptr;

  return DOMFloat32Array::Create(&(vec.value().front()), vec.value().size());
}

}  // namespace

VRPose::VRPose() {}

void VRPose::SetPose(const device::mojom::blink::VRPosePtr& state) {
  if (state.is_null())
    return;

  orientation_ = MojoArrayToFloat32Array(state->orientation);
  position_ = MojoArrayToFloat32Array(state->position);
  angular_velocity_ = MojoArrayToFloat32Array(state->angularVelocity);
  linear_velocity_ = MojoArrayToFloat32Array(state->linearVelocity);
  angular_acceleration_ = MojoArrayToFloat32Array(state->angularAcceleration);
  linear_acceleration_ = MojoArrayToFloat32Array(state->linearAcceleration);
}

DEFINE_TRACE(VRPose) {
  visitor->Trace(orientation_);
  visitor->Trace(position_);
  visitor->Trace(angular_velocity_);
  visitor->Trace(linear_velocity_);
  visitor->Trace(angular_acceleration_);
  visitor->Trace(linear_acceleration_);
}

}  // namespace blink
