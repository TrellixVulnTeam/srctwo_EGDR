// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ProcessMetrics_h
#define ProcessMetrics_h

#include "base/process/process_metrics.h"

namespace WTF {

size_t GetMallocUsage() {
  std::unique_ptr<base::ProcessMetrics> metric(
      base::ProcessMetrics::CreateCurrentProcessMetrics());
  return metric->GetMallocUsage();
}

}  // namespace WTF

#endif  // ProcessMetrics_h
