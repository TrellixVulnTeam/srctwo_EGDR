// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_UTILITY_CLOUD_PRINT_BITMAP_IMAGE_H_
#define CHROME_UTILITY_CLOUD_PRINT_BITMAP_IMAGE_H_

#include <stdint.h>

#include <memory>

#include "base/macros.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/size.h"

namespace cloud_print {

class BitmapImage {
 public:
  enum Colorspace {
    // These are the only types PWGEncoder currently supports.
    RGBA,
    BGRA
  };

  BitmapImage(const gfx::Size& size, Colorspace colorspace);
  ~BitmapImage();

  uint8_t channels() const;
  const gfx::Size& size() const { return size_; }
  Colorspace colorspace() const { return colorspace_; }

  const uint8_t* pixel_data() const { return data_.get(); }
  uint8_t* pixel_data() { return data_.get(); }

  const uint8_t* GetPixel(const gfx::Point& point) const;

 private:
  gfx::Size size_;
  Colorspace colorspace_;
  std::unique_ptr<uint8_t[]> data_;

  DISALLOW_COPY_AND_ASSIGN(BitmapImage);
};

}  // namespace cloud_print

#endif  // CHROME_UTILITY_CLOUD_PRINT_BITMAP_IMAGE_H_
