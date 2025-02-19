// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "HitTestCanvasResult.h"

namespace blink {

HitTestCanvasResult::HitTestCanvasResult(String id, Member<Element> control)
    : id_(id), control_(control) {}

String HitTestCanvasResult::GetId() const {
  return id_;
}

Element* HitTestCanvasResult::GetControl() const {
  return control_.Get();
};

DEFINE_TRACE(HitTestCanvasResult) {
  visitor->Trace(control_);
}

}  // namespace blink
