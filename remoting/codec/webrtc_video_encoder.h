// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_CODEC_WEBRTC_VIDEO_ENCODER_H_
#define REMOTING_CODEC_WEBRTC_VIDEO_ENCODER_H_

#include <stdint.h>

#include <memory>

#include "base/time/time.h"
#include "third_party/webrtc/modules/desktop_capture/desktop_geometry.h"

namespace webrtc {
class DesktopFrame;
}  // namespace webrtc

namespace remoting {

// A class to perform the task of encoding a continuous stream of images. The
// interface is asynchronous to enable maximum throughput.
class WebrtcVideoEncoder {
 public:
  struct FrameParams {
    // Target bitrate in kilobits per second.
    int bitrate_kbps = -1;

    // Frame duration.
    base::TimeDelta duration;

    // If set to true then the active map passed to the encoder will only
    // contain updated_region() from the current frame. Otherwise the active map
    // is not cleared before adding updated_region(), which means it will
    // contain union of updated_region() from all frames since this flag was
    // last set. This flag is used to top-off video quality with VP8.
    bool clear_active_map = false;

    // Indicates that the encoder should encode this frame as a key frame.
    bool key_frame = false;

    // Quantization parameters for the encoder.
    int vpx_min_quantizer = -1;
    int vpx_max_quantizer = -1;
  };

  struct EncodedFrame {
    webrtc::DesktopSize size;
    std::string data;
    bool key_frame;
    int quantizer;
  };

  typedef base::OnceCallback<void(std::unique_ptr<EncodedFrame>)>
      EncodeCallback;

  virtual ~WebrtcVideoEncoder() {}

  // Encode an image stored in |frame|. If frame.updated_region() is empty
  // then the encoder may return a frame (e.g. to top-off previously-encoded
  // portions of the frame to higher quality) or return nullptr to indicate that
  // there is no work to do. |frame| may be nullptr, which is equivalent to a
  // frame with an empty updated_region(). |done| callback may be called
  // synchronously. It must not be called if the encoder is destroyed while
  // request is pending.
  virtual void Encode(std::unique_ptr<webrtc::DesktopFrame> frame,
                      const FrameParams& param,
                      EncodeCallback done) = 0;
};

}  // namespace remoting

#endif  // REMOTING_CODEC_WEBRTC_VIDEO_ENCODER_H_
