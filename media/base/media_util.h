// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_BASE_MEDIA_UTIL_H_
#define MEDIA_BASE_MEDIA_UTIL_H_

#include <stdint.h>
#include <vector>

#include "media/base/encryption_scheme.h"
#include "media/base/media_export.h"

namespace media {

// Simply returns an empty vector. {Audio|Video}DecoderConfig are often
// constructed with empty extra data.
MEDIA_EXPORT std::vector<uint8_t> EmptyExtraData();

// The following helper functions return new instances of EncryptionScheme that
// indicate widely used settings.
MEDIA_EXPORT EncryptionScheme Unencrypted();
MEDIA_EXPORT EncryptionScheme AesCtrEncryptionScheme();

}  // namespace media

#endif  // MEDIA_BASE_MEDIA_UTIL_H_
