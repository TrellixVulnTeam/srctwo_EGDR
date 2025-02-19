// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMECAST_MEDIA_CDM_CAST_CDM_CONTEXT_H_
#define CHROMECAST_MEDIA_CDM_CAST_CDM_CONTEXT_H_

#include <memory>
#include <string>

#include "chromecast/public/media/cast_key_status.h"
#include "media/base/cdm_context.h"

namespace chromecast {
namespace media {

class DecryptContextImpl;
struct EncryptionScheme;

// CdmContext implementation + some extra APIs needed by CastRenderer.
class CastCdmContext : public ::media::CdmContext {
 public:
  // ::media::CdmContext implementation.
  ::media::Decryptor* GetDecryptor() override;
  int GetCdmId() const override;

  // Register a player with this CDM. |new_key_cb| will be called when a new
  // key is available. |cdm_unset_cb| will be called when the CDM is destroyed.
  virtual int RegisterPlayer(const base::Closure& new_key_cb,
                             const base::Closure& cdm_unset_cb) = 0;

  // Unregiester a player with this CDM. |registration_id| should be the id
  // returned by RegisterPlayer().
  virtual void UnregisterPlayer(int registration_id) = 0;

  // Returns the decryption context needed to decrypt frames encrypted with
  // |key_id|. Returns null if |key_id| is not available.
  virtual std::unique_ptr<DecryptContextImpl> GetDecryptContext(
      const std::string& key_id,
      const EncryptionScheme& encryption_scheme) = 0;

  // Notifies that key status has changed (e.g. if expiry is detected by
  // hardware decoder).
  virtual void SetKeyStatus(const std::string& key_id,
                            CastKeyStatus key_status,
                            uint32_t system_code) = 0;

  // Notifies of current decoded video resolution (used for licence policy
  // enforcement).
  virtual void SetVideoResolution(int width, int height) = 0;
};

}  // namespace media
}  // namespace chromecast

#endif  // CHROMECAST_MEDIA_CDM_CAST_CDM_CONTEXT_H_
