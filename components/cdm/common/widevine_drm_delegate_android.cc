// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/cdm/common/widevine_drm_delegate_android.h"

#include "base/macros.h"
#include "media/cdm/cenc_utils.h"
#include "media/media_features.h"

namespace cdm {

namespace {

const uint8_t kWidevineUuid[16] = {
    0xED, 0xEF, 0x8B, 0xA9, 0x79, 0xD6, 0x4A, 0xCE,  //
    0xA3, 0xC8, 0x27, 0xDC, 0xD5, 0x1D, 0x21, 0xED};

}  // namespace

WidevineDrmDelegateAndroid::WidevineDrmDelegateAndroid() {}

WidevineDrmDelegateAndroid::~WidevineDrmDelegateAndroid() {}

const std::vector<uint8_t> WidevineDrmDelegateAndroid::GetUUID() const {
  return std::vector<uint8_t>(kWidevineUuid,
                              kWidevineUuid + arraysize(kWidevineUuid));
}

bool WidevineDrmDelegateAndroid::OnCreateSession(
    const media::EmeInitDataType init_data_type,
    const std::vector<uint8_t>& init_data,
    std::vector<uint8_t>* init_data_out,
    std::vector<std::string>* /* optional_parameters_out */) {
  if (init_data_type != media::EmeInitDataType::CENC)
    return true;

#if BUILDFLAG(USE_PROPRIETARY_CODECS)
  // Widevine MediaDrm plugin only accepts the "data" part of the PSSH box as
  // the init data when using MP4 container.
  return media::GetPsshData(init_data, GetUUID(), init_data_out);
#else
  return false;
#endif
}

}  // namespace cdm
