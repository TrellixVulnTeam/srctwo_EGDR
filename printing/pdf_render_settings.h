// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PRINTING_PDF_RENDER_SETTINGS_H_
#define PRINTING_PDF_RENDER_SETTINGS_H_

#include "build/build_config.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/rect.h"

namespace printing {

// Defining PDF rendering settings.
struct PdfRenderSettings {
  enum Mode {
    NORMAL = 0,
#if defined(OS_WIN)
    TEXTONLY,
    GDI_TEXT,
    POSTSCRIPT_LEVEL2,
    POSTSCRIPT_LEVEL3,
    LAST = POSTSCRIPT_LEVEL3,
#else
    LAST = NORMAL,
#endif
  };

  PdfRenderSettings() : dpi(0), autorotate(false), mode(Mode::NORMAL) {}
  PdfRenderSettings(const gfx::Rect& area,
                    const gfx::Point& offsets,
                    int dpi,
                    bool autorotate,
                    Mode mode)
      : area(area),
        offsets(offsets),
        dpi(dpi),
        autorotate(autorotate),
        mode(mode) {}
  ~PdfRenderSettings() {}

  gfx::Rect area;
  gfx::Point offsets;
  int dpi;
  bool autorotate;
  Mode mode;
};

}  // namespace printing

#endif  // PRINTING_PDF_RENDER_SETTINGS_H_
