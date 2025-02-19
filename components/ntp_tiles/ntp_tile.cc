// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/ntp_tiles/ntp_tile.h"

namespace ntp_tiles {

NTPTile::NTPTile() : source(TileSource::TOP_SITES) {}

NTPTile::NTPTile(const NTPTile&) = default;

NTPTile::~NTPTile() {}

bool operator==(const NTPTile& a, const NTPTile& b) {
  return (a.title == b.title) && (a.url == b.url) && (a.source == b.source) &&
         (a.whitelist_icon_path == b.whitelist_icon_path) &&
         (a.thumbnail_url == b.thumbnail_url) &&
         (a.favicon_url == b.favicon_url);
}

bool operator!=(const NTPTile& a, const NTPTile& b) {
  return !(a == b);
}

}  // namespace ntp_tiles
