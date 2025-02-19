// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_VIZ_HOST_HOST_FRAME_SINK_CLIENT_H_
#define COMPONENTS_VIZ_HOST_HOST_FRAME_SINK_CLIENT_H_

namespace viz {

class SurfaceInfo;

class HostFrameSinkClient {
 public:
  // Called when a CompositorFrame with a new SurfaceId activates for the first
  // time.
  virtual void OnFirstSurfaceActivation(const SurfaceInfo& surface_info) = 0;

 protected:
  virtual ~HostFrameSinkClient() {}
};

}  // namespace viz

#endif  // COMPONENTS_VIZ_HOST_HOST_FRAME_SINK_CLIENT_H_
