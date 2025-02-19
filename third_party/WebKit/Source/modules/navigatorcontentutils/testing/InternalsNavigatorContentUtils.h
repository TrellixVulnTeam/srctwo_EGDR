// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef InternalsNavigatorContentUtils_h
#define InternalsNavigatorContentUtils_h

#include "platform/wtf/Allocator.h"

namespace blink {

class Document;
class Internals;

class InternalsNavigatorContentUtils {
  STATIC_ONLY(InternalsNavigatorContentUtils);

 public:
  static void setNavigatorContentUtilsClientMock(Internals&, Document*);
};

}  // namespace blink

#endif  // InternalsNavigatorContentUtils_h
