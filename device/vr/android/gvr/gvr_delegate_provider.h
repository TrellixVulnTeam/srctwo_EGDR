// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_VR_ANDROID_GVR_DELEGATE_PROVIDER_H_
#define DEVICE_VR_ANDROID_GVR_DELEGATE_PROVIDER_H_

#include "base/macros.h"
#include "device/vr/android/gvr/gvr_device_provider.h"
#include "device/vr/vr_export.h"
#include "device/vr/vr_service.mojom.h"

namespace gvr {
class GvrApi;
}

namespace device {

class VRDisplayImpl;

class DEVICE_VR_EXPORT GvrDelegateProvider {
 public:
  GvrDelegateProvider() = default;
  virtual void SetDeviceId(unsigned int device_id) = 0;
  virtual void RequestWebVRPresent(
      mojom::VRSubmitFrameClientPtr submit_client,
      mojom::VRPresentationProviderRequest request,
      const base::Callback<void(bool)>& callback) = 0;
  virtual void ExitWebVRPresent() = 0;
  virtual void OnDisplayAdded(VRDisplayImpl* display) = 0;
  virtual void OnDisplayRemoved(VRDisplayImpl* display) = 0;
  virtual void OnListeningForActivateChanged(VRDisplayImpl* display) = 0;
  // TODO(mthiesse): Remove the GvrApi from these calls.
  virtual void CreateVRDisplayInfo(
      gvr::GvrApi* gvr_api,
      const base::Callback<void(mojom::VRDisplayInfoPtr)>& callback,
      uint32_t device_id) = 0;
  virtual void GetNextMagicWindowPose(
      gvr::GvrApi* gvr_api,
      VRDisplayImpl* display,
      mojom::VRDisplay::GetNextMagicWindowPoseCallback callback) = 0;

 protected:
  virtual ~GvrDelegateProvider() {}

 private:
  DISALLOW_COPY_AND_ASSIGN(GvrDelegateProvider);
};

}  // namespace device

#endif  // DEVICE_VR_ANDROID_GVR_DELEGATE_PROVIDER_H_
