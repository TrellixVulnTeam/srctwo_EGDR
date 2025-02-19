// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Linux specific implementation of VideoCaptureDevice.
// V4L2 is used for capturing. V4L2 does not provide its own thread for
// capturing so this implementation uses a Chromium thread for fetching frames
// from V4L2.

#ifndef MEDIA_CAPTURE_VIDEO_LINUX_VIDEO_CAPTURE_DEVICE_LINUX_H_
#define MEDIA_CAPTURE_VIDEO_LINUX_VIDEO_CAPTURE_DEVICE_LINUX_H_

#include <stdint.h>

#include <string>

#include "base/files/file_util.h"
#include "base/files/scoped_file.h"
#include "base/macros.h"
#include "base/threading/thread.h"
#include "media/capture/video/video_capture_device.h"
#include "media/capture/video_capture_types.h"

namespace media {

class V4L2CaptureDelegate;

// Linux V4L2 implementation of VideoCaptureDevice.
class VideoCaptureDeviceLinux : public VideoCaptureDevice {
 public:
  static VideoPixelFormat V4l2FourCcToChromiumPixelFormat(uint32_t v4l2_fourcc);
  static std::list<uint32_t> GetListOfUsableFourCCs(bool favour_mjpeg);

  explicit VideoCaptureDeviceLinux(
      const VideoCaptureDeviceDescriptor& device_descriptor);
  ~VideoCaptureDeviceLinux() override;

  // VideoCaptureDevice implementation.
  void AllocateAndStart(const VideoCaptureParams& params,
                        std::unique_ptr<Client> client) override;
  void StopAndDeAllocate() override;
  void TakePhoto(TakePhotoCallback callback) override;
  void GetPhotoState(GetPhotoStateCallback callback) override;
  void SetPhotoOptions(mojom::PhotoSettingsPtr settings,
                       SetPhotoOptionsCallback callback) override;

 protected:
  virtual void SetRotation(int rotation);

  const VideoCaptureDeviceDescriptor device_descriptor_;

 private:
  static int TranslatePowerLineFrequencyToV4L2(PowerLineFrequency frequency);

  // Internal delegate doing the actual capture setting, buffer allocation and
  // circulation with the V4L2 API. Created in the thread where
  // VideoCaptureDeviceLinux lives but otherwise operating and deleted on
  // |v4l2_thread_|.
  std::unique_ptr<V4L2CaptureDelegate> capture_impl_;

  // Photo-related requests waiting for |v4l2_thread_| to be active.
  std::list<base::Closure> photo_requests_queue_;

  base::Thread v4l2_thread_;  // Thread used for reading data from the device.

  DISALLOW_IMPLICIT_CONSTRUCTORS(VideoCaptureDeviceLinux);
};

}  // namespace media

#endif  // MEDIA_CAPTURE_VIDEO_LINUX_VIDEO_CAPTURE_DEVICE_LINUX_H_
