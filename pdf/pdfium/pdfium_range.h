// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PDF_PDFIUM_PDFIUM_RANGE_H_
#define PDF_PDFIUM_PDFIUM_RANGE_H_

#include <string>
#include <vector>

#include "base/strings/string16.h"
#include "pdf/pdfium/pdfium_page.h"
#include "ppapi/cpp/rect.h"

namespace chrome_pdf {

// Describes location of a string of characters.
class PDFiumRange {
 public:
  PDFiumRange(PDFiumPage* page, int char_index, int char_count);
  PDFiumRange(const PDFiumRange& that);
  ~PDFiumRange();

  // Update how many characters are in the selection.  Could be negative if
  // backwards.
  void SetCharCount(int char_count);

  int page_index() const { return page_->index(); }
  int char_index() const { return char_index_; }
  int char_count() const { return char_count_; }

  // Gets bounding rectangles of range in screen coordinates.
  std::vector<pp::Rect> GetScreenRects(const pp::Point& offset,
                                       double zoom,
                                       int rotation);

  // Gets the string of characters in this range.
  base::string16 GetText() const;

 private:
  PDFiumPage* page_;
  // Index of first character.
  int char_index_;
  // How many characters are part of this range (negative if backwards).
  int char_count_;

  // Cache of ScreenRect, and the associated variables used when caching it.
  std::vector<pp::Rect> cached_screen_rects_;
  pp::Point cached_screen_rects_offset_;
  double cached_screen_rects_zoom_;
};

}  // namespace chrome_pdf

#endif  // PDF_PDFIUM_PDFIUM_RANGE_H_
