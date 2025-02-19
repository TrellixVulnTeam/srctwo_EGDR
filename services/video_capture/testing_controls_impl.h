// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_VIDEO_CAPTURE_TESTING_CONTROLS_IMPL_H_
#define SERVICES_VIDEO_CAPTURE_TESTING_CONTROLS_IMPL_H_

#include "services/service_manager/public/cpp/service_context_ref.h"
#include "services/video_capture/public/interfaces/testing_controls.mojom.h"

namespace video_capture {

class TestingControlsImpl : public mojom::TestingControls {
 public:
  TestingControlsImpl(
      std::unique_ptr<service_manager::ServiceContextRef> service_ref);
  ~TestingControlsImpl() override;

  // mojom::TestingControls implementation.
  void Crash() override;

 private:
  const std::unique_ptr<service_manager::ServiceContextRef> service_ref_;

  DISALLOW_COPY_AND_ASSIGN(TestingControlsImpl);
};

}  // namespace video_capture

#endif  // SERVICES_VIDEO_CAPTURE_TESTING_CONTROLS_IMPL_H_
