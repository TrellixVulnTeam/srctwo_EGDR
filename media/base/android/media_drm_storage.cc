// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/base/android/media_drm_storage.h"

#include <utility>

namespace media {

MediaDrmStorage::SessionData::SessionData(std::vector<uint8_t> key_set_id,
                                          std::string mime_type)
    : key_set_id(std::move(key_set_id)), mime_type(std::move(mime_type)) {}

MediaDrmStorage::SessionData::SessionData(const SessionData& other) = default;

MediaDrmStorage::SessionData::~SessionData() {}

MediaDrmStorage::MediaDrmStorage() {}

MediaDrmStorage::~MediaDrmStorage() {}

}  // namespace media
