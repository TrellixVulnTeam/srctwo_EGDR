// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SHELL_APP_SHELL_MAIN_DELEGATE_MAC_H_
#define CONTENT_SHELL_APP_SHELL_MAIN_DELEGATE_MAC_H_

namespace content {

// Set NSHighResolutionCapable to false when running layout tests, so we match
// the expected pixel results on retina capable displays.
void EnsureCorrectResolutionSettings();

}  // namespace content

#endif  // CONTENT_SHELL_APP_SHELL_MAIN_DELEGATE_MAC_H_
