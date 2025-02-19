// Copyright (c) 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/quic/core/frames/quic_goaway_frame.h"

using std::string;

namespace net {

QuicGoAwayFrame::QuicGoAwayFrame()
    : error_code(QUIC_NO_ERROR), last_good_stream_id(0) {}

QuicGoAwayFrame::QuicGoAwayFrame(QuicErrorCode error_code,
                                 QuicStreamId last_good_stream_id,
                                 const string& reason)
    : error_code(error_code),
      last_good_stream_id(last_good_stream_id),
      reason_phrase(reason) {}

std::ostream& operator<<(std::ostream& os,
                         const QuicGoAwayFrame& goaway_frame) {
  os << "{ error_code: " << goaway_frame.error_code
     << ", last_good_stream_id: " << goaway_frame.last_good_stream_id
     << ", reason_phrase: '" << goaway_frame.reason_phrase << "' }\n";
  return os;
}

}  // namespace net
