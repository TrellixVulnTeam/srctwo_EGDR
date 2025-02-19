// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SizeListPropertyFunctions_h
#define SizeListPropertyFunctions_h

#include "core/CSSPropertyNames.h"
#include "core/style/FillLayer.h"
#include "platform/wtf/Vector.h"

namespace blink {

class ComputedStyle;

using SizeList = Vector<FillSize, 1>;

class SizeListPropertyFunctions {
  STATIC_ONLY(SizeListPropertyFunctions);

 public:
  static SizeList GetInitialSizeList(CSSPropertyID);
  static SizeList GetSizeList(CSSPropertyID, const ComputedStyle&);
  static void SetSizeList(CSSPropertyID, ComputedStyle&, const SizeList&);
};

}  // namespace blink

#endif  // SizeListPropertyFunctions_h
