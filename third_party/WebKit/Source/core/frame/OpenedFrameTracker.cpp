// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/frame/OpenedFrameTracker.h"

#include "platform/heap/Handle.h"
#include "public/web/WebFrame.h"

namespace blink {

OpenedFrameTracker::OpenedFrameTracker() {}

OpenedFrameTracker::~OpenedFrameTracker() {
  DCHECK(IsEmpty());
}

bool OpenedFrameTracker::IsEmpty() const {
  return opened_frames_.IsEmpty();
}

void OpenedFrameTracker::Add(WebFrame* frame) {
  opened_frames_.insert(frame);
}

void OpenedFrameTracker::Remove(WebFrame* frame) {
  opened_frames_.erase(frame);
}

void OpenedFrameTracker::TransferTo(WebFrame* opener) {
  // Copy the set of opened frames, since changing the owner will mutate this
  // set.
  HashSet<WebFrame*> frames(opened_frames_);
  for (WebFrame* frame : frames)
    frame->SetOpener(opener);
}

}  // namespace blink
