// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_CDM_PLATFORM_VERIFICATION_H_
#define MEDIA_CDM_PLATFORM_VERIFICATION_H_

#include <string>
#include <vector>

#include "base/callback.h"
#include "base/macros.h"
#include "media/base/media_export.h"

namespace media {

class MEDIA_EXPORT PlatformVerification {
 public:
  PlatformVerification() = default;
  virtual ~PlatformVerification() = default;

  using ChallengePlatformCB =
      base::OnceCallback<void(bool success,
                              const std::string& signed_data,
                              const std::string& signed_data_signature,
                              const std::string& platform_key_certificate)>;
  using StorageIdCB =
      base::OnceCallback<void(const std::vector<uint8_t>& storage_id)>;

  // Allows authorized services to verify that the underlying platform is
  // trusted. An example of a trusted platform is a Chrome OS device in
  // verified boot mode. This can be used for protected content playback.
  //
  // |service_id| is the service ID for the |challenge|. |challenge| is the
  // challenge data. |callback| will be called with the following values:
  // - |success|: whether the platform is successfully verified. If true/false
  //              the following 3 parameters should be non-empty/empty.
  // - |signed_data|: the data signed by the platform.
  // - |signed_data_signature|: the signature of the signed data block.
  // - |platform_key_certificate|: the device specific certificate for the
  //                               requested service.
  virtual void ChallengePlatform(const std::string& service_id,
                                 const std::string& challenge,
                                 ChallengePlatformCB callback) = 0;

  // Requests the device's Storage Id. |callback| will be called with the
  // following value:
  // - |storage_id|: The devices Storage Id. It may be the empty string if it
  //                 is not supported by the platform.
  virtual void GetStorageId(StorageIdCB callback) = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(PlatformVerification);
};

}  // namespace media

#endif  // MEDIA_CDM_PLATFORM_VERIFICATION_H_
