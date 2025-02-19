// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gpu/ipc/common/memory_stats.h"

namespace gpu {

VideoMemoryUsageStats::VideoMemoryUsageStats()
    : bytes_allocated(0) {}

VideoMemoryUsageStats::VideoMemoryUsageStats(
    const VideoMemoryUsageStats& other) = default;

VideoMemoryUsageStats::~VideoMemoryUsageStats() {}

VideoMemoryUsageStats::ProcessStats::ProcessStats()
    : video_memory(0), has_duplicates(false) {}

VideoMemoryUsageStats::ProcessStats::~ProcessStats() {}

}  // namespace gpu
