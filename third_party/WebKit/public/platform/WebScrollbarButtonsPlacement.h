// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebScrollbarButtonsPlacement_h
#define WebScrollbarButtonsPlacement_h

namespace blink {

enum WebScrollbarButtonsPlacement {
  kWebScrollbarButtonsPlacementNone,
  kWebScrollbarButtonsPlacementSingle,
  kWebScrollbarButtonsPlacementDoubleStart,
  kWebScrollbarButtonsPlacementDoubleEnd,
  kWebScrollbarButtonsPlacementDoubleBoth,
  kWebScrollbarButtonsPlacementLast = kWebScrollbarButtonsPlacementDoubleBoth
};

}  // namespace blink

#endif  // WebScrollbarButtonsPlacement_h
