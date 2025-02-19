// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_GPU_DATA_MANAGER_OBSERVER_H_
#define CONTENT_PUBLIC_BROWSER_GPU_DATA_MANAGER_OBSERVER_H_

#include "base/process/kill.h"
#include "content/common/content_export.h"
#include "content/public/common/three_d_api_types.h"

class GURL;

namespace content {

// Observers can register themselves via GpuDataManager::AddObserver, and
// can un-register with GpuDataManager::RemoveObserver.
class CONTENT_EXPORT GpuDataManagerObserver {
 public:
  // Called for any observers whenever there is a GPU info update.
  virtual void OnGpuInfoUpdate() {}

  // Indicates that client 3D APIs (Pepper 3D, WebGL) were just blocked on the
  // given page, specifically because the GPU was reset recently.
  virtual void DidBlock3DAPIs(const GURL& top_origin_url,
                              int render_process_id,
                              int render_frame_id,
                              ThreeDAPIType requester) {}

  // Called for any observer when the GPU process crashed.
  virtual void OnGpuProcessCrashed(base::TerminationStatus exit_code) {}

 protected:
  virtual ~GpuDataManagerObserver() {}
};

};  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_GPU_DATA_MANAGER_OBSERVER_H_
