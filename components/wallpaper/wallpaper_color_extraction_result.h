// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_WALLPAPER_WALLPAPER_COLOR_EXTRACTION_RESULT_H_
#define COMPONENTS_WALLPAPER_WALLPAPER_COLOR_EXTRACTION_RESULT_H_

namespace wallpaper {

// This enum is used to back a histogram, and should therefore be treated as
// append-only.
// For result, transparent indicates extraction failure and opaque indicates
// extraction success.
enum WallpaperColorExtractionResult {
  // Transparent result on (dark, vibrant) color profile.
  RESULT_DARK_VIBRANT_TRANSPARENT = 0,
  // Opaque result on (dark, vibrant) color profile.
  RESULT_DARK_VIBRANT_OPAQUE,
  // Transparent result on (normal, vibrant) color profile.
  RESULT_NORMAL_VIBRANT_TRANSPARENT,
  // Opaque result on (normal, vibrant) color profile.
  RESULT_NORMAL_VIBRANT_OPAQUE,
  // Transparent result on (light, vibrant) color profile.
  RESULT_LIGHT_VIBRANT_TRANSPARENT,
  // Opaque result on (light, vibrant) color profile.
  RESULT_LIGHT_VIBRANT_OPAQUE,
  // Transparent result on (dark, muted) color profile.
  RESULT_DARK_MUTED_TRANSPARENT,
  // Opaque result on (dark, muted) color profile.
  RESULT_DARK_MUTED_OPAQUE,
  // Transparent result on (normal, muted) color profile.
  RESULT_NORMAL_MUTED_TRANSPARENT,
  // Opaque result on (normal, muted) color profile.
  RESULT_NORMAL_MUTED_OPAQUE,
  // Transparent result on (light, muted) color profile.
  RESULT_LIGHT_MUTED_TRANSPARENT,
  // Opaque result on (light, muted) color profile.
  RESULT_LIGHT_MUTED_OPAQUE,

  NUM_COLOR_EXTRACTION_RESULTS,
};

}  // namespace wallpaper

#endif  // COMPONENTS_WALLPAPER_WALLPAPER_COLOR_EXTRACTION_RESULT_H_
