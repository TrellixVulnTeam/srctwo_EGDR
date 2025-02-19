// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_VERSION_INFO_VERSION_STRING_H_
#define COMPONENTS_VERSION_INFO_VERSION_STRING_H_

#include <string>

namespace version_info {

// Returns a version string to be displayed in "About Chromium" dialog.
// |modifier| is a string representation of the channel with system specific
// information, e.g. "dev SyzyASan". It is appended to the returned version
// information if non-empty.
std::string GetVersionStringWithModifier(const std::string& modifier);

}  // namespace version_info

#endif  // COMPONENTS_VERSION_INFO_VERSION_STRING_H_
