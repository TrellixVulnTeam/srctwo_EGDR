// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/audio/audio_features.h"

namespace features {

#if defined(OS_CHROMEOS)
// Allows experimentally enables mediaDevices.enumerateDevices() on ChromeOS.
// Default disabled (crbug.com/554168).
const base::Feature kEnumerateAudioDevices{"EnumerateAudioDevices",
                                           base::FEATURE_ENABLED_BY_DEFAULT};
#endif

}  // namespace features
