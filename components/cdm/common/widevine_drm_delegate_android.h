// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_CDM_COMMON_WIDEVINE_DRM_DELEGATE_ANDROID_H_
#define COMPONENTS_CDM_COMMON_WIDEVINE_DRM_DELEGATE_ANDROID_H_

#include <stdint.h>

#include "base/macros.h"
#include "media/base/android/media_drm_bridge_delegate.h"

namespace cdm {

class WidevineDrmDelegateAndroid : public media::MediaDrmBridgeDelegate {
 public:
  WidevineDrmDelegateAndroid();
  ~WidevineDrmDelegateAndroid() override;

  // media::MediaDrmBridgeDelegate implementation:
  const std::vector<uint8_t> GetUUID() const override;
  bool OnCreateSession(
      const media::EmeInitDataType init_data_type,
      const std::vector<uint8_t>& init_data,
      std::vector<uint8_t>* init_data_out,
      std::vector<std::string>* optional_parameters_out) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(WidevineDrmDelegateAndroid);
};

}  // namespace cdm

#endif  // COMPONENTS_CDM_COMMON_WIDEVINE_DRM_DELEGATE_ANDROID_H_
