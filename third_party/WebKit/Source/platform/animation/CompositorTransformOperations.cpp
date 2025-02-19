// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/animation/CompositorTransformOperations.h"

#include "ui/gfx/transform.h"

namespace blink {

const cc::TransformOperations&
CompositorTransformOperations::AsCcTransformOperations() const {
  return transform_operations_;
}

cc::TransformOperations
CompositorTransformOperations::ReleaseCcTransformOperations() {
  return std::move(transform_operations_);
}

bool CompositorTransformOperations::CanBlendWith(
    const blink::CompositorTransformOperations& other) const {
  return transform_operations_.CanBlendWith(other.transform_operations_);
}

void CompositorTransformOperations::AppendTranslate(double x,
                                                    double y,
                                                    double z) {
  transform_operations_.AppendTranslate(x, y, z);
}

void CompositorTransformOperations::AppendRotate(double x,
                                                 double y,
                                                 double z,
                                                 double degrees) {
  transform_operations_.AppendRotate(x, y, z, degrees);
}

void CompositorTransformOperations::AppendScale(double x, double y, double z) {
  transform_operations_.AppendScale(x, y, z);
}

void CompositorTransformOperations::AppendSkew(double x, double y) {
  transform_operations_.AppendSkew(x, y);
}

void CompositorTransformOperations::AppendPerspective(double depth) {
  transform_operations_.AppendPerspective(depth);
}

void CompositorTransformOperations::AppendMatrix(const SkMatrix44& matrix) {
  gfx::Transform transform(gfx::Transform::kSkipInitialization);
  transform.matrix() = matrix;
  transform_operations_.AppendMatrix(transform);
}

void CompositorTransformOperations::AppendIdentity() {
  transform_operations_.AppendIdentity();
}

bool CompositorTransformOperations::IsIdentity() const {
  return transform_operations_.IsIdentity();
}

}  // namespace blink
