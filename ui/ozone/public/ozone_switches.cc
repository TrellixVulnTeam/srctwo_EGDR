// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/ozone/public/ozone_switches.h"

namespace switches {

// Specify ozone platform implementation to use.
const char kOzonePlatform[] = "ozone-platform";

// Specify location for image dumps.
const char kOzoneDumpFile[] = "ozone-dump-file";

// Try to enable drm atomic. This works only with drm platform.
const char kEnableDrmAtomic[] = "enable-drm-atomic";

}  // namespace switches
