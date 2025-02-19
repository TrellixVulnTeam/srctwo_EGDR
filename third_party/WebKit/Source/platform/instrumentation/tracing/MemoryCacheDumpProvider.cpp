// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/instrumentation/tracing/MemoryCacheDumpProvider.h"

namespace blink {

DEFINE_TRACE(MemoryCacheDumpClient) {}

MemoryCacheDumpProvider* MemoryCacheDumpProvider::Instance() {
  DEFINE_STATIC_LOCAL(MemoryCacheDumpProvider, instance, ());
  return &instance;
}

bool MemoryCacheDumpProvider::OnMemoryDump(
    const base::trace_event::MemoryDumpArgs& args,
    base::trace_event::ProcessMemoryDump* memory_dump) {
  DCHECK(IsMainThread());
  if (!client_)
    return false;

  WebMemoryDumpLevelOfDetail level;
  switch (args.level_of_detail) {
    case base::trace_event::MemoryDumpLevelOfDetail::BACKGROUND:
      level = blink::WebMemoryDumpLevelOfDetail::kBackground;
      break;
    case base::trace_event::MemoryDumpLevelOfDetail::LIGHT:
      level = blink::WebMemoryDumpLevelOfDetail::kLight;
      break;
    case base::trace_event::MemoryDumpLevelOfDetail::DETAILED:
      level = blink::WebMemoryDumpLevelOfDetail::kDetailed;
      break;
    default:
      NOTREACHED();
      return false;
  }

  WebProcessMemoryDump dump(args.level_of_detail, memory_dump);
  return client_->OnMemoryDump(level, &dump);
}

MemoryCacheDumpProvider::MemoryCacheDumpProvider() {}

MemoryCacheDumpProvider::~MemoryCacheDumpProvider() {}

}  // namespace blink
