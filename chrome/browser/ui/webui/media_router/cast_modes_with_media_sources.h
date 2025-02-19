// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_MEDIA_ROUTER_CAST_MODES_WITH_MEDIA_SOURCES_H_
#define CHROME_BROWSER_UI_WEBUI_MEDIA_ROUTER_CAST_MODES_WITH_MEDIA_SOURCES_H_

#include <map>
#include <unordered_set>

#include "chrome/browser/ui/webui/media_router/media_cast_mode.h"
#include "chrome/common/media_router/media_source.h"

namespace media_router {

// Contains information on cast modes and the sources associated with them.
// Each cast mode contained has at least one source.
class CastModesWithMediaSources {
 public:
  CastModesWithMediaSources();
  CastModesWithMediaSources(CastModesWithMediaSources&& other);
  ~CastModesWithMediaSources();

  // Adds a source for the cast mode.
  void AddSource(MediaCastMode cast_mode, const MediaSource& source);

  // Removes a source from the cast mode. The cast mode will also get removed if
  // it has no other sources. This is a no-op if the source is not found.
  void RemoveSource(MediaCastMode cast_mode, const MediaSource& source);

  // Returns true if the source for the cast mode is contained.
  bool HasSource(MediaCastMode cast_mode, const MediaSource& source) const;

  // Returns a set of all the cast modes contained.
  CastModeSet GetCastModes() const;

  // Returns true if there are no cast modes contained.
  bool IsEmpty() const;

 private:
  std::map<MediaCastMode, std::unordered_set<MediaSource, MediaSource::Hash>>
      cast_modes_;

  DISALLOW_COPY_AND_ASSIGN(CastModesWithMediaSources);
};

}  // namespace media_router

#endif  // CHROME_BROWSER_UI_WEBUI_MEDIA_ROUTER_CAST_MODES_WITH_MEDIA_SOURCES_H_
