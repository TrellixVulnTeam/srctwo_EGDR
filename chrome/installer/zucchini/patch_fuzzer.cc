// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>
#include <stdint.h>

#include "base/optional.h"
#include "chrome/installer/zucchini/buffer_view.h"
#include "chrome/installer/zucchini/patch_reader.h"

// Entry point for LibFuzzer.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  logging::SetMinLogLevel(3);  // Disable console spamming.
  zucchini::ConstBufferView patch(data, size);
  base::Optional<zucchini::EnsemblePatchReader> patch_reader =
      zucchini::EnsemblePatchReader::Create(patch);
  return 0;
}
