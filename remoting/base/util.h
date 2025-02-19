// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_BASE_UTIL_H_
#define REMOTING_BASE_UTIL_H_

#include <stddef.h>
#include <stdint.h>
#include <string>

#include "third_party/webrtc/modules/desktop_capture/desktop_geometry.h"

namespace remoting {

// Return a string that contains the current date formatted as 'MMDD/HHMMSS:'.
std::string GetTimestampString();

int RoundToTwosMultiple(int x);

// Align the sides of the rectangle to multiples of 2 (expanding outwards).
webrtc::DesktopRect AlignRect(const webrtc::DesktopRect& rect);

// Copy content of a rectangle in a RGB32 image.
void CopyRGB32Rect(const uint8_t* source_buffer,
                   int source_stride,
                   const webrtc::DesktopRect& source_buffer_rect,
                   uint8_t* dest_buffer,
                   int dest_stride,
                   const webrtc::DesktopRect& dest_buffer_rect,
                   const webrtc::DesktopRect& dest_rect);

// Replaces every occurrence of "\n" in a string by "\r\n".
std::string ReplaceLfByCrLf(const std::string& in);

// Replaces every occurrence of "\r\n" in a string by "\n".
std::string ReplaceCrLfByLf(const std::string& in);

// Checks if the given string is a valid UTF-8 string.
bool StringIsUtf8(const char* data, size_t length);

bool DoesRectContain(const webrtc::DesktopRect& a,
                     const webrtc::DesktopRect& b);

}  // namespace remoting

#endif  // REMOTING_BASE_UTIL_H_
