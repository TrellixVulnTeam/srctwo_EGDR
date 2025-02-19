// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>
#include <stdint.h>

#include "base/logging.h"
#include "base/values.h"
#include "components/webcrypto/algorithm_dispatch.h"
#include "components/webcrypto/algorithms/test_helpers.h"
#include "components/webcrypto/crypto_data.h"
#include "components/webcrypto/status.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/WebKit/public/platform/WebCryptoAlgorithmParams.h"
#include "third_party/WebKit/public/platform/WebCryptoKey.h"
#include "third_party/WebKit/public/platform/WebCryptoKeyAlgorithm.h"

namespace webcrypto {

namespace {

class WebCryptoShaTest : public WebCryptoTestBase {};

TEST_F(WebCryptoShaTest, DigestSampleSets) {
  std::unique_ptr<base::ListValue> tests;
  ASSERT_TRUE(ReadJsonTestFileToList("sha.json", &tests));

  for (size_t test_index = 0; test_index < tests->GetSize(); ++test_index) {
    SCOPED_TRACE(test_index);
    base::DictionaryValue* test;
    ASSERT_TRUE(tests->GetDictionary(test_index, &test));

    blink::WebCryptoAlgorithm test_algorithm =
        GetDigestAlgorithm(test, "algorithm");
    std::vector<uint8_t> test_input = GetBytesFromHexString(test, "input");
    std::vector<uint8_t> test_output = GetBytesFromHexString(test, "output");

    std::vector<uint8_t> output;
    ASSERT_EQ(Status::Success(),
              Digest(test_algorithm, CryptoData(test_input), &output));
    EXPECT_BYTES_EQ(test_output, output);
  }
}

TEST_F(WebCryptoShaTest, DigestSampleSetsInChunks) {
  std::unique_ptr<base::ListValue> tests;
  ASSERT_TRUE(ReadJsonTestFileToList("sha.json", &tests));

  for (size_t test_index = 0; test_index < tests->GetSize(); ++test_index) {
    SCOPED_TRACE(test_index);
    base::DictionaryValue* test;
    ASSERT_TRUE(tests->GetDictionary(test_index, &test));

    blink::WebCryptoAlgorithm test_algorithm =
        GetDigestAlgorithm(test, "algorithm");
    std::vector<uint8_t> test_input = GetBytesFromHexString(test, "input");
    std::vector<uint8_t> test_output = GetBytesFromHexString(test, "output");

    // Test the chunk version of the digest functions. Test with 129 byte chunks
    // because the SHA-512 chunk size is 128 bytes.
    unsigned char* output;
    unsigned int output_length;
    static const size_t kChunkSizeBytes = 129;
    size_t length = test_input.size();
    std::unique_ptr<blink::WebCryptoDigestor> digestor(
        CreateDigestor(test_algorithm.Id()));
    std::vector<uint8_t>::iterator begin = test_input.begin();
    size_t chunk_index = 0;
    while (begin != test_input.end()) {
      size_t chunk_length = std::min(kChunkSizeBytes, length - chunk_index);
      std::vector<uint8_t> chunk(begin, begin + chunk_length);
      ASSERT_TRUE(chunk.size() > 0);
      EXPECT_TRUE(digestor->Consume(chunk.data(),
                                    static_cast<unsigned int>(chunk.size())));
      chunk_index = chunk_index + chunk_length;
      begin = begin + chunk_length;
    }
    EXPECT_TRUE(digestor->Finish(output, output_length));
    EXPECT_BYTES_EQ(test_output, CryptoData(output, output_length));
  }
}

}  // namespace

}  // namespace webcrypto
