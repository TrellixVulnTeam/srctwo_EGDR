/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
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

#ifndef SourceBuffer_h
#define SourceBuffer_h

#include <memory>
#include "core/dom/SuspendableObject.h"
#include "core/typed_arrays/ArrayBufferViewHelpers.h"
#include "modules/EventTargetModules.h"
#include "modules/mediasource/TrackDefaultList.h"
#include "platform/AsyncMethodRunner.h"
#include "platform/bindings/ActiveScriptWrappable.h"
#include "platform/weborigin/KURL.h"
#include "platform/wtf/text/WTFString.h"
#include "public/platform/WebSourceBufferClient.h"

namespace blink {

class AudioTrackList;
class DOMArrayBuffer;
class DOMArrayBufferView;
class ExceptionState;
class MediaElementEventQueue;
class MediaSource;
class TimeRanges;
class VideoTrackList;
class WebSourceBuffer;

class SourceBuffer final : public EventTargetWithInlineData,
                           public ActiveScriptWrappable<SourceBuffer>,
                           public SuspendableObject,
                           public WebSourceBufferClient {
  USING_GARBAGE_COLLECTED_MIXIN(SourceBuffer);
  DEFINE_WRAPPERTYPEINFO();
  USING_PRE_FINALIZER(SourceBuffer, Dispose);

 public:
  static SourceBuffer* Create(std::unique_ptr<WebSourceBuffer>,
                              MediaSource*,
                              MediaElementEventQueue*);
  static const AtomicString& SegmentsKeyword();
  static const AtomicString& SequenceKeyword();

  ~SourceBuffer() override;

  // SourceBuffer.idl methods
  const AtomicString& mode() const { return mode_; }
  void setMode(const AtomicString&, ExceptionState&);
  bool updating() const { return updating_; }
  TimeRanges* buffered(ExceptionState&) const;
  double timestampOffset() const;
  void setTimestampOffset(double, ExceptionState&);
  void appendBuffer(DOMArrayBuffer* data, ExceptionState&);
  void appendBuffer(NotShared<DOMArrayBufferView> data, ExceptionState&);
  void abort(ExceptionState&);
  void remove(double start, double end, ExceptionState&);
  double appendWindowStart() const;
  void setAppendWindowStart(double, ExceptionState&);
  double appendWindowEnd() const;
  void setAppendWindowEnd(double, ExceptionState&);
  DEFINE_ATTRIBUTE_EVENT_LISTENER(updatestart);
  DEFINE_ATTRIBUTE_EVENT_LISTENER(update);
  DEFINE_ATTRIBUTE_EVENT_LISTENER(updateend);
  DEFINE_ATTRIBUTE_EVENT_LISTENER(error);
  DEFINE_ATTRIBUTE_EVENT_LISTENER(abort);
  TrackDefaultList* trackDefaults() const { return track_defaults_.Get(); }
  void setTrackDefaults(TrackDefaultList*, ExceptionState&);

  AudioTrackList& audioTracks();
  VideoTrackList& videoTracks();

  void RemovedFromMediaSource();
  double HighestPresentationTimestamp();

  // ScriptWrappable
  bool HasPendingActivity() const final;

  // SuspendableObject
  void Suspend() override;
  void Resume() override;
  void ContextDestroyed(ExecutionContext*) override;

  // EventTarget interface
  ExecutionContext* GetExecutionContext() const override;
  const AtomicString& InterfaceName() const override;

  // WebSourceBufferClient interface
  bool InitializationSegmentReceived(const WebVector<MediaTrackInfo>&) override;
  void NotifyParseWarning(const ParseWarning) override;

  DECLARE_VIRTUAL_TRACE();

 private:
  SourceBuffer(std::unique_ptr<WebSourceBuffer>,
               MediaSource*,
               MediaElementEventQueue*);
  void Dispose();

  bool IsRemoved() const;
  void ScheduleEvent(const AtomicString& event_name);

  bool PrepareAppend(double media_time, size_t new_data_size, ExceptionState&);
  bool EvictCodedFrames(double media_time, size_t new_data_size);
  void AppendBufferInternal(double media_time,
                            const unsigned char*,
                            unsigned,
                            ExceptionState&);
  void AppendBufferAsyncPart();
  void AppendError();

  void RemoveAsyncPart();

  void CancelRemove();
  void AbortIfUpdating();

  void RemoveMediaTracks();

  // Returns MediaElement playback position (i.e. MediaElement.currentTime() )
  // in seconds, or NaN if media element is not available.
  double GetMediaTime();

  const TrackDefault* GetTrackDefault(
      const AtomicString& track_type,
      const AtomicString& byte_stream_track_id) const;
  AtomicString DefaultTrackLabel(
      const AtomicString& track_type,
      const AtomicString& byte_stream_track_id) const;
  AtomicString DefaultTrackLanguage(
      const AtomicString& track_type,
      const AtomicString& byte_stream_track_id) const;

  std::unique_ptr<WebSourceBuffer> web_source_buffer_;
  Member<MediaSource> source_;
  Member<TrackDefaultList> track_defaults_;
  Member<MediaElementEventQueue> async_event_queue_;

  AtomicString mode_;
  bool updating_;
  double timestamp_offset_;
  Member<AudioTrackList> audio_tracks_;
  Member<VideoTrackList> video_tracks_;
  double append_window_start_;
  double append_window_end_;
  bool first_initialization_segment_received_;

  Vector<unsigned char> pending_append_data_;
  size_t pending_append_data_offset_;
  Member<AsyncMethodRunner<SourceBuffer>> append_buffer_async_part_runner_;

  double pending_remove_start_;
  double pending_remove_end_;
  Member<AsyncMethodRunner<SourceBuffer>> remove_async_part_runner_;
};

}  // namespace blink

#endif  // SourceBuffer_h
