/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Google Inc. nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef V0CustomElementProcessingStack_h
#define V0CustomElementProcessingStack_h

#include "core/CoreExport.h"
#include "core/html/custom/V0CustomElementCallbackQueue.h"
#include "platform/wtf/Vector.h"

namespace blink {

class CORE_EXPORT V0CustomElementProcessingStack
    : public GarbageCollected<V0CustomElementProcessingStack> {
  WTF_MAKE_NONCOPYABLE(V0CustomElementProcessingStack);

 public:
  // This is stack allocated in many DOM callbacks. Make it cheap.
  class CallbackDeliveryScope {
    STACK_ALLOCATED();

   public:
    CallbackDeliveryScope() : saved_element_queue_start_(element_queue_start_) {
      element_queue_start_ = element_queue_end_;
    }

    ~CallbackDeliveryScope() {
      if (element_queue_start_ != element_queue_end_)
        ProcessElementQueueAndPop();
      element_queue_start_ = saved_element_queue_start_;
    }

   private:
    size_t saved_element_queue_start_;
  };

  static bool InCallbackDeliveryScope() { return element_queue_start_; }

  static V0CustomElementProcessingStack& Instance();
  void Enqueue(V0CustomElementCallbackQueue*);

  DECLARE_TRACE();

 private:
  V0CustomElementProcessingStack() {
    // Add a null element as a sentinel. This makes it possible to
    // identify elements queued when there is no
    // CallbackDeliveryScope active. Also, if the processing stack
    // is popped when empty, this sentinel will cause a null deref
    // crash.
    V0CustomElementCallbackQueue* sentinel = 0;
    for (size_t i = 0; i < kNumSentinels; i++)
      flattened_processing_stack_.push_back(sentinel);
    DCHECK_EQ(element_queue_end_, flattened_processing_stack_.size());
  }

  // The start of the element queue on the top of the processing
  // stack. An offset into instance().m_flattenedProcessingStack.
  static size_t element_queue_start_;

  // The end of the element queue on the top of the processing
  // stack. A cache of instance().m_flattenedProcessingStack.size().
  static size_t element_queue_end_;

  static V0CustomElementCallbackQueue::ElementQueueId CurrentElementQueue() {
    return V0CustomElementCallbackQueue::ElementQueueId(element_queue_start_);
  }

  static void ProcessElementQueueAndPop();
  void ProcessElementQueueAndPop(size_t start, size_t end);

  // The processing stack, flattened. Element queues lower in the
  // stack appear toward the head of the vector. The first element
  // is a null sentinel value.
  static const size_t kNumSentinels = 1;
  HeapVector<Member<V0CustomElementCallbackQueue>> flattened_processing_stack_;
};

}  // namespace blink

#endif  // V0CustomElementProcessingStack_h
