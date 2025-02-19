/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 * Copyright (C) 2011 Ericsson AB. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MediaStream_h
#define MediaStream_h

#include "core/html/URLRegistry.h"
#include "modules/EventTargetModules.h"
#include "modules/ModulesExport.h"
#include "modules/mediastream/MediaStreamTrack.h"
#include "platform/Timer.h"
#include "platform/mediastream/MediaStreamDescriptor.h"

namespace blink {

class ExceptionState;
class ExecutionContext;
class ScriptState;

class MODULES_EXPORT MediaStreamObserver : public GarbageCollectedMixin {
 public:
  virtual ~MediaStreamObserver() {}

  // Invoked when |MediaStream::addTrack| is called.
  virtual void OnStreamAddTrack(MediaStream*, MediaStreamTrack*) = 0;
  // Invoked when |MediaStream::removeTrack| is called.
  virtual void OnStreamRemoveTrack(MediaStream*, MediaStreamTrack*) = 0;

  DEFINE_INLINE_VIRTUAL_TRACE() {}
};

class MODULES_EXPORT MediaStream final : public EventTargetWithInlineData,
                                         public ContextClient,
                                         public URLRegistrable,
                                         public MediaStreamDescriptorClient {
  USING_GARBAGE_COLLECTED_MIXIN(MediaStream);
  DEFINE_WRAPPERTYPEINFO();

 public:
  static MediaStream* Create(ExecutionContext*);
  static MediaStream* Create(ExecutionContext*, MediaStream*);
  static MediaStream* Create(ExecutionContext*, const MediaStreamTrackVector&);
  static MediaStream* Create(ExecutionContext*, MediaStreamDescriptor*);
  ~MediaStream() override;

  String id() const { return descriptor_->Id(); }

  void addTrack(MediaStreamTrack*, ExceptionState&);
  void removeTrack(MediaStreamTrack*, ExceptionState&);
  MediaStreamTrack* getTrackById(String);
  MediaStream* clone(ScriptState*);

  MediaStreamTrackVector getAudioTracks() const { return audio_tracks_; }
  MediaStreamTrackVector getVideoTracks() const { return video_tracks_; }
  MediaStreamTrackVector getTracks();

  bool active() const { return descriptor_->Active(); }

  DEFINE_ATTRIBUTE_EVENT_LISTENER(active);
  DEFINE_ATTRIBUTE_EVENT_LISTENER(inactive);
  DEFINE_ATTRIBUTE_EVENT_LISTENER(addtrack);
  DEFINE_ATTRIBUTE_EVENT_LISTENER(removetrack);

  void TrackEnded();

  void RegisterObserver(MediaStreamObserver*);
  void UnregisterObserver(MediaStreamObserver*);

  // MediaStreamDescriptorClient implementation
  void StreamEnded() override;
  void AddTrackByComponent(MediaStreamComponent*) override;
  void RemoveTrackByComponent(MediaStreamComponent*) override;

  MediaStreamDescriptor* Descriptor() const { return descriptor_; }

  // EventTarget
  const AtomicString& InterfaceName() const override;
  ExecutionContext* GetExecutionContext() const override {
    return ContextClient::GetExecutionContext();
  }

  // URLRegistrable
  URLRegistry& Registry() const override;

  DECLARE_VIRTUAL_TRACE();

 protected:
  bool AddEventListenerInternal(
      const AtomicString& event_type,
      EventListener*,
      const AddEventListenerOptionsResolved&) override;

 private:
  MediaStream(ExecutionContext*, MediaStreamDescriptor*);
  MediaStream(ExecutionContext*,
              const MediaStreamTrackVector& audio_tracks,
              const MediaStreamTrackVector& video_tracks);

  bool EmptyOrOnlyEndedTracks();

  void ScheduleDispatchEvent(Event*);
  void ScheduledEventTimerFired(TimerBase*);

  MediaStreamTrackVector audio_tracks_;
  MediaStreamTrackVector video_tracks_;
  Member<MediaStreamDescriptor> descriptor_;
  // Observers are informed when |addTrack| and |removeTrack| are called.
  HeapHashSet<WeakMember<MediaStreamObserver>> observers_;

  TaskRunnerTimer<MediaStream> scheduled_event_timer_;
  HeapVector<Member<Event>> scheduled_events_;
};

using MediaStreamVector = HeapVector<Member<MediaStream>>;

MediaStream* ToMediaStream(MediaStreamDescriptor*);

}  // namespace blink

#endif  // MediaStream_h
