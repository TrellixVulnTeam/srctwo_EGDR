// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EventHandlerRegistry_h
#define EventHandlerRegistry_h

#include "core/CoreExport.h"  // TODO(sashab): Remove this.
#include "core/page/Page.h"
#include "platform/wtf/HashCountedSet.h"

namespace blink {

class AddEventListenerOptions;
class Document;
class EventTarget;

typedef HashCountedSet<UntracedMember<EventTarget>> EventTargetSet;

// Registry for keeping track of event handlers. Note that only handlers on
// documents that can be rendered or can receive input (i.e., are attached to a
// Page) are registered here.
class CORE_EXPORT EventHandlerRegistry final
    : public GarbageCollectedFinalized<EventHandlerRegistry> {
 public:
  explicit EventHandlerRegistry(Page&);
  virtual ~EventHandlerRegistry();

  // Supported event handler classes. Note that each one may correspond to
  // multiple event types.
  enum EventHandlerClass {
    kScrollEvent,
    kWheelEventBlocking,
    kWheelEventPassive,
    kTouchAction,
    kTouchStartOrMoveEventBlocking,
    kTouchStartOrMoveEventBlockingLowLatency,
    kTouchStartOrMoveEventPassive,
    kTouchEndOrCancelEventBlocking,
    kTouchEndOrCancelEventPassive,
    kPointerEvent,
#if DCHECK_IS_ON()
    // Additional event categories for verifying handler tracking logic.
    kEventsForTesting,
#endif
    kEventHandlerClassCount,  // Must be the last entry.
  };

  // Returns true if the Page has event handlers of the specified class.
  bool HasEventHandlers(EventHandlerClass) const;

  // Returns a set of EventTargets which have registered handlers of the given
  // class.
  const EventTargetSet* EventHandlerTargets(EventHandlerClass) const;

  // Registration and management of event handlers attached to EventTargets.
  void DidAddEventHandler(EventTarget&,
                          const AtomicString& event_type,
                          const AddEventListenerOptions&);
  void DidAddEventHandler(EventTarget&, EventHandlerClass);
  void DidRemoveEventHandler(EventTarget&,
                             const AtomicString& event_type,
                             const AddEventListenerOptions&);
  void DidRemoveEventHandler(EventTarget&, EventHandlerClass);
  void DidRemoveAllEventHandlers(EventTarget&);

  void DidMoveIntoPage(EventTarget&);
  void DidMoveOutOfPage(EventTarget&);

  // Either |documentDetached| or |didMove{Into,OutOf,Between}Pages| must
  // be called whenever the Page that is associated with a registered event
  // target changes. This ensures the registry does not end up with stale
  // references to handlers that are no longer related to it.
  void DocumentDetached(Document&);

  DECLARE_TRACE();
  void ClearWeakMembers(Visitor*);

 private:
  enum ChangeOperation {
    kAdd,       // Add a new event handler.
    kRemove,    // Remove an existing event handler.
    kRemoveAll  // Remove any and all existing event handlers for a given
                // target.
  };

  // Returns true if |eventType| belongs to a class this registry tracks.
  static bool EventTypeToClass(const AtomicString& event_type,
                               const AddEventListenerOptions&,
                               EventHandlerClass* result);

  // Returns true if the operation actually added a new target or completely
  // removed an existing one.
  bool UpdateEventHandlerTargets(ChangeOperation,
                                 EventHandlerClass,
                                 EventTarget*);

  // Called on the EventHandlerRegistry of the root Document to notify
  // clients when we have added the first handler or removed the last one for
  // a given event class. |hasActiveHandlers| can be used to distinguish
  // between the two cases.
  void NotifyHasHandlersChanged(EventTarget*,
                                EventHandlerClass,
                                bool has_active_handlers);

  // Called to notify clients whenever a single event handler target is
  // registered or unregistered. If several handlers are registered for the
  // same target, only the first registration will trigger this notification.
  void NotifyDidAddOrRemoveEventHandlerTarget(EventHandlerClass);

  // Record a change operation to a given event handler class and notify any
  // parent registry and other clients accordingly.
  void UpdateEventHandlerOfType(ChangeOperation,
                                const AtomicString& event_type,
                                const AddEventListenerOptions&,
                                EventTarget*);

  bool UpdateEventHandlerInternal(ChangeOperation,
                                  EventHandlerClass,
                                  EventTarget*);

  void UpdateAllEventHandlers(ChangeOperation, EventTarget&);

  void CheckConsistency(EventHandlerClass) const;

  Member<Page> page_;
  EventTargetSet targets_[kEventHandlerClassCount];
};

}  // namespace blink

#endif  // EventHandlerRegistry_h
