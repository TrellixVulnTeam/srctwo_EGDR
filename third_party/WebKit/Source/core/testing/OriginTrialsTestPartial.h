// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef OriginTrialsTestPartial_h
#define OriginTrialsTestPartial_h

#include "platform/wtf/Allocator.h"

namespace blink {

class OriginTrialsTest;

// This interface is used for testing that partial interfaces can  extend other
// interfaces at run-time, if the origin trial is enabled.
class OriginTrialsTestPartial final {
  STATIC_ONLY(OriginTrialsTestPartial);

 public:
  static bool normalAttributePartial(OriginTrialsTest&) { return true; }
  static bool staticAttributePartial() { return true; }
  static bool normalMethodPartial(OriginTrialsTest&) { return true; }
  static bool staticMethodPartial() { return true; }
  static const unsigned kConstantPartial = 2;
  static bool secureAttributePartial(OriginTrialsTest&) { return true; }
  static bool secureStaticAttributePartial() { return true; }
  static bool secureMethodPartial(OriginTrialsTest&) { return true; }
  static bool secureStaticMethodPartial() { return true; }
};

}  // namespace blink

#endif  // OriginTrialsTestPartial_h
