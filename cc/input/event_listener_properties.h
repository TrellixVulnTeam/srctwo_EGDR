// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_INPUT_EVENT_LISTENER_PROPERTIES_H_
#define CC_INPUT_EVENT_LISTENER_PROPERTIES_H_

namespace cc {

enum class EventListenerClass {
  // This value includes "touchstart", "touchmove", and "pointer" events.
  kTouchStartOrMove,
  // This value includes "wheel" and "mousewheel" events.
  kMouseWheel,
  // This value includes "touchend" and "touchcancel" events.
  kTouchEndOrCancel,
  kNumClasses
};

enum class EventListenerProperties {
  kNone,
  kPassive,
  kBlocking,
  kBlockingAndPassive,
  kMax
};

}  // namespace cc

#endif  // CC_INPUT_EVENT_LISTENER_PROPERTIES_H_
