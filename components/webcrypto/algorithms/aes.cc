// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/webcrypto/algorithms/aes.h"

#include <stddef.h>

#include "base/logging.h"
#include "components/webcrypto/algorithms/secret_key_util.h"
#include "components/webcrypto/algorithms/util.h"
#include "components/webcrypto/blink_key_handle.h"
#include "components/webcrypto/crypto_data.h"
#include "components/webcrypto/jwk.h"
#include "components/webcrypto/status.h"
#include "third_party/WebKit/public/platform/WebCryptoAlgorithmParams.h"
#include "third_party/WebKit/public/platform/WebCryptoKeyAlgorithm.h"

namespace webcrypto {

namespace {

// Creates an AES algorithm name for the given key size (in bytes). For
// instance "A128CBC" is the result of suffix="CBC", keylen_bytes=16.
std::string MakeJwkAesAlgorithmName(const std::string& suffix,
                                    size_t keylen_bytes) {
  if (keylen_bytes == 16)
    return std::string("A128") + suffix;
  if (keylen_bytes == 24)
    return std::string("A192") + suffix;
  if (keylen_bytes == 32)
    return std::string("A256") + suffix;
  return std::string();
}

// Synthesizes an import algorithm given a key algorithm, so that
// deserialization can re-use the ImportKey*() methods.
blink::WebCryptoAlgorithm SynthesizeImportAlgorithmForClone(
    const blink::WebCryptoKeyAlgorithm& algorithm) {
  return blink::WebCryptoAlgorithm::AdoptParamsAndCreate(algorithm.Id(),
                                                         nullptr);
}

}  // namespace

AesAlgorithm::AesAlgorithm(blink::WebCryptoKeyUsageMask all_key_usages,
                           const std::string& jwk_suffix)
    : all_key_usages_(all_key_usages), jwk_suffix_(jwk_suffix) {
}

AesAlgorithm::AesAlgorithm(const std::string& jwk_suffix)
    : all_key_usages_(blink::kWebCryptoKeyUsageEncrypt |
                      blink::kWebCryptoKeyUsageDecrypt |
                      blink::kWebCryptoKeyUsageWrapKey |
                      blink::kWebCryptoKeyUsageUnwrapKey),
      jwk_suffix_(jwk_suffix) {}

Status AesAlgorithm::GenerateKey(const blink::WebCryptoAlgorithm& algorithm,
                                 bool extractable,
                                 blink::WebCryptoKeyUsageMask usages,
                                 GenerateKeyResult* result) const {
  Status status = CheckKeyCreationUsages(all_key_usages_, usages);
  if (status.IsError())
    return status;

  unsigned int keylen_bits = algorithm.AesKeyGenParams()->LengthBits();

  // 192-bit AES is intentionally unsupported (http://crbug.com/533699).
  if (keylen_bits == 192)
    return Status::ErrorAes192BitUnsupported();

  if (keylen_bits != 128 && keylen_bits != 256)
    return Status::ErrorGenerateAesKeyLength();

  return GenerateWebCryptoSecretKey(
      blink::WebCryptoKeyAlgorithm::CreateAes(algorithm.Id(), keylen_bits),
      extractable, usages, keylen_bits, result);
}

Status AesAlgorithm::ImportKey(blink::WebCryptoKeyFormat format,
                               const CryptoData& key_data,
                               const blink::WebCryptoAlgorithm& algorithm,
                               bool extractable,
                               blink::WebCryptoKeyUsageMask usages,
                               blink::WebCryptoKey* key) const {
  switch (format) {
    case blink::kWebCryptoKeyFormatRaw:
      return ImportKeyRaw(key_data, algorithm, extractable, usages, key);
    case blink::kWebCryptoKeyFormatJwk:
      return ImportKeyJwk(key_data, algorithm, extractable, usages, key);
    default:
      return Status::ErrorUnsupportedImportKeyFormat();
  }
}

Status AesAlgorithm::ExportKey(blink::WebCryptoKeyFormat format,
                               const blink::WebCryptoKey& key,
                               std::vector<uint8_t>* buffer) const {
  switch (format) {
    case blink::kWebCryptoKeyFormatRaw:
      return ExportKeyRaw(key, buffer);
    case blink::kWebCryptoKeyFormatJwk:
      return ExportKeyJwk(key, buffer);
    default:
      return Status::ErrorUnsupportedExportKeyFormat();
  }
}

Status AesAlgorithm::ImportKeyRaw(const CryptoData& key_data,
                                  const blink::WebCryptoAlgorithm& algorithm,
                                  bool extractable,
                                  blink::WebCryptoKeyUsageMask usages,
                                  blink::WebCryptoKey* key) const {
  Status status = CheckKeyCreationUsages(all_key_usages_, usages);
  if (status.IsError())
    return status;

  const unsigned int keylen_bytes = key_data.byte_length();

  // 192-bit AES is intentionally unsupported (http://crbug.com/533699).
  if (keylen_bytes == 24)
    return Status::ErrorAes192BitUnsupported();

  if (keylen_bytes != 16 && keylen_bytes != 32)
    return Status::ErrorImportAesKeyLength();

  // No possibility of overflow.
  unsigned int keylen_bits = keylen_bytes * 8;

  return CreateWebCryptoSecretKey(
      key_data,
      blink::WebCryptoKeyAlgorithm::CreateAes(algorithm.Id(), keylen_bits),
      extractable, usages, key);
}

Status AesAlgorithm::ImportKeyJwk(const CryptoData& key_data,
                                  const blink::WebCryptoAlgorithm& algorithm,
                                  bool extractable,
                                  blink::WebCryptoKeyUsageMask usages,
                                  blink::WebCryptoKey* key) const {
  Status status = CheckKeyCreationUsages(all_key_usages_, usages);
  if (status.IsError())
    return status;

  std::vector<uint8_t> raw_data;
  JwkReader jwk;
  status = ReadSecretKeyNoExpectedAlgJwk(key_data, extractable, usages,
                                         &raw_data, &jwk);
  if (status.IsError())
    return status;

  bool has_jwk_alg;
  std::string jwk_alg;
  status = jwk.GetAlg(&jwk_alg, &has_jwk_alg);
  if (status.IsError())
    return status;

  if (has_jwk_alg) {
    std::string expected_algorithm_name =
        MakeJwkAesAlgorithmName(jwk_suffix_, raw_data.size());

    if (jwk_alg != expected_algorithm_name) {
      // Give a different error message if the key length was wrong.
      if (jwk_alg == MakeJwkAesAlgorithmName(jwk_suffix_, 16) ||
          jwk_alg == MakeJwkAesAlgorithmName(jwk_suffix_, 24) ||
          jwk_alg == MakeJwkAesAlgorithmName(jwk_suffix_, 32)) {
        return Status::ErrorJwkIncorrectKeyLength();
      }
      return Status::ErrorJwkAlgorithmInconsistent();
    }
  }

  return ImportKeyRaw(CryptoData(raw_data), algorithm, extractable, usages,
                      key);
}

Status AesAlgorithm::ExportKeyRaw(const blink::WebCryptoKey& key,
                                  std::vector<uint8_t>* buffer) const {
  *buffer = GetSymmetricKeyData(key);
  return Status::Success();
}

Status AesAlgorithm::ExportKeyJwk(const blink::WebCryptoKey& key,
                                  std::vector<uint8_t>* buffer) const {
  const std::vector<uint8_t>& raw_data = GetSymmetricKeyData(key);

  WriteSecretKeyJwk(CryptoData(raw_data),
                    MakeJwkAesAlgorithmName(jwk_suffix_, raw_data.size()),
                    key.Extractable(), key.Usages(), buffer);

  return Status::Success();
}

Status AesAlgorithm::DeserializeKeyForClone(
    const blink::WebCryptoKeyAlgorithm& algorithm,
    blink::WebCryptoKeyType type,
    bool extractable,
    blink::WebCryptoKeyUsageMask usages,
    const CryptoData& key_data,
    blink::WebCryptoKey* key) const {
  if (algorithm.ParamsType() != blink::kWebCryptoKeyAlgorithmParamsTypeAes ||
      type != blink::kWebCryptoKeyTypeSecret)
    return Status::ErrorUnexpected();

  return ImportKeyRaw(key_data, SynthesizeImportAlgorithmForClone(algorithm),
                      extractable, usages, key);
}

Status AesAlgorithm::GetKeyLength(
    const blink::WebCryptoAlgorithm& key_length_algorithm,
    bool* has_length_bits,
    unsigned int* length_bits) const {
  *has_length_bits = true;
  *length_bits = key_length_algorithm.AesDerivedKeyParams()->LengthBits();

  if (*length_bits == 128 || *length_bits == 256)
    return Status::Success();

  // 192-bit AES is intentionally unsupported (http://crbug.com/533699).
  if (*length_bits == 192)
    return Status::ErrorAes192BitUnsupported();

  return Status::ErrorGetAesKeyLength();
}

}  // namespace webcrypto
