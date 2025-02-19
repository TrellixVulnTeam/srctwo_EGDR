// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_CAST_TEST_LINUX_OUTPUT_WINDOW_H_
#define MEDIA_CAST_TEST_LINUX_OUTPUT_WINDOW_H_

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <memory>
#include <string>

#include "base/memory/ref_counted.h"

namespace media {
class VideoFrame;
}

namespace media {
namespace cast {
namespace test {

class LinuxOutputWindow {
 public:
  LinuxOutputWindow(int x_pos,
                    int y_pos,
                    int width,
                    int height,
                    const std::string& name);
  virtual ~LinuxOutputWindow();

  void RenderFrame(const scoped_refptr<media::VideoFrame>& video_frame);

 private:
  void CreateWindow(int x_pos,
                    int y_pos,
                    int width,
                    int height,
                    const std::string& name);
  Display* display_;
  Window window_;
  XShmSegmentInfo shminfo_;
  GC gc_;
  XImage* image_;
};

}  // namespace test
}  // namespace cast
}  // namespace media

#endif  // MEDIA_CAST_TEST_LINUX_OUTPUT_WINDOW_H_
