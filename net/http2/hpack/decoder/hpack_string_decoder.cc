// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/http2/hpack/decoder/hpack_string_decoder.h"

#include "net/http2/platform/api/http2_string_utils.h"

namespace net {

Http2String HpackStringDecoder::DebugString() const {
  return Http2StrCat("HpackStringDecoder(state=", StateToString(state_),
                     ", length=", length_decoder_.DebugString(),
                     ", remaining=", remaining_,
                     ", huffman=", huffman_encoded_ ? "true)" : "false)");
}

// static
Http2String HpackStringDecoder::StateToString(StringDecoderState v) {
  switch (v) {
    case kStartDecodingLength:
      return "kStartDecodingLength";
    case kDecodingString:
      return "kDecodingString";
    case kResumeDecodingLength:
      return "kResumeDecodingLength";
  }
  return Http2StrCat("UNKNOWN_STATE(", static_cast<uint32_t>(v), ")");
}

std::ostream& operator<<(std::ostream& out, const HpackStringDecoder& v) {
  return out << v.DebugString();
}

}  // namespace net
