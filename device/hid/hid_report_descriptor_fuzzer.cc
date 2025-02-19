// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>
#include <stdint.h>

#include "device/hid/hid_report_descriptor.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  device::HidReportDescriptor desc(std::vector<uint8_t>(data, data + size));
  std::vector<device::HidCollectionInfo> top_level_collections;
  bool has_report_id;
  size_t max_input_report_size;
  size_t max_output_report_size;
  size_t max_feature_report_size;
  desc.GetDetails(&top_level_collections, &has_report_id,
                  &max_input_report_size, &max_output_report_size,
                  &max_feature_report_size);
  return 0;
}
