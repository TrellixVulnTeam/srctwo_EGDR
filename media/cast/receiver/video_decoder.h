// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_CAST_RECEIVER_VIDEO_DECODER_H_
#define MEDIA_CAST_RECEIVER_VIDEO_DECODER_H_

#include <memory>

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "media/base/video_frame.h"
#include "media/cast/constants.h"
#include "media/cast/net/cast_transport_config.h"

namespace media {
namespace cast {

class CastEnvironment;

class VideoDecoder {
 public:
  // Callback passed to DecodeFrame, to deliver a decoded video frame from the
  // decoder.  |frame| can be NULL when errors occur.  |is_continuous| is
  // normally true, but will be false if the decoder has detected a frame skip
  // since the last decode operation; and the client might choose to take steps
  // to smooth/interpolate video discontinuities in this case.
  typedef base::Callback<void(const scoped_refptr<VideoFrame>& frame,
                              bool is_continuous)> DecodeFrameCallback;

  VideoDecoder(const scoped_refptr<CastEnvironment>& cast_environment,
               Codec codec);
  virtual ~VideoDecoder();

  // Returns STATUS_INITIALIZED if the decoder was successfully constructed.  If
  // this method returns any other value, calls to DecodeFrame() will not
  // succeed.
  OperationalStatus InitializationResult() const;

  // Decode the payload in |encoded_frame| asynchronously.  |callback| will be
  // invoked on the CastEnvironment::MAIN thread with the result.
  //
  // In the normal case, |encoded_frame->frame_id| will be
  // monotonically-increasing by 1 for each successive call to this method.
  // When it is not, the decoder will assume one or more frames have been
  // dropped (e.g., due to packet loss), and will perform recovery actions.
  void DecodeFrame(std::unique_ptr<EncodedFrame> encoded_frame,
                   const DecodeFrameCallback& callback);

 private:
  class FakeImpl;
  class ImplBase;
  class Vp8Impl;

  const scoped_refptr<CastEnvironment> cast_environment_;
  scoped_refptr<ImplBase> impl_;

  DISALLOW_COPY_AND_ASSIGN(VideoDecoder);
};

}  // namespace cast
}  // namespace media

#endif  // MEDIA_CAST_RECEIVER_VIDEO_DECODER_H_
