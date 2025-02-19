// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/LayoutUnit.h"

#include <ostream>
#include "platform/wtf/text/WTFString.h"

namespace blink {

String LayoutUnit::ToString() const {
  if (value_ == LayoutUnit::Max().RawValue())
    return "LayoutUnit::max(" + String::Number(ToDouble()) + ")";
  if (value_ == LayoutUnit::Min().RawValue())
    return "LayoutUnit::min(" + String::Number(ToDouble()) + ")";
  if (value_ == LayoutUnit::NearlyMax().RawValue())
    return "LayoutUnit::nearlyMax(" + String::Number(ToDouble()) + ")";
  if (value_ == LayoutUnit::NearlyMin().RawValue())
    return "LayoutUnit::nearlyMin(" + String::Number(ToDouble()) + ")";
  return String::Number(ToDouble());
}

std::ostream& operator<<(std::ostream& stream, const LayoutUnit& value) {
  return stream << value.ToString();
}

}  // namespace blink
