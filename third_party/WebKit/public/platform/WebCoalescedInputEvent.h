// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebCoalescedInputEvent_h
#define WebCoalescedInputEvent_h

#include "WebCommon.h"
#include "WebInputEvent.h"

#include <memory>
#include <vector>

namespace blink {

// This class is representing a polymorphic WebInputEvent structure with its
// coalesced events. The event could be any events defined in WebInputEvent.h.
class BLINK_PLATFORM_EXPORT WebCoalescedInputEvent {
 public:
  explicit WebCoalescedInputEvent(const WebInputEvent&);
  WebCoalescedInputEvent(const WebInputEvent&,
                         const std::vector<const WebInputEvent*>&);
  // Copy constructor to deep copy the event.
  WebCoalescedInputEvent(const WebCoalescedInputEvent&);

  WebInputEvent* EventPointer();
  void AddCoalescedEvent(const blink::WebInputEvent&);
  const WebInputEvent& Event() const;
  size_t CoalescedEventSize() const;
  const WebInputEvent& CoalescedEvent(size_t index) const;
  std::vector<const WebInputEvent*> GetCoalescedEventsPointers() const;

 private:
  // TODO(hans): Remove this once clang-cl knows to not inline dtors that
  // call operator(), https://crbug.com/691714
  struct BLINK_PLATFORM_EXPORT WebInputEventDeleter {
    void operator()(blink::WebInputEvent*) const;
  };

  using WebScopedInputEvent =
      std::unique_ptr<WebInputEvent, WebInputEventDeleter>;

  WebScopedInputEvent MakeWebScopedInputEvent(const blink::WebInputEvent&);

  WebScopedInputEvent event_;
  std::vector<WebScopedInputEvent> coalesced_events_;
};

using WebScopedCoalescedInputEvent = std::unique_ptr<WebCoalescedInputEvent>;

}  // namespace blink

#endif
