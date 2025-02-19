// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_VR_UI_UNSUPPORTED_MODE_H_
#define CHROME_BROWSER_VR_UI_UNSUPPORTED_MODE_H_

namespace vr {

// Ensure that this stays in sync with VRUnsupportedMode in enums.xml
// These values are written to logs.  New enum values can be added, but existing
// enums must never be renumbered or deleted and reused.
// GENERATED_JAVA_ENUM_PACKAGE: org.chromium.chrome.browser.vr_shell
enum class UiUnsupportedMode : int {
  kUnhandledCodePoint = 0,
  kCouldNotElideURL,
  kUnhandledPageInfo,
  kURLWithStrongRTLChars,

  // This must be last.
  kCount,
};

}  // namespace vr

#endif  // CHROME_BROWSER_VR_UI_UNSUPPORTED_MODE_H_
