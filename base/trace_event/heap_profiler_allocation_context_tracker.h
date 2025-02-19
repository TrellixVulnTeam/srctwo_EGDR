// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TRACE_EVENT_HEAP_PROFILER_ALLOCATION_CONTEXT_TRACKER_H_
#define BASE_TRACE_EVENT_HEAP_PROFILER_ALLOCATION_CONTEXT_TRACKER_H_

#include <vector>

#include "base/atomicops.h"
#include "base/base_export.h"
#include "base/debug/stack_trace.h"
#include "base/macros.h"
#include "base/trace_event/heap_profiler_allocation_context.h"

namespace base {
namespace trace_event {

// The allocation context tracker keeps track of thread-local context for heap
// profiling. It includes a pseudo stack of trace events. On every allocation
// the tracker provides a snapshot of its context in the form of an
// |AllocationContext| that is to be stored together with the allocation
// details.
class BASE_EXPORT AllocationContextTracker {
 public:
  enum class CaptureMode : int32_t {
    DISABLED,      // Don't capture anything
    PSEUDO_STACK,  // GetContextSnapshot() returns pseudo stack trace
    NATIVE_STACK,  // GetContextSnapshot() returns native (real) stack trace
    NO_STACK,  // GetContextSnapshot() returns thread names and task contexts.
  };

  // Stack frame constructed from trace events in codebase.
  struct BASE_EXPORT PseudoStackFrame {
    const char* trace_event_category;
    const char* trace_event_name;

    bool operator==(const PseudoStackFrame& other) const {
      return trace_event_category == other.trace_event_category &&
             trace_event_name == other.trace_event_name;
    }
  };

  // Globally sets capturing mode.
  // TODO(primiano): How to guard against *_STACK -> DISABLED -> *_STACK?
  static void SetCaptureMode(CaptureMode mode);

  // Returns global capturing mode.
  inline static CaptureMode capture_mode() {
    // A little lag after heap profiling is enabled or disabled is fine, it is
    // more important that the check is as cheap as possible when capturing is
    // not enabled, so do not issue a memory barrier in the fast path.
    if (subtle::NoBarrier_Load(&capture_mode_) ==
            static_cast<int32_t>(CaptureMode::DISABLED))
      return CaptureMode::DISABLED;

    // In the slow path, an acquire load is required to pair with the release
    // store in |SetCaptureMode|. This is to ensure that the TLS slot for
    // the thread-local allocation context tracker has been initialized if
    // |capture_mode| returns something other than DISABLED.
    return static_cast<CaptureMode>(subtle::Acquire_Load(&capture_mode_));
  }

  // Returns the thread-local instance, creating one if necessary. Returns
  // always a valid instance, unless it is called re-entrantly, in which case
  // returns nullptr in the nested calls.
  static AllocationContextTracker* GetInstanceForCurrentThread();

  // Set the thread name in the AllocationContextTracker of the current thread
  // if capture is enabled.
  static void SetCurrentThreadName(const char* name);

  // Starts and ends a new ignore scope between which the allocations are
  // ignored by the heap profiler. GetContextSnapshot() returns false when
  // allocations are ignored.
  void begin_ignore_scope() { ignore_scope_depth_++; }
  void end_ignore_scope() {
    if (ignore_scope_depth_)
      ignore_scope_depth_--;
  }

  // Pushes a frame onto the thread-local pseudo stack.
  void PushPseudoStackFrame(PseudoStackFrame stack_frame);

  // Pops a frame from the thread-local pseudo stack.
  void PopPseudoStackFrame(PseudoStackFrame stack_frame);

  // Push and pop current task's context. A stack is used to support nested
  // tasks and the top of the stack will be used in allocation context.
  void PushCurrentTaskContext(const char* context);
  void PopCurrentTaskContext(const char* context);

  // Fills a snapshot of the current thread-local context. Doesn't fill and
  // returns false if allocations are being ignored.
  bool GetContextSnapshot(AllocationContext* snapshot);

  ~AllocationContextTracker();

 private:
  AllocationContextTracker();

  static subtle::Atomic32 capture_mode_;

  // The pseudo stack where frames are |TRACE_EVENT| names.
  std::vector<PseudoStackFrame> pseudo_stack_;

  // The thread name is used as the first entry in the pseudo stack.
  const char* thread_name_;

  // Stack of tasks' contexts. Context serves as a different dimension than
  // pseudo stack to cluster allocations.
  std::vector<const char*> task_contexts_;

  uint32_t ignore_scope_depth_;

  DISALLOW_COPY_AND_ASSIGN(AllocationContextTracker);
};

}  // namespace trace_event
}  // namespace base

#endif  // BASE_TRACE_EVENT_HEAP_PROFILER_ALLOCATION_CONTEXT_TRACKER_H_
