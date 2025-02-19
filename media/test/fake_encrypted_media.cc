// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/test/fake_encrypted_media.h"

#include "base/bind.h"
#include "media/base/cdm_key_information.h"
#include "media/cdm/aes_decryptor.h"

namespace media {

FakeEncryptedMedia::TestCdmContext::TestCdmContext(Decryptor* decryptor)
    : decryptor_(decryptor) {}

Decryptor* FakeEncryptedMedia::TestCdmContext::GetDecryptor() {
  return decryptor_;
}

int FakeEncryptedMedia::TestCdmContext::GetCdmId() const {
  return kInvalidCdmId;
}

FakeEncryptedMedia::FakeEncryptedMedia(AppBase* app)
    : decryptor_(new AesDecryptor(
          base::Bind(&FakeEncryptedMedia::OnSessionMessage,
                     base::Unretained(this)),
          base::Bind(&FakeEncryptedMedia::OnSessionClosed,
                     base::Unretained(this)),
          base::Bind(&FakeEncryptedMedia::OnSessionKeysChange,
                     base::Unretained(this)),
          base::Bind(&FakeEncryptedMedia::OnSessionExpirationUpdate,
                     base::Unretained(this)))),
      cdm_context_(decryptor_.get()),
      app_(app) {}

FakeEncryptedMedia::~FakeEncryptedMedia() {}

CdmContext* FakeEncryptedMedia::GetCdmContext() {
  return &cdm_context_;
}

// Callbacks for firing session events. Delegate to |app_|.
void FakeEncryptedMedia::OnSessionMessage(const std::string& session_id,
                                          CdmMessageType message_type,
                                          const std::vector<uint8_t>& message) {
  app_->OnSessionMessage(session_id, message_type, message, decryptor_.get());
}

void FakeEncryptedMedia::OnSessionClosed(const std::string& session_id) {
  app_->OnSessionClosed(session_id);
}

void FakeEncryptedMedia::OnSessionKeysChange(const std::string& session_id,
                                             bool has_additional_usable_key,
                                             CdmKeysInfo keys_info) {
  app_->OnSessionKeysChange(session_id, has_additional_usable_key,
                            std::move(keys_info));
}

void FakeEncryptedMedia::OnSessionExpirationUpdate(
    const std::string& session_id,
    base::Time new_expiry_time) {
  app_->OnSessionExpirationUpdate(session_id, new_expiry_time);
}

void FakeEncryptedMedia::OnEncryptedMediaInitData(
    EmeInitDataType init_data_type,
    const std::vector<uint8_t>& init_data) {
  app_->OnEncryptedMediaInitData(init_data_type, init_data, decryptor_.get());
}

}  // namespace media
