// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_CAPTURE_VIDEO_VIDEO_FRAME_RECEIVER_ON_TASK_RUNNER_H_
#define MEDIA_CAPTURE_VIDEO_VIDEO_FRAME_RECEIVER_ON_TASK_RUNNER_H_

#include "media/capture/video/video_frame_receiver.h"

namespace base {
class SingleThreadTaskRunner;
}

namespace media {

// Decorator for VideoFrameReceiver that forwards all incoming calls to the
// given |task_runner|.
class CAPTURE_EXPORT VideoFrameReceiverOnTaskRunner
    : public VideoFrameReceiver {
 public:
  explicit VideoFrameReceiverOnTaskRunner(
      const base::WeakPtr<VideoFrameReceiver>& receiver,
      scoped_refptr<base::SingleThreadTaskRunner> task_runner);
  ~VideoFrameReceiverOnTaskRunner() override;

  void OnNewBufferHandle(
      int buffer_id,
      std::unique_ptr<VideoCaptureDevice::Client::Buffer::HandleProvider>
          handle_provider) override;
  void OnFrameReadyInBuffer(
      int buffer_id,
      int frame_feedback_id,
      std::unique_ptr<
          VideoCaptureDevice::Client::Buffer::ScopedAccessPermission>
          buffer_read_permission,
      mojom::VideoFrameInfoPtr frame_info) override;
  void OnBufferRetired(int buffer_id) override;
  void OnError() override;
  void OnLog(const std::string& message) override;
  void OnStarted() override;
  void OnStartedUsingGpuDecode() override;

 private:
  const base::WeakPtr<VideoFrameReceiver> receiver_;
  const scoped_refptr<base::SingleThreadTaskRunner> task_runner_;
};

}  // namespace media

#endif  // MEDIA_CAPTURE_VIDEO_VIDEO_FRAME_RECEIVER_ON_TASK_RUNNER_H_
