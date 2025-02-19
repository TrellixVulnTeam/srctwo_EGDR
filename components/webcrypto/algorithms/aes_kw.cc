// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>
#include <stdint.h>

#include <vector>

#include "base/location.h"
#include "base/memory/ptr_util.h"
#include "base/numerics/safe_math.h"
#include "components/webcrypto/algorithms/aes.h"
#include "components/webcrypto/blink_key_handle.h"
#include "components/webcrypto/crypto_data.h"
#include "components/webcrypto/status.h"
#include "crypto/openssl_util.h"
#include "third_party/boringssl/src/include/openssl/aes.h"

namespace webcrypto {

namespace {

class AesKwImplementation : public AesAlgorithm {
 public:
  AesKwImplementation()
      : AesAlgorithm(blink::kWebCryptoKeyUsageWrapKey |
                         blink::kWebCryptoKeyUsageUnwrapKey,
                     "KW") {}

  Status Encrypt(const blink::WebCryptoAlgorithm& algorithm,
                 const blink::WebCryptoKey& key,
                 const CryptoData& data,
                 std::vector<uint8_t>* buffer) const override {
    crypto::OpenSSLErrStackTracer err_tracer(FROM_HERE);

    // These length checks are done in order to give a more specific
    // error. These are not required for correctness.
    if (data.byte_length() < 16)
      return Status::ErrorDataTooSmall();
    if (data.byte_length() % 8)
      return Status::ErrorInvalidAesKwDataLength();

    // Key import validates key sizes, so the bits computation will not
    // overflow.
    const std::vector<uint8_t>& raw_key = GetSymmetricKeyData(key);
    AES_KEY aes_key;
    if (AES_set_encrypt_key(raw_key.data(),
                            static_cast<unsigned>(raw_key.size() * 8),
                            &aes_key) < 0) {
      return Status::OperationError();
    }

    // Key wrap's overhead is 8 bytes.
    base::CheckedNumeric<size_t> length(data.byte_length());
    length += 8;
    if (!length.IsValid())
      return Status::ErrorDataTooLarge();

    buffer->resize(length.ValueOrDie());
    if (AES_wrap_key(&aes_key, nullptr /* default IV */, buffer->data(),
                     data.bytes(), data.byte_length()) < 0) {
      return Status::OperationError();
    }

    return Status::Success();
  }

  Status Decrypt(const blink::WebCryptoAlgorithm& algorithm,
                 const blink::WebCryptoKey& key,
                 const CryptoData& data,
                 std::vector<uint8_t>* buffer) const override {
    crypto::OpenSSLErrStackTracer err_tracer(FROM_HERE);

    // These length checks are done in order to give a more specific
    // error. These are not required for correctness.
    if (data.byte_length() < 24)
      return Status::ErrorDataTooSmall();
    if (data.byte_length() % 8)
      return Status::ErrorInvalidAesKwDataLength();

    // Key import validates key sizes, so the bits computation will not
    // overflow.
    const std::vector<uint8_t>& raw_key = GetSymmetricKeyData(key);
    AES_KEY aes_key;
    if (AES_set_decrypt_key(raw_key.data(),
                            static_cast<unsigned>(raw_key.size() * 8),
                            &aes_key) < 0) {
      return Status::OperationError();
    }

    // Key wrap's overhead is 8 bytes.
    buffer->resize(data.byte_length() - 8);

    if (AES_unwrap_key(&aes_key, nullptr /* default IV */, buffer->data(),
                       data.bytes(), data.byte_length()) < 0) {
      return Status::OperationError();
    }

    return Status::Success();
  }
};

}  // namespace

std::unique_ptr<AlgorithmImplementation> CreateAesKwImplementation() {
  return base::WrapUnique(new AesKwImplementation);
}

}  // namespace webcrypto
