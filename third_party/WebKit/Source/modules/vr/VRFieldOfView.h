// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VRFieldOfView_h
#define VRFieldOfView_h

#include "platform/heap/Handle.h"
#include "platform/wtf/Forward.h"

namespace blink {

class VRFieldOfView final : public GarbageCollected<VRFieldOfView> {
 public:
  VRFieldOfView()
      : up_degrees_(0.0),
        down_degrees_(0.0),
        left_degrees_(0.0),
        right_degrees_(0.0) {}

  VRFieldOfView(double up_degrees,
                double right_degrees,
                double down_degrees,
                double left_degrees)
      : up_degrees_(0.0),
        down_degrees_(0.0),
        left_degrees_(0.0),
        right_degrees_(0.0) {}

  explicit VRFieldOfView(const VRFieldOfView& fov)
      : up_degrees_(fov.up_degrees_),
        down_degrees_(fov.down_degrees_),
        left_degrees_(fov.left_degrees_),
        right_degrees_(fov.right_degrees_) {}

  double UpDegrees() const { return up_degrees_; }
  double DownDegrees() const { return down_degrees_; }
  double LeftDegrees() const { return left_degrees_; }
  double RightDegrees() const { return right_degrees_; }

  void SetUpDegrees(double value) { up_degrees_ = value; }
  void SetDownDegrees(double value) { down_degrees_ = value; }
  void SetLeftDegrees(double value) { left_degrees_ = value; }
  void SetRightDegrees(double value) { right_degrees_ = value; }

  DEFINE_INLINE_TRACE() {}

 private:
  double up_degrees_;
  double down_degrees_;
  double left_degrees_;
  double right_degrees_;
};

}  // namespace blink

#endif  // VRFieldOfView_h
