// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_CAPTURE_VIDEO_CHROMEOS_MOCK_VIDEO_CAPTURE_CLIENT_H_
#define MEDIA_CAPTURE_VIDEO_CHROMEOS_MOCK_VIDEO_CAPTURE_CLIENT_H_

#include "media/capture/video/video_capture_device.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace media {
namespace unittest_internal {

class MockVideoCaptureClient : public VideoCaptureDevice::Client {
 public:
  MOCK_METHOD0(DoReserveOutputBuffer, void(void));
  MOCK_METHOD0(DoOnIncomingCapturedBuffer, void(void));
  MOCK_METHOD0(DoOnIncomingCapturedVideoFrame, void(void));
  MOCK_METHOD0(DoResurrectLastOutputBuffer, void(void));
  MOCK_METHOD2(OnError,
               void(const tracked_objects::Location& from_here,
                    const std::string& reason));
  MOCK_CONST_METHOD0(GetBufferPoolUtilization, double(void));
  MOCK_METHOD0(OnStarted, void(void));

  explicit MockVideoCaptureClient();

  ~MockVideoCaptureClient();

  void SetFrameCb(base::OnceClosure frame_cb);

  void SetQuitCb(base::OnceClosure quit_cb);

  void DumpError(const tracked_objects::Location& location,
                 const std::string& message);

  void OnIncomingCapturedData(const uint8_t* data,
                              int length,
                              const VideoCaptureFormat& format,
                              int rotation,
                              base::TimeTicks reference_time,
                              base::TimeDelta timestamp,
                              int frame_feedback_id) override;
  // Trampoline methods to workaround GMOCK problems with std::unique_ptr<>.
  Buffer ReserveOutputBuffer(const gfx::Size& dimensions,
                             media::VideoPixelFormat format,
                             media::VideoPixelStorage storage,
                             int frame_feedback_id) override;
  void OnIncomingCapturedBuffer(Buffer buffer,
                                const VideoCaptureFormat& format,
                                base::TimeTicks reference_time,
                                base::TimeDelta timestamp) override;
  void OnIncomingCapturedBufferExt(
      Buffer buffer,
      const VideoCaptureFormat& format,
      base::TimeTicks reference_time,
      base::TimeDelta timestamp,
      gfx::Rect visible_rect,
      const VideoFrameMetadata& additional_metadata) override;
  Buffer ResurrectLastOutputBuffer(const gfx::Size& dimensions,
                                   media::VideoPixelFormat format,
                                   media::VideoPixelStorage storage,
                                   int frame_feedback_id) override;

 private:
  base::OnceClosure frame_cb_;
  base::OnceClosure quit_cb_;
};

}  // namespace unittest_internal
}  // namespace media

#endif  // MEDIA_CAPTURE_VIDEO_CHROMEOS_MOCK_VIDEO_CAPTURE_CLIENT_H_
