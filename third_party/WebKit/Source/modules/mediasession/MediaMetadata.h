// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MediaMetadata_h
#define MediaMetadata_h

#include "modules/ModulesExport.h"
#include "platform/Timer.h"
#include "platform/bindings/ScriptWrappable.h"
#include "platform/heap/Handle.h"
#include "platform/wtf/text/WTFString.h"

namespace blink {

class ExceptionState;
class MediaImage;
class MediaMetadataInit;
class MediaSession;
class ScriptState;

// Implementation of MediaMetadata interface from the Media Session API.
// The MediaMetadata object is linked to a MediaSession that owns it. When one
// of its properties are updated, the object will notify its MediaSession if
// any. The notification will be made asynchronously in order to combine changes
// made inside the same event loop. When a MediaMetadata is created and assigned
// to a MediaSession, the MediaSession will automatically update.
class MODULES_EXPORT MediaMetadata final
    : public GarbageCollectedFinalized<MediaMetadata>,
      public ScriptWrappable {
  DEFINE_WRAPPERTYPEINFO();

 public:
  static MediaMetadata* Create(ScriptState*,
                               const MediaMetadataInit&,
                               ExceptionState&);

  String title() const;
  String artist() const;
  String album() const;
  Vector<v8::Local<v8::Value>> artwork(ScriptState*) const;

  // Internal use only, returns a reference to m_artwork instead of a Frozen
  // copy of a MediaImage array.
  const HeapVector<MediaImage>& artwork() const;

  void setTitle(const String&);
  void setArtist(const String&);
  void setAlbum(const String&);
  void setArtwork(ScriptState*, const HeapVector<MediaImage>&, ExceptionState&);

  // Called by MediaSession to associate or de-associate itself.
  void SetSession(MediaSession*);

  DECLARE_VIRTUAL_TRACE();

 private:
  MediaMetadata(ScriptState*, const MediaMetadataInit&, ExceptionState&);

  // Called when one of the metadata fields is updated from script. It will
  // notify the session asynchronously in order to bundle multiple call in one
  // notification.
  void NotifySessionAsync();

  // Called asynchronously after at least one field of MediaMetadata has been
  // modified.
  void NotifySessionTimerFired(TimerBase*);

  // Make an internal copy of the MediaImage vector with some internal steps
  // such as parsing of the src property.
  void SetArtworkInternal(ScriptState*,
                          const HeapVector<MediaImage>&,
                          ExceptionState&);

  String title_;
  String artist_;
  String album_;
  HeapVector<MediaImage> artwork_;

  Member<MediaSession> session_;
  TaskRunnerTimer<MediaMetadata> notify_session_timer_;
};

}  // namespace blink

#endif  // MediaMetadata_h
