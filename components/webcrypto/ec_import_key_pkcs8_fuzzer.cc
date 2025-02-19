// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>
#include <stdint.h>

#include "components/webcrypto/fuzzer_support.h"

// Entry point for LibFuzzer.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  webcrypto::ImportEcKeyFromDerFuzzData(data, size,
                                        blink::kWebCryptoKeyFormatPkcs8);
  return 0;
}
