// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/instrumentation/tracing/web_process_memory_dump.h"

#include "base/memory/discardable_memory.h"
#include "base/memory/ptr_util.h"
#include "base/strings/stringprintf.h"
#include "base/trace_event/heap_profiler_heap_dump_writer.h"
#include "base/trace_event/process_memory_dump.h"
#include "base/trace_event/trace_event_argument.h"
#include "base/trace_event/trace_event_memory_overhead.h"
#include "platform/instrumentation/tracing/web_memory_allocator_dump.h"
#include "platform/wtf/text/StringUTF8Adaptor.h"
#include "skia/ext/skia_trace_memory_dump_impl.h"

#include <stddef.h>

namespace blink {

WebProcessMemoryDump::WebProcessMemoryDump()
    : owned_process_memory_dump_(new base::trace_event::ProcessMemoryDump(
          nullptr,
          {base::trace_event::MemoryDumpLevelOfDetail::DETAILED})),
      process_memory_dump_(owned_process_memory_dump_.get()),
      level_of_detail_(base::trace_event::MemoryDumpLevelOfDetail::DETAILED) {}

WebProcessMemoryDump::WebProcessMemoryDump(
    base::trace_event::MemoryDumpLevelOfDetail level_of_detail,
    base::trace_event::ProcessMemoryDump* process_memory_dump)
    : process_memory_dump_(process_memory_dump),
      level_of_detail_(level_of_detail) {}

WebProcessMemoryDump::~WebProcessMemoryDump() {}

blink::WebMemoryAllocatorDump* WebProcessMemoryDump::CreateMemoryAllocatorDump(
    const String& absolute_name) {
  StringUTF8Adaptor adapter(absolute_name);
  std::string name(adapter.Data(), adapter.length());
  // Get a MemoryAllocatorDump from the base/ object.
  base::trace_event::MemoryAllocatorDump* memory_allocator_dump =
      process_memory_dump_->CreateAllocatorDump(name);

  return CreateWebMemoryAllocatorDump(memory_allocator_dump);
}

blink::WebMemoryAllocatorDump* WebProcessMemoryDump::CreateMemoryAllocatorDump(
    const String& absolute_name,
    blink::WebMemoryAllocatorDumpGuid guid) {
  StringUTF8Adaptor adapter(absolute_name);
  std::string name(adapter.Data(), adapter.length());
  // Get a MemoryAllocatorDump from the base/ object with given guid.
  base::trace_event::MemoryAllocatorDump* memory_allocator_dump =
      process_memory_dump_->CreateAllocatorDump(
          name, base::trace_event::MemoryAllocatorDumpGuid(guid));
  return CreateWebMemoryAllocatorDump(memory_allocator_dump);
}

blink::WebMemoryAllocatorDump*
WebProcessMemoryDump::CreateWebMemoryAllocatorDump(
    base::trace_event::MemoryAllocatorDump* memory_allocator_dump) {
  if (!memory_allocator_dump)
    return nullptr;

  // Wrap it and return to blink.
  WebMemoryAllocatorDump* web_memory_allocator_dump =
      new WebMemoryAllocatorDump(memory_allocator_dump);

  // memory_allocator_dumps_ will take ownership of
  // |web_memory_allocator_dump|.
  memory_allocator_dumps_.Set(memory_allocator_dump,
                              WTF::WrapUnique(web_memory_allocator_dump));
  return web_memory_allocator_dump;
}

blink::WebMemoryAllocatorDump* WebProcessMemoryDump::GetMemoryAllocatorDump(
    const String& absolute_name) const {
  StringUTF8Adaptor adapter(absolute_name);
  std::string name(adapter.Data(), adapter.length());
  // Retrieve the base MemoryAllocatorDump object and then reverse lookup
  // its wrapper.
  base::trace_event::MemoryAllocatorDump* memory_allocator_dump =
      process_memory_dump_->GetAllocatorDump(name);
  if (!memory_allocator_dump)
    return nullptr;

  // The only case of (memory_allocator_dump && !web_memory_allocator_dump)
  // is something from blink trying to get a MAD that was created from chromium,
  // which is an odd use case.
  blink::WebMemoryAllocatorDump* web_memory_allocator_dump =
      memory_allocator_dumps_.at(memory_allocator_dump);
  DCHECK(web_memory_allocator_dump);
  return web_memory_allocator_dump;
}

void WebProcessMemoryDump::Clear() {
  // Clear all the WebMemoryAllocatorDump wrappers.
  memory_allocator_dumps_.clear();

  // Clear the actual MemoryAllocatorDump objects from the underlying PMD.
  process_memory_dump_->Clear();
}

void WebProcessMemoryDump::TakeAllDumpsFrom(
    blink::WebProcessMemoryDump* other) {
  // WebProcessMemoryDump is a container of WebMemoryAllocatorDump(s) which
  // in turn are wrappers of base::trace_event::MemoryAllocatorDump(s).
  // In order to expose the move and ownership transfer semantics of the
  // underlying ProcessMemoryDump, we need to:

  // 1) Move and transfer the ownership of the wrapped
  // base::trace_event::MemoryAllocatorDump(s) instances.
  process_memory_dump_->TakeAllDumpsFrom(other->process_memory_dump_);

  // 2) Move and transfer the ownership of the WebMemoryAllocatorDump wrappers.
  const size_t expected_final_size =
      memory_allocator_dumps_.size() + other->memory_allocator_dumps_.size();
  while (!other->memory_allocator_dumps_.IsEmpty()) {
    auto first_entry = other->memory_allocator_dumps_.begin();
    base::trace_event::MemoryAllocatorDump* memory_allocator_dump =
        first_entry->key;
    memory_allocator_dumps_.Set(
        memory_allocator_dump,
        other->memory_allocator_dumps_.Take(memory_allocator_dump));
  }
  DCHECK_EQ(expected_final_size, memory_allocator_dumps_.size());
  DCHECK(other->memory_allocator_dumps_.IsEmpty());
}

void WebProcessMemoryDump::AddOwnershipEdge(
    blink::WebMemoryAllocatorDumpGuid source,
    blink::WebMemoryAllocatorDumpGuid target,
    int importance) {
  process_memory_dump_->AddOwnershipEdge(
      base::trace_event::MemoryAllocatorDumpGuid(source),
      base::trace_event::MemoryAllocatorDumpGuid(target), importance);
}

void WebProcessMemoryDump::AddOwnershipEdge(
    blink::WebMemoryAllocatorDumpGuid source,
    blink::WebMemoryAllocatorDumpGuid target) {
  process_memory_dump_->AddOwnershipEdge(
      base::trace_event::MemoryAllocatorDumpGuid(source),
      base::trace_event::MemoryAllocatorDumpGuid(target));
}

void WebProcessMemoryDump::AddSuballocation(
    blink::WebMemoryAllocatorDumpGuid source,
    const String& target_node_name) {
  StringUTF8Adaptor adapter(target_node_name);
  std::string target_node(adapter.Data(), adapter.length());
  process_memory_dump_->AddSuballocation(
      base::trace_event::MemoryAllocatorDumpGuid(source), target_node);
}

SkTraceMemoryDump* WebProcessMemoryDump::CreateDumpAdapterForSkia(
    const String& dump_name_prefix) {
  StringUTF8Adaptor adapter(dump_name_prefix);
  std::string prefix(adapter.Data(), adapter.length());
  sk_trace_dump_list_.push_back(
      base::WrapUnique(new skia::SkiaTraceMemoryDumpImpl(
          prefix, level_of_detail_, process_memory_dump_)));
  return sk_trace_dump_list_.back().get();
}

blink::WebMemoryAllocatorDump*
WebProcessMemoryDump::CreateDiscardableMemoryAllocatorDump(
    const std::string& name,
    base::DiscardableMemory* discardable) {
  base::trace_event::MemoryAllocatorDump* dump =
      discardable->CreateMemoryAllocatorDump(name.c_str(),
                                             process_memory_dump_);
  return CreateWebMemoryAllocatorDump(dump);
}

void WebProcessMemoryDump::DumpHeapUsage(
    const std::unordered_map<base::trace_event::AllocationContext,
                             base::trace_event::AllocationMetrics>&
        metrics_by_context,
    base::trace_event::TraceEventMemoryOverhead& overhead,
    const char* allocator_name) {
  process_memory_dump_->DumpHeapUsage(metrics_by_context, overhead,
                                      allocator_name);
}

}  // namespace content
