// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebRTCRtpContributingSource_h
#define WebRTCRtpContributingSource_h

#include "WebCommon.h"

namespace blink {

enum class WebRTCRtpContributingSourceType {
  SSRC,
  CSRC,
};

// https://w3c.github.io/webrtc-pc/#dom-rtcrtpcontributingsource
class BLINK_PLATFORM_EXPORT WebRTCRtpContributingSource {
 public:
  virtual ~WebRTCRtpContributingSource();

  virtual WebRTCRtpContributingSourceType SourceType() const = 0;
  virtual double TimestampMs() const = 0;
  virtual uint32_t Source() const = 0;
};

}  // namespace blink

#endif  // WebRTCRtpContributingSource_h
