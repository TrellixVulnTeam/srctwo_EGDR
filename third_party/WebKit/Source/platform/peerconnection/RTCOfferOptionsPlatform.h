// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RTCOfferOptionsPlatform_h
#define RTCOfferOptionsPlatform_h

#include "platform/heap/Handle.h"

namespace blink {

class RTCOfferOptionsPlatform final
    : public GarbageCollected<RTCOfferOptionsPlatform> {
 public:
  static RTCOfferOptionsPlatform* Create(int32_t offer_to_receive_video,
                                         int32_t offer_to_receive_audio,
                                         bool voice_activity_detection,
                                         bool ice_restart) {
    return new RTCOfferOptionsPlatform(offer_to_receive_video,
                                       offer_to_receive_audio,
                                       voice_activity_detection, ice_restart);
  }

  int32_t OfferToReceiveVideo() const { return offer_to_receive_video_; }
  int32_t OfferToReceiveAudio() const { return offer_to_receive_audio_; }
  bool VoiceActivityDetection() const { return voice_activity_detection_; }
  bool IceRestart() const { return ice_restart_; }

  DEFINE_INLINE_TRACE() {}

 private:
  RTCOfferOptionsPlatform(int32_t offer_to_receive_video,
                          int32_t offer_to_receive_audio,
                          bool voice_activity_detection,
                          bool ice_restart)
      : offer_to_receive_video_(offer_to_receive_video),
        offer_to_receive_audio_(offer_to_receive_audio),
        voice_activity_detection_(voice_activity_detection),
        ice_restart_(ice_restart) {}

  int32_t offer_to_receive_video_;
  int32_t offer_to_receive_audio_;
  bool voice_activity_detection_;
  bool ice_restart_;
};

}  // namespace blink

#endif  // RTCOfferOptionsPlatform_h
