// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EventSourceParser_h
#define EventSourceParser_h

#include <memory>
#include "modules/ModulesExport.h"
#include "platform/heap/Handle.h"
#include "platform/wtf/Vector.h"
#include "platform/wtf/text/AtomicString.h"
#include "platform/wtf/text/TextCodec.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

class MODULES_EXPORT EventSourceParser final
    : public GarbageCollectedFinalized<EventSourceParser> {
 public:
  class MODULES_EXPORT Client : public GarbageCollectedMixin {
   public:
    virtual ~Client() {}
    virtual void OnMessageEvent(const AtomicString& type,
                                const String& data,
                                const AtomicString& last_event_id) = 0;
    virtual void OnReconnectionTimeSet(
        unsigned long long reconnection_time) = 0;
    DEFINE_INLINE_VIRTUAL_TRACE() {}
  };

  EventSourceParser(const AtomicString& last_event_id, Client*);

  void AddBytes(const char*, size_t);
  const AtomicString& LastEventId() const { return last_event_id_; }
  // Stop parsing. This can be called from Client::onMessageEvent.
  void Stop() { is_stopped_ = true; }
  DECLARE_TRACE();

 private:
  void ParseLine();
  String FromUTF8(const char* bytes, size_t);

  Vector<char> line_;

  AtomicString event_type_;
  Vector<char> data_;
  // This variable corresponds to "last event ID buffer" in the spec. The
  // value can be discarded when a connection is disconnected while
  // parsing an event.
  AtomicString id_;
  // This variable corresponds to "last event ID string" in the spec.
  AtomicString last_event_id_;

  Member<Client> client_;
  std::unique_ptr<TextCodec> codec_;

  bool is_recognizing_crlf_ = false;
  bool is_recognizing_bom_ = true;
  bool is_stopped_ = false;
};

}  // namespace blink

#endif  // EventSourceParser_h
