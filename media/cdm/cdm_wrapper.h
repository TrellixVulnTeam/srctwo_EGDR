// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_CDM_CDM_WRAPPER_H_
#define MEDIA_CDM_CDM_WRAPPER_H_

#include <stdint.h>

#include <string>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "media/cdm/api/content_decryption_module.h"
#include "media/cdm/supported_cdm_versions.h"

#if defined(USE_PPAPI_CDM_ADAPTER)
// When building the ppapi adapter do not include any non-trivial base/ headers.
#include "ppapi/cpp/logging.h"  // nogncheck
#define PLATFORM_DCHECK PP_DCHECK
#else
#include "base/logging.h"
#include "media/base/media_switches.h"  // nogncheck
#define PLATFORM_DCHECK DCHECK
#endif

namespace media {

namespace {

bool IsExperimentalCdmInterfaceSupported() {
#if defined(USE_PPAPI_CDM_ADAPTER)
#if defined(SUPPORT_EXPERIMENTAL_CDM_INTERFACE)
  return true;
#else
  return false;
#endif  // defined(SUPPORT_EXPERIMENTAL_CDM_INTERFACE)
#else
  return base::FeatureList::IsEnabled(media::kSupportExperimentalCdmInterface);
#endif  // defined(USE_PPAPI_CDM_ADAPTER)
}

}  // namespace

// Returns a pointer to the requested CDM upon success.
// Returns NULL if an error occurs or the requested |cdm_interface_version| or
// |key_system| is not supported or another error occurs.
// The caller should cast the returned pointer to the type matching
// |cdm_interface_version|.
// Caller retains ownership of arguments and must call Destroy() on the returned
// object.
typedef void* (*CreateCdmFunc)(int cdm_interface_version,
                               const char* key_system,
                               uint32_t key_system_size,
                               GetCdmHostFunc get_cdm_host_func,
                               void* user_data);

// CdmWrapper wraps different versions of ContentDecryptionModule interfaces and
// exposes a common interface to the caller.
//
// The caller should call CdmWrapper::Create() to create a CDM instance.
// CdmWrapper will first try to create a CDM instance that supports the latest
// CDM interface (ContentDecryptionModule). If such an instance cannot be
// created (e.g. an older CDM was loaded), CdmWrapper will try to create a CDM
// that supports an older version of CDM interface (e.g.
// ContentDecryptionModule_*). Internally CdmWrapper converts the CdmWrapper
// calls to corresponding ContentDecryptionModule calls.
//
// Since this file is highly templated and default implementations are short
// (just a shim layer in most cases), everything is done in this header file.
class CdmWrapper {
 public:
  static CdmWrapper* Create(CreateCdmFunc create_cdm_func,
                            const char* key_system,
                            uint32_t key_system_size,
                            GetCdmHostFunc get_cdm_host_func,
                            void* user_data);

  virtual ~CdmWrapper() {}

  // Returns the version of the CDM interface that the created CDM uses.
  virtual int GetInterfaceVersion() = 0;

  virtual void Initialize(bool allow_distinctive_identifier,
                          bool allow_persistent_state) = 0;
  virtual void SetServerCertificate(uint32_t promise_id,
                                    const uint8_t* server_certificate_data,
                                    uint32_t server_certificate_data_size) = 0;

  // Gets the key status for a policy that contains the |min_hdcp_version|.
  // Returns whether GetStatusForPolicy() is supported. If true, the CDM should
  // resolve or reject the promise. If false, the caller will reject the
  // promise.
  virtual bool GetStatusForPolicy(uint32_t promise_id,
                                  cdm::HdcpVersion min_hdcp_version)
      WARN_UNUSED_RESULT = 0;

  virtual void CreateSessionAndGenerateRequest(uint32_t promise_id,
                                               cdm::SessionType session_type,
                                               cdm::InitDataType init_data_type,
                                               const uint8_t* init_data,
                                               uint32_t init_data_size) = 0;
  virtual void LoadSession(uint32_t promise_id,
                           cdm::SessionType session_type,
                           const char* session_id,
                           uint32_t session_id_size) = 0;
  virtual void UpdateSession(uint32_t promise_id,
                             const char* session_id,
                             uint32_t session_id_size,
                             const uint8_t* response,
                             uint32_t response_size) = 0;
  virtual void CloseSession(uint32_t promise_id,
                            const char* session_id,
                            uint32_t session_id_size) = 0;
  virtual void RemoveSession(uint32_t promise_id,
                             const char* session_id,
                             uint32_t session_id_size) = 0;
  virtual void TimerExpired(void* context) = 0;
  virtual cdm::Status Decrypt(const cdm::InputBuffer& encrypted_buffer,
                              cdm::DecryptedBlock* decrypted_buffer) = 0;
  virtual cdm::Status InitializeAudioDecoder(
      const cdm::AudioDecoderConfig& audio_decoder_config) = 0;
  virtual cdm::Status InitializeVideoDecoder(
      const cdm::VideoDecoderConfig& video_decoder_config) = 0;
  virtual void DeinitializeDecoder(cdm::StreamType decoder_type) = 0;
  virtual void ResetDecoder(cdm::StreamType decoder_type) = 0;
  virtual cdm::Status DecryptAndDecodeFrame(
      const cdm::InputBuffer& encrypted_buffer,
      cdm::VideoFrame* video_frame) = 0;
  virtual cdm::Status DecryptAndDecodeSamples(
      const cdm::InputBuffer& encrypted_buffer,
      cdm::AudioFrames* audio_frames) = 0;
  virtual void OnPlatformChallengeResponse(
      const cdm::PlatformChallengeResponse& response) = 0;
  virtual void OnQueryOutputProtectionStatus(
      cdm::QueryResult result,
      uint32_t link_mask,
      uint32_t output_protection_mask) = 0;
  virtual void OnStorageId(const uint8_t* storage_id,
                           uint32_t storage_id_size) = 0;

 protected:
  CdmWrapper() {}

 private:
  DISALLOW_COPY_AND_ASSIGN(CdmWrapper);
};

// Template class that does the CdmWrapper -> CdmInterface conversion. Default
// implementations are provided. Any methods that need special treatment should
// be specialized.
template <class CdmInterface>
class CdmWrapperImpl : public CdmWrapper {
 public:
  static CdmWrapper* Create(CreateCdmFunc create_cdm_func,
                            const char* key_system,
                            uint32_t key_system_size,
                            GetCdmHostFunc get_cdm_host_func,
                            void* user_data) {
    void* cdm_instance =
        create_cdm_func(CdmInterface::kVersion, key_system, key_system_size,
                        get_cdm_host_func, user_data);
    if (!cdm_instance)
      return nullptr;

    return new CdmWrapperImpl<CdmInterface>(
        static_cast<CdmInterface*>(cdm_instance));
  }

  ~CdmWrapperImpl() override { cdm_->Destroy(); }

  int GetInterfaceVersion() override { return CdmInterface::kVersion; }

  void Initialize(bool allow_distinctive_identifier,
                  bool allow_persistent_state) override {
    cdm_->Initialize(allow_distinctive_identifier, allow_persistent_state);
  }

  void SetServerCertificate(uint32_t promise_id,
                            const uint8_t* server_certificate_data,
                            uint32_t server_certificate_data_size) override {
    cdm_->SetServerCertificate(promise_id, server_certificate_data,
                               server_certificate_data_size);
  }

  bool GetStatusForPolicy(uint32_t promise_id,
                          cdm::HdcpVersion min_hdcp_version) override {
    cdm::Policy policy;
    policy.min_hdcp_version = min_hdcp_version;
    cdm_->GetStatusForPolicy(promise_id, policy);
    return true;
  }

  void CreateSessionAndGenerateRequest(uint32_t promise_id,
                                       cdm::SessionType session_type,
                                       cdm::InitDataType init_data_type,
                                       const uint8_t* init_data,
                                       uint32_t init_data_size) override {
    cdm_->CreateSessionAndGenerateRequest(
        promise_id, session_type, init_data_type, init_data, init_data_size);
  }

  void LoadSession(uint32_t promise_id,
                   cdm::SessionType session_type,
                   const char* session_id,
                   uint32_t session_id_size) override {
    cdm_->LoadSession(promise_id, session_type, session_id, session_id_size);
  }

  void UpdateSession(uint32_t promise_id,
                     const char* session_id,
                     uint32_t session_id_size,
                     const uint8_t* response,
                     uint32_t response_size) override {
    cdm_->UpdateSession(promise_id, session_id, session_id_size, response,
                        response_size);
  }

  void CloseSession(uint32_t promise_id,
                    const char* session_id,
                    uint32_t session_id_size) override {
    cdm_->CloseSession(promise_id, session_id, session_id_size);
  }

  void RemoveSession(uint32_t promise_id,
                     const char* session_id,
                     uint32_t session_id_size) override {
    cdm_->RemoveSession(promise_id, session_id, session_id_size);
  }

  void TimerExpired(void* context) override { cdm_->TimerExpired(context); }

  cdm::Status Decrypt(const cdm::InputBuffer& encrypted_buffer,
                      cdm::DecryptedBlock* decrypted_buffer) override {
    return cdm_->Decrypt(encrypted_buffer, decrypted_buffer);
  }

  cdm::Status InitializeAudioDecoder(
      const cdm::AudioDecoderConfig& audio_decoder_config) override {
    return cdm_->InitializeAudioDecoder(audio_decoder_config);
  }

  cdm::Status InitializeVideoDecoder(
      const cdm::VideoDecoderConfig& video_decoder_config) override {
    return cdm_->InitializeVideoDecoder(video_decoder_config);
  }

  void DeinitializeDecoder(cdm::StreamType decoder_type) override {
    cdm_->DeinitializeDecoder(decoder_type);
  }

  void ResetDecoder(cdm::StreamType decoder_type) override {
    cdm_->ResetDecoder(decoder_type);
  }

  cdm::Status DecryptAndDecodeFrame(const cdm::InputBuffer& encrypted_buffer,
                                    cdm::VideoFrame* video_frame) override {
    return cdm_->DecryptAndDecodeFrame(encrypted_buffer, video_frame);
  }

  cdm::Status DecryptAndDecodeSamples(const cdm::InputBuffer& encrypted_buffer,
                                      cdm::AudioFrames* audio_frames) override {
    return cdm_->DecryptAndDecodeSamples(encrypted_buffer, audio_frames);
  }

  void OnPlatformChallengeResponse(
      const cdm::PlatformChallengeResponse& response) override {
    cdm_->OnPlatformChallengeResponse(response);
  }

  void OnQueryOutputProtectionStatus(cdm::QueryResult result,
                                     uint32_t link_mask,
                                     uint32_t output_protection_mask) override {
    cdm_->OnQueryOutputProtectionStatus(result, link_mask,
                                        output_protection_mask);
  }

  void OnStorageId(const uint8_t* storage_id, uint32_t storage_id_size) {
    cdm_->OnStorageId(storage_id, storage_id_size);
  }

 private:
  CdmWrapperImpl(CdmInterface* cdm) : cdm_(cdm) { PLATFORM_DCHECK(cdm_); }

  CdmInterface* cdm_;

  DISALLOW_COPY_AND_ASSIGN(CdmWrapperImpl);
};

// Overrides for cdm::Host_8 methods.
// TODO(jrummell): Remove when CDM_8 no longer supported.
// https://crbug.com/737296.

template <>
bool CdmWrapperImpl<cdm::ContentDecryptionModule_8>::GetStatusForPolicy(
    uint32_t /* promise_id */,
    cdm::HdcpVersion /* min_hdcp_version */) {
  return false;
}

template <>
void CdmWrapperImpl<cdm::ContentDecryptionModule_8>::OnStorageId(
    const uint8_t* storage_id,
    uint32_t storage_id_size) {}

CdmWrapper* CdmWrapper::Create(CreateCdmFunc create_cdm_func,
                               const char* key_system,
                               uint32_t key_system_size,
                               GetCdmHostFunc get_cdm_host_func,
                               void* user_data) {
  static_assert(cdm::ContentDecryptionModule::kVersion ==
                    cdm::ContentDecryptionModule_9::kVersion,
                "update the code below");

  // Ensure IsSupportedCdmInterfaceVersion() matches this implementation.
  // Always update this DCHECK when updating this function.
  // If this check fails, update this function and DCHECK or update
  // IsSupportedCdmInterfaceVersion().
  PLATFORM_DCHECK(!IsSupportedCdmInterfaceVersion(
                      cdm::ContentDecryptionModule_9::kVersion + 1) &&
                  IsSupportedCdmInterfaceVersion(
                      cdm::ContentDecryptionModule_9::kVersion) &&
                  IsSupportedCdmInterfaceVersion(
                      cdm::ContentDecryptionModule_8::kVersion) &&
                  !IsSupportedCdmInterfaceVersion(
                      cdm::ContentDecryptionModule_8::kVersion - 1));

  // Try to create the CDM using the latest CDM interface version.
  // This is only attempted if requested. For pepper plugins, this is done
  // at compile time. For mojo, it is done using a media feature setting.
  CdmWrapper* cdm_wrapper = nullptr;

  if (IsExperimentalCdmInterfaceSupported()) {
    cdm_wrapper = CdmWrapperImpl<cdm::ContentDecryptionModule_9>::Create(
        create_cdm_func, key_system, key_system_size, get_cdm_host_func,
        user_data);
  }

  // If |cdm_wrapper| is NULL, try to create the CDM using older supported
  // versions of the CDM interface here.
  if (!cdm_wrapper) {
    cdm_wrapper = CdmWrapperImpl<cdm::ContentDecryptionModule_8>::Create(
        create_cdm_func, key_system, key_system_size, get_cdm_host_func,
        user_data);
  }

  return cdm_wrapper;
}

// When updating the CdmAdapter, ensure you've updated the CdmWrapper to contain
// stub implementations for new or modified methods that the older CDM interface
// does not have.
// Also update supported_cdm_versions.h.
static_assert(cdm::ContentDecryptionModule::kVersion ==
                  cdm::ContentDecryptionModule_9::kVersion,
              "ensure cdm wrapper templates have old version support");

}  // namespace media

#undef PLATFORM_DCHECK

#endif  // MEDIA_CDM_CDM_WRAPPER_H_
