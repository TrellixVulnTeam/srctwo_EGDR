// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "modules/mediasource/TrackDefault.h"

#include "bindings/core/v8/ExceptionState.h"
#include "bindings/core/v8/ToV8ForCore.h"
#include "core/html/track/AudioTrack.h"
#include "core/html/track/TextTrack.h"
#include "core/html/track/VideoTrack.h"
#include "platform/bindings/ScriptState.h"

namespace blink {

const AtomicString& TrackDefault::AudioKeyword() {
  DEFINE_STATIC_LOCAL(const AtomicString, audio, ("audio"));
  return audio;
}

const AtomicString& TrackDefault::VideoKeyword() {
  DEFINE_STATIC_LOCAL(const AtomicString, video, ("video"));
  return video;
}

const AtomicString& TrackDefault::TextKeyword() {
  DEFINE_STATIC_LOCAL(const AtomicString, text, ("text"));
  return text;
}

ScriptValue TrackDefault::kinds(ScriptState* script_state) const {
  return ScriptValue(script_state, ToV8(kinds_, script_state));
}

TrackDefault* TrackDefault::Create(const AtomicString& type,
                                   const String& language,
                                   const String& label,
                                   const Vector<String>& kinds,
                                   const String& byte_stream_track_id,
                                   ExceptionState& exception_state) {
  // Per 11 Nov 2014 Editor's Draft
  // https://dvcs.w3.org/hg/html-media/raw-file/tip/media-source/media-source.html#idl-def-TrackDefault
  // with expectation that
  // https://www.w3.org/Bugs/Public/show_bug.cgi?id=27352 will be fixed soon:
  // When this method is invoked, the user agent must run the following steps:
  // 1. if |language| is not an empty string and |language| is not a BCP 47
  //    language tag, then throw an INVALID_ACCESS_ERR and abort these steps.
  // FIXME: Implement BCP 47 language tag validation.

  if (type == AudioKeyword()) {
    // 2.1. If |type| equals "audio":
    //      If any string in |kinds| contains a value that is not listed as
    //      applying to audio in the kind categories table, then throw a
    //      TypeError and abort these steps.
    for (const String& kind : kinds) {
      if (!AudioTrack::IsValidKindKeyword(kind)) {
        exception_state.ThrowTypeError("Invalid audio track default kind '" +
                                       kind + "'");
        return nullptr;
      }
    }
  } else if (type == VideoKeyword()) {
    // 2.2. If |type| equals "video":
    //      If any string in |kinds| contains a value that is not listed as
    //      applying to video in the kind categories table, then throw a
    //      TypeError and abort these steps.
    for (const String& kind : kinds) {
      if (!VideoTrack::IsValidKindKeyword(kind)) {
        exception_state.ThrowTypeError("Invalid video track default kind '" +
                                       kind + "'");
        return nullptr;
      }
    }
  } else if (type == TextKeyword()) {
    // 2.3. If |type| equals "text":
    //      If any string in |kinds| contains a value that is not listed in the
    //      text track kind list, then throw a TypeError and abort these
    //      steps.
    for (const String& kind : kinds) {
      if (!TextTrack::IsValidKindKeyword(kind)) {
        exception_state.ThrowTypeError("Invalid text track default kind '" +
                                       kind + "'");
        return nullptr;
      }
    }
  } else {
    NOTREACHED();  // IDL enforcement should prevent this case.
    return nullptr;
  }

  // 3. Set the type attribute on this new object to |type|.
  // 4. Set the language attribute on this new object to |language|.
  // 5. Set the label attribute on this new object to |label|.
  // 6. Set the kinds attribute on this new object to |kinds|.
  // 7. Set the byteStreamTrackID attribute on this new object to
  //    |byteStreamTrackID|.
  // These steps are done as constructor initializers.
  return new TrackDefault(type, language, label, kinds, byte_stream_track_id);
}

TrackDefault::~TrackDefault() {}

TrackDefault::TrackDefault(const AtomicString& type,
                           const String& language,
                           const String& label,
                           const Vector<String>& kinds,
                           const String& byte_stream_track_id)
    : type_(type),
      byte_stream_track_id_(byte_stream_track_id),
      language_(language),
      label_(label),
      kinds_(kinds) {}

}  // namespace blink
