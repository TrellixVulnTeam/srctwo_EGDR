// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_ANDROID_CONTENT_STARTUP_FLAGS_H_
#define CONTENT_BROWSER_ANDROID_CONTENT_STARTUP_FLAGS_H_

#include <string>

namespace content {

// Force-appends flags to the command line turning on Android-specific
// features owned by Content. This is called as soon as possible during
// initialization to make sure code sees the new flags.
void SetContentCommandLineFlags(bool single_process,
                                const std::string& plugin_descriptor);

}  // namespace content

#endif  // CONTENT_BROWSER_ANDROID_CONTENT_STARTUP_FLAGS_H_
