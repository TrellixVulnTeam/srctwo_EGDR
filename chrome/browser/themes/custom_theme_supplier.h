// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_THEMES_CUSTOM_THEME_SUPPLIER_H_
#define CHROME_BROWSER_THEMES_CUSTOM_THEME_SUPPLIER_H_

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/ref_counted_delete_on_sequence.h"
#include "content/public/browser/browser_thread.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/base/layout.h"

namespace base {
class RefCountedMemory;
}

namespace color_utils {
struct HSL;
}

namespace gfx {
class Image;
}

// A representation of a theme. All theme properties can be accessed through the
// public methods. Subclasses are expected to override all methods which should
// provide non-default values.
class CustomThemeSupplier
    : public base::RefCountedDeleteOnSequence<CustomThemeSupplier> {
 public:
  enum ThemeType {
    EXTENSION,
    NATIVE_X11,
    SUPERVISED_USER_THEME,
  };

  explicit CustomThemeSupplier(ThemeType type);

  ThemeType get_theme_type() const {
    return theme_type_;
  }

  // Called when the theme starts being used.
  virtual void StartUsingTheme();

  // Called when the theme is not used anymore.
  virtual void StopUsingTheme();

  // If the theme specifies data for the corresponding |id|, returns true and
  // writes the corresponding value to the output parameter. These methods
  // should not return the default data. These methods should only be called
  // from the UI thread.
  virtual bool GetTint(int id, color_utils::HSL* hsl) const;
  virtual bool GetColor(int id, SkColor* color) const;
  virtual bool GetDisplayProperty(int id, int* result) const;

  // Returns the theme image for |id|. Returns an empty image if no image is
  // found for |id|.
  // TODO(estade): Remove this function; it's not used in Material Design.
  virtual gfx::Image GetImageNamed(int id);

  // Returns the raw PNG encoded data for IDR_THEME_NTP_*. This method only
  // works for the NTP attribution and background resources.
  virtual base::RefCountedMemory* GetRawData(
      int id, ui::ScaleFactor scale_factor) const;

  // Whether this theme provides an image for |id|.
  virtual bool HasCustomImage(int id) const;

 protected:
  virtual ~CustomThemeSupplier();

 private:
  friend class base::RefCountedDeleteOnSequence<CustomThemeSupplier>;
  friend class base::DeleteHelper<CustomThemeSupplier>;

  ThemeType theme_type_;

  DISALLOW_COPY_AND_ASSIGN(CustomThemeSupplier);
};

#endif  // CHROME_BROWSER_THEMES_CUSTOM_THEME_SUPPLIER_H_
