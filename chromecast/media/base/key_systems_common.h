// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMECAST_MEDIA_BASE_KEY_SYSTEMS_COMMON_H_
#define CHROMECAST_MEDIA_BASE_KEY_SYSTEMS_COMMON_H_

#include <string>

#include "chromecast/public/media/cast_key_system.h"

namespace chromecast {
namespace media {

#if defined(PLAYREADY_CDM_AVAILABLE)
extern const char kChromecastPlayreadyKeySystem[];
#endif  // defined(PLAYREADY_CDM_AVAILABLE)

// Translates a key system string into a CastKeySystem, calling into the
// platform for known key systems if needed.
CastKeySystem GetKeySystemByName(const std::string& key_system_name);

}  // namespace media
}  // namespace chromecast

#endif  // CHROMECAST_MEDIA_BASE_KEY_SYSTEMS_COMMON_H_
