// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CompositionMarker_h
#define CompositionMarker_h

#include "core/editing/markers/StyleableMarker.h"

namespace blink {

// A subclass of DocumentMarker used to store information specific to
// composition markers. We store what color to display the underline (possibly
// transparent), whether the underline should be thick or not, and what
// background color should be used under the marked text (also possibly
// transparent).
class CORE_EXPORT CompositionMarker final : public StyleableMarker {
 public:
  CompositionMarker(unsigned start_offset,
                    unsigned end_offset,
                    Color underline_color,
                    Thickness,
                    Color background_color);

  // DocumentMarker implementations
  MarkerType GetType() const final;

 private:
  DISALLOW_COPY_AND_ASSIGN(CompositionMarker);
};

DEFINE_TYPE_CASTS(CompositionMarker,
                  DocumentMarker,
                  marker,
                  marker->GetType() == DocumentMarker::kComposition,
                  marker.GetType() == DocumentMarker::kComposition);

}  // namespace blink

#endif
