// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/events/event_source.h"

#include <algorithm>

#include "base/stl_util.h"
#include "ui/events/event_rewriter.h"
#include "ui/events/event_sink.h"

namespace ui {

EventSource::EventSource() {}

EventSource::~EventSource() {}

void EventSource::AddEventRewriter(EventRewriter* rewriter) {
  DCHECK(rewriter);
  DCHECK(!base::ContainsValue(rewriter_list_, rewriter));
  rewriter_list_.push_back(rewriter);
}

void EventSource::RemoveEventRewriter(EventRewriter* rewriter) {
  EventRewriterList::iterator find =
      std::find(rewriter_list_.begin(), rewriter_list_.end(), rewriter);
  if (find != rewriter_list_.end())
    rewriter_list_.erase(find);
}

EventDispatchDetails EventSource::SendEventToSink(Event* event) {
  std::unique_ptr<Event> rewritten_event;
  EventRewriteStatus status = EVENT_REWRITE_CONTINUE;
  EventRewriterList::const_iterator it = rewriter_list_.begin(),
                                    end = rewriter_list_.end();
  for (; it != end; ++it) {
    status = (*it)->RewriteEvent(*event, &rewritten_event);
    if (status == EVENT_REWRITE_DISCARD) {
      CHECK(!rewritten_event);
      return EventDispatchDetails();
    }
    if (status == EVENT_REWRITE_CONTINUE) {
      CHECK(!rewritten_event);
      continue;
    }
    break;
  }
  CHECK((it == end && !rewritten_event) || rewritten_event);
  EventDispatchDetails details =
      DeliverEventToSink(rewritten_event ? rewritten_event.get() : event);
  if (details.dispatcher_destroyed)
    return details;

  while (status == EVENT_REWRITE_DISPATCH_ANOTHER) {
    std::unique_ptr<Event> new_event;
    status = (*it)->NextDispatchEvent(*rewritten_event, &new_event);
    if (status == EVENT_REWRITE_DISCARD)
      return EventDispatchDetails();
    CHECK_NE(EVENT_REWRITE_CONTINUE, status);
    CHECK(new_event);
    details = DeliverEventToSink(new_event.get());
    if (details.dispatcher_destroyed)
      return details;
    rewritten_event = std::move(new_event);
  }
  return EventDispatchDetails();
}

EventDispatchDetails EventSource::DeliverEventToSink(Event* event) {
  EventSink* sink = GetEventSink();
  CHECK(sink);
  return sink->OnEventFromSource(event);
}

}  // namespace ui
