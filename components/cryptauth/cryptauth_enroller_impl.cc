// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/cryptauth/cryptauth_enroller_impl.h"

#include <utility>

#include "base/bind.h"
#include "components/cryptauth/cryptauth_client_impl.h"
#include "components/cryptauth/cryptauth_enrollment_utils.h"
#include "components/cryptauth/secure_message_delegate.h"
#include "components/proximity_auth/logging/logging.h"
#include "crypto/sha2.h"

namespace cryptauth {

namespace {

// A successful SetupEnrollment or FinishEnrollment response should contain this
// status string.
const char kResponseStatusOk[] = "ok";

// The name of the "gcmV1" protocol that the enrolling device supports.
const char kSupportedEnrollmentTypeGcmV1[] = "gcmV1";

// The version field of the GcmMetadata message.
const int kGCMMetadataVersion = 1;

// Returns true if |device_info| contains the required fields for enrollment.
bool ValidateDeviceInfo(const GcmDeviceInfo& device_info) {
  if (!device_info.has_long_device_id()) {
    PA_LOG(ERROR) << "Expected long_device_id field in GcmDeviceInfo.";
    return false;
  }

  if (!device_info.has_device_type()) {
    PA_LOG(ERROR) << "Expected device_type field in GcmDeviceInfo.";
    return false;
  }

  return true;
}

// Creates the public metadata to put in the SecureMessage that is sent to the
// server with the FinishEnrollment request.
std::string CreateEnrollmentPublicMetadata() {
  GcmMetadata metadata;
  metadata.set_version(kGCMMetadataVersion);
  metadata.set_type(MessageType::ENROLLMENT);
  return metadata.SerializeAsString();
}

}  // namespace

CryptAuthEnrollerImpl::CryptAuthEnrollerImpl(
    std::unique_ptr<CryptAuthClientFactory> client_factory,
    std::unique_ptr<SecureMessageDelegate> secure_message_delegate)
    : client_factory_(std::move(client_factory)),
      secure_message_delegate_(std::move(secure_message_delegate)),
      weak_ptr_factory_(this) {}

CryptAuthEnrollerImpl::~CryptAuthEnrollerImpl() {
}

void CryptAuthEnrollerImpl::Enroll(
    const std::string& user_public_key,
    const std::string& user_private_key,
    const GcmDeviceInfo& device_info,
    InvocationReason invocation_reason,
    const EnrollmentFinishedCallback& callback) {
  if (!callback_.is_null()) {
    PA_LOG(ERROR) << "Enroll() already called. Do not reuse.";
    callback.Run(false);
    return;
  }

  user_public_key_ = user_public_key;
  user_private_key_ = user_private_key;
  device_info_ = device_info;
  invocation_reason_ = invocation_reason;
  callback_ = callback;

  if (!ValidateDeviceInfo(device_info)) {
    callback.Run(false);
    return;
  }

  secure_message_delegate_->GenerateKeyPair(
      base::Bind(&CryptAuthEnrollerImpl::OnKeyPairGenerated,
                 weak_ptr_factory_.GetWeakPtr()));
}

void CryptAuthEnrollerImpl::OnKeyPairGenerated(const std::string& public_key,
                                               const std::string& private_key) {
  PA_LOG(INFO) << "Ephemeral key pair generated, calling SetupEnrollment API.";
  session_public_key_ = public_key;
  session_private_key_ = private_key;

  cryptauth_client_ = client_factory_->CreateInstance();
  SetupEnrollmentRequest request;
  request.add_types(kSupportedEnrollmentTypeGcmV1);
  request.set_invocation_reason(invocation_reason_);
  cryptauth_client_->SetupEnrollment(
      request, base::Bind(&CryptAuthEnrollerImpl::OnSetupEnrollmentSuccess,
                          weak_ptr_factory_.GetWeakPtr()),
      base::Bind(&CryptAuthEnrollerImpl::OnSetupEnrollmentFailure,
                 weak_ptr_factory_.GetWeakPtr()));
}

void CryptAuthEnrollerImpl::OnSetupEnrollmentSuccess(
    const SetupEnrollmentResponse& response) {
  if (response.status() != kResponseStatusOk) {
    PA_LOG(WARNING) << "Unexpected status for SetupEnrollment: "
                    << response.status();
    callback_.Run(false);
    return;
  }

  if (response.infos_size() == 0) {
    PA_LOG(ERROR) << "No response info returned by server for SetupEnrollment";
    callback_.Run(false);
    return;
  }

  PA_LOG(INFO) << "SetupEnrollment request succeeded: deriving symmetric key.";
  setup_info_ = response.infos(0);
  device_info_.set_enrollment_session_id(setup_info_.enrollment_session_id());

  secure_message_delegate_->DeriveKey(
      session_private_key_, setup_info_.server_ephemeral_key(),
      base::Bind(&CryptAuthEnrollerImpl::OnKeyDerived,
                 weak_ptr_factory_.GetWeakPtr()));
}

void CryptAuthEnrollerImpl::OnSetupEnrollmentFailure(const std::string& error) {
  PA_LOG(WARNING) << "SetupEnrollment API failed with error: " << error;
  callback_.Run(false);
}

void CryptAuthEnrollerImpl::OnKeyDerived(const std::string& symmetric_key) {
  PA_LOG(INFO) << "Derived symmetric key, "
               << "encrypting enrollment data for upload.";

  // Make sure we're enrolling the same public key used below to sign the
  // secure message.
  device_info_.set_user_public_key(user_public_key_);
  device_info_.set_key_handle(user_public_key_);

  // Hash the symmetric key and add it to the |device_info_| to be uploaded.
  device_info_.set_device_master_key_hash(
      crypto::SHA256HashString(symmetric_key));

  // The server verifies that the access token set here and in the header
  // of the FinishEnrollment() request are the same.
  device_info_.set_oauth_token(cryptauth_client_->GetAccessTokenUsed());
  PA_LOG(INFO) << "Using access token: " << device_info_.oauth_token();

  symmetric_key_ = symmetric_key;
  SecureMessageDelegate::CreateOptions options;
  options.encryption_scheme = securemessage::NONE;
  options.signature_scheme = securemessage::ECDSA_P256_SHA256;
  options.verification_key_id = user_public_key_;

  // The inner message contains the signed device information that will be
  // sent to CryptAuth.
  secure_message_delegate_->CreateSecureMessage(
      device_info_.SerializeAsString(), user_private_key_, options,
      base::Bind(&CryptAuthEnrollerImpl::OnInnerSecureMessageCreated,
                 weak_ptr_factory_.GetWeakPtr()));
}

void CryptAuthEnrollerImpl::OnInnerSecureMessageCreated(
    const std::string& inner_message) {
  if (inner_message.empty()) {
    PA_LOG(ERROR) << "Error creating inner message";
    callback_.Run(false);
    return;
  }

  SecureMessageDelegate::CreateOptions options;
  options.encryption_scheme = securemessage::AES_256_CBC;
  options.signature_scheme = securemessage::HMAC_SHA256;
  options.public_metadata = CreateEnrollmentPublicMetadata();

  // The outer message encrypts and signs the inner message with the derived
  // symmetric session key.
  secure_message_delegate_->CreateSecureMessage(
      inner_message, symmetric_key_, options,
      base::Bind(&CryptAuthEnrollerImpl::OnOuterSecureMessageCreated,
                 weak_ptr_factory_.GetWeakPtr()));
}

void CryptAuthEnrollerImpl::OnOuterSecureMessageCreated(
    const std::string& outer_message) {
  PA_LOG(INFO) << "SecureMessage created, calling FinishEnrollment API.";

  FinishEnrollmentRequest request;
  request.set_enrollment_session_id(setup_info_.enrollment_session_id());
  request.set_enrollment_message(outer_message);
  request.set_device_ephemeral_key(session_public_key_);
  request.set_invocation_reason(invocation_reason_);

  cryptauth_client_ = client_factory_->CreateInstance();
  cryptauth_client_->FinishEnrollment(
      request, base::Bind(&CryptAuthEnrollerImpl::OnFinishEnrollmentSuccess,
                          weak_ptr_factory_.GetWeakPtr()),
      base::Bind(&CryptAuthEnrollerImpl::OnFinishEnrollmentFailure,
                 weak_ptr_factory_.GetWeakPtr()));
}

void CryptAuthEnrollerImpl::OnFinishEnrollmentSuccess(
    const FinishEnrollmentResponse& response) {
  if (response.status() != kResponseStatusOk) {
    PA_LOG(WARNING) << "Unexpected status for FinishEnrollment: "
                    << response.status();
    callback_.Run(false);
  } else {
    callback_.Run(true);
  }
}

void CryptAuthEnrollerImpl::OnFinishEnrollmentFailure(
    const std::string& error) {
  PA_LOG(WARNING) << "FinishEnrollment API failed with error: " << error;
  callback_.Run(false);
}

}  // namespace cryptauth
