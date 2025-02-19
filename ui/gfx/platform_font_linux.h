// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_PLATFORM_FONT_LINUX_H_
#define UI_GFX_PLATFORM_FONT_LINUX_H_

#include <memory>
#include <string>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "build/build_config.h"
#include "third_party/skia/include/core/SkTypeface.h"
#include "ui/gfx/font_render_params.h"
#include "ui/gfx/platform_font.h"

namespace gfx {

class GFX_EXPORT PlatformFontLinux : public PlatformFont {
 public:
  // TODO(derat): Get rid of the default constructor in favor of using
  // FontList (which also has the concept of a default font but may contain
  // multiple font families) everywhere. See http://crbug.com/398885#c16.
  PlatformFontLinux();
  PlatformFontLinux(const std::string& font_name, int font_size_pixels);

  // Initials the default PlatformFont. Returns true if this is successful, or
  // false if fonts resources are not available. If this returns false, the
  // calling service should shut down.
  static bool InitDefaultFont();

  // Resets and reloads the cached system font used by the default constructor.
  // This function is useful when the system font has changed, for example, when
  // the locale has changed.
  static void ReloadDefaultFont();

#if defined(OS_CHROMEOS)
  // Sets the default font. |font_description| is a FontList font description;
  // only the first family will be used.
  static void SetDefaultFontDescription(const std::string& font_description);
#endif

  // Overridden from PlatformFont:
  Font DeriveFont(int size_delta,
                  int style,
                  Font::Weight weight) const override;
  int GetHeight() override;
  Font::Weight GetWeight() const override;
  int GetBaseline() override;
  int GetCapHeight() override;
  int GetExpectedTextWidth(int length) override;
  int GetStyle() const override;
  const std::string& GetFontName() const override;
  std::string GetActualFontNameForTesting() const override;
  int GetFontSize() const override;
  const FontRenderParams& GetFontRenderParams() override;

  sk_sp<SkTypeface> typeface() const { return typeface_; }

 private:
  // Create a new instance of this object with the specified properties. Called
  // from DeriveFont.
  PlatformFontLinux(sk_sp<SkTypeface> typeface,
                    const std::string& family,
                    int size_pixels,
                    int style,
                    Font::Weight weight,
                    const FontRenderParams& params);
  ~PlatformFontLinux() override;

  // Initializes this object based on the passed-in details. If |typeface| is
  // empty, a new typeface will be loaded.
  void InitFromDetails(
      sk_sp<SkTypeface> typeface,
      const std::string& font_family,
      int font_size_pixels,
      int style,
      Font::Weight weight,
      const FontRenderParams& params);

  // Initializes this object as a copy of another PlatformFontLinux.
  void InitFromPlatformFont(const PlatformFontLinux* other);

  // Computes the metrics if they have not yet been computed.
  void ComputeMetricsIfNecessary();

  sk_sp<SkTypeface> typeface_;

  // Additional information about the face.
  // Skia actually expects a family name and not a font name.
  std::string font_family_;
  int font_size_pixels_;
  int style_;
  float device_scale_factor_;

  // Information describing how the font should be rendered.
  FontRenderParams font_render_params_;

  // Cached metrics, generated on demand.
  bool metrics_need_computation_ = true;
  int ascent_pixels_;
  int height_pixels_;
  int cap_height_pixels_;
  double average_width_pixels_;
  Font::Weight weight_;

#if defined(OS_CHROMEOS)
  // A font description string of the format used by FontList.
  static std::string* default_font_description_;
#endif

  DISALLOW_COPY_AND_ASSIGN(PlatformFontLinux);
};

}  // namespace gfx

#endif  // UI_GFX_PLATFORM_FONT_LINUX_H_
