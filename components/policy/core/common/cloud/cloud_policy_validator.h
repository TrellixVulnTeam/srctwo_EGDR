// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_POLICY_CORE_COMMON_CLOUD_CLOUD_POLICY_VALIDATOR_H_
#define COMPONENTS_POLICY_CORE_COMMON_CLOUD_CLOUD_POLICY_VALIDATOR_H_

#include <stdint.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/bind.h"
#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/memory/ref_counted.h"
#include "base/sequenced_task_runner.h"
#include "base/time/time.h"
#include "build/build_config.h"
#include "components/policy/policy_export.h"
#include "components/policy/proto/cloud_policy.pb.h"

#if !defined(OS_ANDROID) && !defined(OS_IOS)
#include "components/policy/proto/chrome_extension_policy.pb.h"
#endif

namespace base {
class SingleThreadTaskRunner;
}

namespace google {
namespace protobuf {
class MessageLite;
}
}

namespace enterprise_management {
class PolicyData;
class PolicyFetchResponse;
}

namespace policy {

// Helper class that implements the gory details of validating a policy blob.
// Since signature checks are expensive, validation can happen on a background
// thread. The pattern is to create a validator, configure its behavior through
// the ValidateXYZ() functions, and then call StartValidation(). Alternatively,
// RunValidation() can be used to perform validation on the current thread.
class POLICY_EXPORT CloudPolicyValidatorBase {
 public:
  // Validation result codes. These values are also used for UMA histograms by
  // UserCloudPolicyStoreChromeOS and must stay stable - new elements should
  // be added at the end before VALIDATION_STATUS_SIZE. Also update the
  // associated enum definition in histograms.xml.
  enum Status {
    // Indicates successful validation.
    VALIDATION_OK,
    // Bad signature on the initial key.
    VALIDATION_BAD_INITIAL_SIGNATURE,
    // Bad signature.
    VALIDATION_BAD_SIGNATURE,
    // Policy blob contains error code.
    VALIDATION_ERROR_CODE_PRESENT,
    // Policy payload failed to decode.
    VALIDATION_PAYLOAD_PARSE_ERROR,
    // Unexpected policy type.
    VALIDATION_WRONG_POLICY_TYPE,
    // Unexpected settings entity id.
    VALIDATION_WRONG_SETTINGS_ENTITY_ID,
    // Timestamp is missing or is older than expected.
    VALIDATION_BAD_TIMESTAMP,
    // DM token is empty or doesn't match.
    VALIDATION_BAD_DM_TOKEN,
    // Device id is empty or doesn't match.
    VALIDATION_BAD_DEVICE_ID,
    // Username doesn't match.
    VALIDATION_BAD_USERNAME,
    // Policy payload protobuf parse error.
    VALIDATION_POLICY_PARSE_ERROR,
    // Policy key signature could not be verified using the hard-coded
    // verification key.
    VALIDATION_BAD_KEY_VERIFICATION_SIGNATURE,
    VALIDATION_STATUS_SIZE  // MUST BE LAST
  };

  enum ValidateDMTokenOption {
    // The DM token from policy must match the expected DM token unless the
    // expected DM token is empty. In addition, the DM token from policy must
    // not be empty.
    DM_TOKEN_REQUIRED,

    // The DM token from policy must match the expected DM token unless the
    // expected DM token is empty.
    DM_TOKEN_NOT_REQUIRED,
  };

  enum ValidateDeviceIdOption {
    // The device id from policy must match the expected device id unless the
    // expected device id is empty. In addition, the device id from policy must
    // not be empty.
    DEVICE_ID_REQUIRED,

    // The device id from policy must match the expected device id unless the
    // expected device id is empty.
    DEVICE_ID_NOT_REQUIRED,
  };

  enum ValidateTimestampOption {
    // The policy must have a timestamp field and the timestamp is checked
    // against the |not_before| value.
    TIMESTAMP_VALIDATED,

    // The timestamp is not validated.
    TIMESTAMP_NOT_VALIDATED,
  };

  virtual ~CloudPolicyValidatorBase();

  // Validation status which can be read after completion has been signaled.
  Status status() const { return status_; }
  bool success() const { return status_ == VALIDATION_OK; }

  // The policy objects owned by the validator. These are unique_ptr
  // references, so ownership can be passed on once validation is complete.
  std::unique_ptr<enterprise_management::PolicyFetchResponse>& policy() {
    return policy_;
  }
  std::unique_ptr<enterprise_management::PolicyData>& policy_data() {
    return policy_data_;
  }

  // Instruct the validator to check that the policy timestamp is present and is
  // not before |not_before| if |timestamp_option| is TIMESTAMP_VALIDATED, or to
  // not check the policy timestamp if |timestamp_option| is
  // TIMESTAMP_NOT_VALIDATED.
  void ValidateTimestamp(base::Time not_before,
                         ValidateTimestampOption timestamp_option);

  // Instruct the validator to check that the username in the policy blob
  // matches |expected_user|. If |canonicalize| is set to true, both values are
  // canonicalized before comparison.
  void ValidateUsername(const std::string& expected_user, bool canonicalize);

  // Instruct the validator to check that the policy blob is addressed to
  // |expected_domain|. This uses the domain part of the username field in the
  // policy for the check.
  void ValidateDomain(const std::string& expected_domain);

  // Instruct the validator to check that the DM token from policy matches
  // |expected_dm_token| unless |expected_dm_token| is empty. In addition, the
  // DM token from policy must not be empty if |dm_token_option| is
  // DM_TOKEN_REQUIRED.
  void ValidateDMToken(const std::string& expected_dm_token,
                       ValidateDMTokenOption dm_token_option);

  // Instruct the validator to check that the device id from policy matches
  // |expected_device_id| unless |expected_device_id| is empty. In addition, the
  // device id from policy must not be empty if |device_id_option| is
  // DEVICE_ID_REQUIRED.
  void ValidateDeviceId(const std::string& expected_device_id,
                        ValidateDeviceIdOption device_id_option);

  // Instruct the validator to check the policy type.
  void ValidatePolicyType(const std::string& policy_type);

  // Instruct the validator to check the settings_entity_id value.
  void ValidateSettingsEntityId(const std::string& settings_entity_id);

  // Instruct the validator to check that the payload can be decoded
  // successfully.
  void ValidatePayload();

  // Instruct the validator to check that |cached_key| is valid by verifying the
  // |cached_key_signature| using the passed |owning_domain| and the baked-in
  // policy verification key.
  void ValidateCachedKey(const std::string& cached_key,
                         const std::string& cached_key_signature,
                         const std::string& owning_domain);

  // Instruct the validator to check that the signature on the policy blob
  // verifies against |key|.
  void ValidateSignature(const std::string& key);

  // Instruct the validator to check that the signature on the policy blob
  // verifies against |key|. If there is a key rotation present in the policy
  // blob, this checks the signature on the new key against |key| and the policy
  // blob against the new key. New key is also validated using the passed
  // |owning_domain| and the baked-in policy verification key against the
  // proto's new_public_key_verification_signature_deprecated field.
  void ValidateSignatureAllowingRotation(const std::string& key,
                                         const std::string& owning_domain);

  // Similar to ValidateSignature(), this instructs the validator to check the
  // signature on the policy blob. However, this variant expects a new policy
  // key set in the policy blob and makes sure the policy is signed using that
  // key. This should be called at setup time when there is no existing policy
  // key present to check against. New key is validated using the baked-in
  // policy verification key against the proto's
  // new_public_key_verification_signature_deprecated field.
  void ValidateInitialKey(const std::string& owning_domain);

  // Convenience helper that instructs the validator to check timestamp, DM
  // token and device id based on the current policy blob. |policy_data| may be
  // nullptr, in which case the timestamp lower bound check is waived and the DM
  // token as well as the device id are checked against empty strings.
  // |timestamp_option|, |dm_token_option| and |device_id_option| have the same
  // effect as the corresponding parameters for ValidateTimestamp(),
  // ValidateDMToken() and ValidateDeviceId().
  void ValidateAgainstCurrentPolicy(
      const enterprise_management::PolicyData* policy_data,
      ValidateTimestampOption timestamp_option,
      ValidateDMTokenOption dm_token_option,
      ValidateDeviceIdOption device_id_option);

  // Immediately performs validation on the current thread.
  void RunValidation();

 protected:
  // Create a new validator that checks |policy_response|. |payload| is the
  // message that the policy payload will be parsed to, and it needs to stay
  // valid for the lifetime of the validator.
  CloudPolicyValidatorBase(
      std::unique_ptr<enterprise_management::PolicyFetchResponse>
          policy_response,
      google::protobuf::MessageLite* payload,
      scoped_refptr<base::SequencedTaskRunner> background_task_runner);

  // Posts an asynchronous call to PerformValidation of the passed |validator|,
  // which will eventually report its result via |completion_callback|.
  static void PostValidationTask(
      std::unique_ptr<CloudPolicyValidatorBase> validator,
      const base::Closure& completion_callback);

 private:
  // Internal flags indicating what to check.
  enum ValidationFlags {
    VALIDATE_TIMESTAMP   = 1 << 0,
    VALIDATE_USERNAME    = 1 << 1,
    VALIDATE_DOMAIN      = 1 << 2,
    VALIDATE_DM_TOKEN    = 1 << 3,
    VALIDATE_POLICY_TYPE = 1 << 4,
    VALIDATE_ENTITY_ID   = 1 << 5,
    VALIDATE_PAYLOAD     = 1 << 6,
    VALIDATE_SIGNATURE   = 1 << 7,
    VALIDATE_INITIAL_KEY = 1 << 8,
    VALIDATE_CACHED_KEY  = 1 << 9,
    VALIDATE_DEVICE_ID   = 1 << 10,
  };

  enum SignatureType {
    SHA1,
    SHA256
  };

  // Performs validation, called on a background thread.
  static void PerformValidation(
      std::unique_ptr<CloudPolicyValidatorBase> self,
      scoped_refptr<base::SingleThreadTaskRunner> task_runner,
      const base::Closure& completion_callback);

  // Reports completion to the |completion_callback_|.
  static void ReportCompletion(std::unique_ptr<CloudPolicyValidatorBase> self,
                               const base::Closure& completion_callback);

  // Invokes all the checks and reports the result.
  void RunChecks();

  // Helper routine that verifies that the new public key in the policy blob
  // is properly signed by the baked-in policy verification key.
  bool CheckNewPublicKeyVerificationSignature();

  // Helper routine that performs a verification-key-based signature check,
  // which includes the domain name associated with this policy. Returns true
  // if the verification succeeds, or if |signature| is empty.
  bool CheckVerificationKeySignature(const std::string& key_to_verify,
                                     const std::string& server_key,
                                     const std::string& signature);

  // Returns the domain name from the policy being validated. Returns an
  // empty string if the policy does not contain a username field.
  std::string ExtractDomainFromPolicy();

  // Sets the owning domain used to verify new public keys, and ensures that
  // callers don't try to set conflicting values.
  void set_owning_domain(const std::string& owning_domain);

  // Helper functions implementing individual checks.
  Status CheckTimestamp();
  Status CheckUsername();
  Status CheckDomain();
  Status CheckDMToken();
  Status CheckDeviceId();
  Status CheckPolicyType();
  Status CheckEntityId();
  Status CheckPayload();
  Status CheckSignature();
  Status CheckInitialKey();
  Status CheckCachedKey();

  // Verifies the SHA1/ or SHA256/RSA |signature| on |data| against |key|.
  // |signature_type| specifies the type of signature (SHA1 or SHA256).
  static bool VerifySignature(const std::string& data,
                              const std::string& key,
                              const std::string& signature,
                              SignatureType signature_type);

  Status status_;
  std::unique_ptr<enterprise_management::PolicyFetchResponse> policy_;
  std::unique_ptr<enterprise_management::PolicyData> policy_data_;
  google::protobuf::MessageLite* payload_;

  int validation_flags_;
  int64_t timestamp_not_before_;
  ValidateTimestampOption timestamp_option_;
  ValidateDMTokenOption dm_token_option_;
  ValidateDeviceIdOption device_id_option_;
  std::string user_;
  bool canonicalize_user_;
  std::string domain_;
  std::string dm_token_;
  std::string device_id_;
  std::string policy_type_;
  std::string settings_entity_id_;
  std::string key_;
  std::string cached_key_;
  std::string cached_key_signature_;
  std::string verification_key_;
  std::string owning_domain_;
  bool allow_key_rotation_;
  scoped_refptr<base::SequencedTaskRunner> background_task_runner_;

  DISALLOW_COPY_AND_ASSIGN(CloudPolicyValidatorBase);
};

// A simple type-parameterized extension of CloudPolicyValidator that
// facilitates working with the actual protobuf payload type.
template<typename PayloadProto>
class POLICY_EXPORT CloudPolicyValidator : public CloudPolicyValidatorBase {
 public:
  using CompletionCallback = base::Callback<void(CloudPolicyValidator*)>;

  virtual ~CloudPolicyValidator() {}

  // Creates a new validator.
  // |background_task_runner| is optional; if RunValidation() is used directly
  // and StartValidation() is not used then it can be nullptr.
  static std::unique_ptr<CloudPolicyValidator> Create(
      std::unique_ptr<enterprise_management::PolicyFetchResponse>
          policy_response,
      scoped_refptr<base::SequencedTaskRunner> background_task_runner) {
    return base::WrapUnique<CloudPolicyValidator>(new CloudPolicyValidator(
        std::move(policy_response), base::MakeUnique<PayloadProto>(),
        background_task_runner));
  }

  std::unique_ptr<PayloadProto>& payload() { return payload_; }

  // Kicks off asynchronous validation through |validator|.
  // |completion_callback| is invoked when done.
  static void StartValidation(std::unique_ptr<CloudPolicyValidator> validator,
                              const CompletionCallback& completion_callback) {
    CloudPolicyValidator* const validator_ptr = validator.release();
    PostValidationTask(
        base::WrapUnique<CloudPolicyValidatorBase>(validator_ptr),
        base::Bind(completion_callback, validator_ptr));
  }

 private:
  CloudPolicyValidator(
      std::unique_ptr<enterprise_management::PolicyFetchResponse>
          policy_response,
      std::unique_ptr<PayloadProto> payload,
      scoped_refptr<base::SequencedTaskRunner> background_task_runner)
      : CloudPolicyValidatorBase(std::move(policy_response),
                                 payload.get(),
                                 background_task_runner),
        payload_(std::move(payload)) {}

  std::unique_ptr<PayloadProto> payload_;

  DISALLOW_COPY_AND_ASSIGN(CloudPolicyValidator);
};

using UserCloudPolicyValidator =
    CloudPolicyValidator<enterprise_management::CloudPolicySettings>;

#if !defined(OS_ANDROID) && !defined(OS_IOS)
using ComponentCloudPolicyValidator =
    CloudPolicyValidator<enterprise_management::ExternalPolicyData>;
#endif

}  // namespace policy

#endif  // COMPONENTS_POLICY_CORE_COMMON_CLOUD_CLOUD_POLICY_VALIDATOR_H_
