// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_FILTERS_SOURCE_BUFFER_PARSE_WARNINGS_H_
#define MEDIA_FILTERS_SOURCE_BUFFER_PARSE_WARNINGS_H_

#include "base/callback_forward.h"

namespace media {

// Non-fatal parsing, coded frame processing, or buffering warning. These are
// intended to be used for telemetry reporting to better understand the
// frequency at which they occur.
enum class SourceBufferParseWarning {
  kKeyframeTimeGreaterThanDependant,  // Reported up to once per track.
  kMuxedSequenceMode,                 // Reported up to once per SourceBuffer.
};

// For reporting telemetry of a non-fatal SourceBufferParseWarning.
using SourceBufferParseWarningCB =
    base::Callback<void(SourceBufferParseWarning)>;

}  // namespace media

#endif  // MEDIA_FILTERS_SOURCE_BUFFER_PARSE_WARNINGS_H_
