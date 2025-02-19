// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include <memory>

#include "base/macros.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/webrtc/modules/desktop_capture/desktop_frame.h"

namespace remoting {

// DesktopFrame implementation used by screen capture on ChromeOS.
// Frame data is stored in a SkBitmap.
class SkiaBitmapDesktopFrame : public webrtc::DesktopFrame {
 public:
  static SkiaBitmapDesktopFrame* Create(std::unique_ptr<SkBitmap> bitmap);
  ~SkiaBitmapDesktopFrame() override;

 private:
  SkiaBitmapDesktopFrame(webrtc::DesktopSize size,
                         int stride,
                         uint8_t* data,
                         std::unique_ptr<SkBitmap> bitmap);

  std::unique_ptr<SkBitmap> bitmap_;

  DISALLOW_COPY_AND_ASSIGN(SkiaBitmapDesktopFrame);
};

}  // namespace remoting
