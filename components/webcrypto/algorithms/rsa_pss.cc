// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include "base/memory/ptr_util.h"
#include "components/webcrypto/algorithms/rsa.h"
#include "components/webcrypto/algorithms/rsa_sign.h"
#include "components/webcrypto/status.h"
#include "third_party/WebKit/public/platform/WebCryptoAlgorithmParams.h"

namespace webcrypto {

namespace {

class RsaPssImplementation : public RsaHashedAlgorithm {
 public:
  RsaPssImplementation()
      : RsaHashedAlgorithm(blink::kWebCryptoKeyUsageVerify,
                           blink::kWebCryptoKeyUsageSign) {}

  const char* GetJwkAlgorithm(
      const blink::WebCryptoAlgorithmId hash) const override {
    switch (hash) {
      case blink::kWebCryptoAlgorithmIdSha1:
        return "PS1";
      case blink::kWebCryptoAlgorithmIdSha256:
        return "PS256";
      case blink::kWebCryptoAlgorithmIdSha384:
        return "PS384";
      case blink::kWebCryptoAlgorithmIdSha512:
        return "PS512";
      default:
        return NULL;
    }
  }

  Status Sign(const blink::WebCryptoAlgorithm& algorithm,
              const blink::WebCryptoKey& key,
              const CryptoData& data,
              std::vector<uint8_t>* buffer) const override {
    return RsaSign(key, algorithm.RsaPssParams()->SaltLengthBytes(), data,
                   buffer);
  }

  Status Verify(const blink::WebCryptoAlgorithm& algorithm,
                const blink::WebCryptoKey& key,
                const CryptoData& signature,
                const CryptoData& data,
                bool* signature_match) const override {
    return RsaVerify(key, algorithm.RsaPssParams()->SaltLengthBytes(),
                     signature, data, signature_match);
  }
};

}  // namespace

std::unique_ptr<AlgorithmImplementation> CreateRsaPssImplementation() {
  return base::WrapUnique(new RsaPssImplementation);
}

}  // namespace webcrypto
