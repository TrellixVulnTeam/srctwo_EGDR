// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_MEDIA_CAPTURE_DESKTOP_CAPTURE_DEVICE_UMA_TYPES_H_
#define CONTENT_BROWSER_MEDIA_CAPTURE_DESKTOP_CAPTURE_DEVICE_UMA_TYPES_H_

namespace content {

// This enum must be kept in-sync with DesktopCaptureCounters defined in
// histograms.xml. New fields should be added right before
// DESKTOP_CAPTURE_COUNTER_BOUNDARY.
enum DesktopCaptureCounters {
  SCREEN_CAPTURER_CREATED,
  WINDOW_CAPTURER_CREATED,
  FIRST_SCREEN_CAPTURE_SUCCEEDED,
  FIRST_SCREEN_CAPTURE_FAILED,
  FIRST_WINDOW_CAPTURE_SUCCEEDED,
  FIRST_WINDOW_CAPTURE_FAILED,
  TAB_VIDEO_CAPTURER_CREATED,
  TAB_AUDIO_CAPTURER_CREATED,
  SYSTEM_LOOPBACK_AUDIO_CAPTURER_CREATED,
  SCREEN_CAPTURER_CREATED_WITH_AUDIO,
  SCREEN_CAPTURER_CREATED_WITHOUT_AUDIO,
  TAB_VIDEO_CAPTURER_CREATED_WITH_AUDIO,
  TAB_VIDEO_CAPTURER_CREATED_WITHOUT_AUDIO,
  DESKTOP_CAPTURE_COUNTER_BOUNDARY
};

extern const char kUmaScreenCaptureTime[];
extern const char kUmaWindowCaptureTime[];

void IncrementDesktopCaptureCounter(DesktopCaptureCounters counter);

}  // namespace content

#endif  // CONTENT_BROWSER_MEDIA_CAPTURE_DESKTOP_CAPTURE_DEVICE_UMA_TYPES_H_
