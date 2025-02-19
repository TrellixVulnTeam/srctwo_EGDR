// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/html/custom/V0CustomElementMicrotaskRunQueue.h"

#include "core/html/custom/V0CustomElementAsyncImportMicrotaskQueue.h"
#include "core/html/custom/V0CustomElementSyncMicrotaskQueue.h"
#include "core/html/imports/HTMLImportLoader.h"
#include "platform/bindings/Microtask.h"

namespace blink {

V0CustomElementMicrotaskRunQueue::V0CustomElementMicrotaskRunQueue()
    : sync_queue_(V0CustomElementSyncMicrotaskQueue::Create()),
      async_queue_(V0CustomElementAsyncImportMicrotaskQueue::Create()),
      dispatch_is_pending_(false) {}

void V0CustomElementMicrotaskRunQueue::Enqueue(
    HTMLImportLoader* parent_loader,
    V0CustomElementMicrotaskStep* step,
    bool import_is_sync) {
  if (import_is_sync) {
    if (parent_loader)
      parent_loader->MicrotaskQueue()->Enqueue(step);
    else
      sync_queue_->Enqueue(step);
  } else {
    async_queue_->Enqueue(step);
  }

  RequestDispatchIfNeeded();
}

void V0CustomElementMicrotaskRunQueue::RequestDispatchIfNeeded() {
  if (dispatch_is_pending_ || IsEmpty())
    return;
  Microtask::EnqueueMicrotask(WTF::Bind(
      &V0CustomElementMicrotaskRunQueue::Dispatch, WrapWeakPersistent(this)));
  dispatch_is_pending_ = true;
}

DEFINE_TRACE(V0CustomElementMicrotaskRunQueue) {
  visitor->Trace(sync_queue_);
  visitor->Trace(async_queue_);
}

void V0CustomElementMicrotaskRunQueue::Dispatch() {
  dispatch_is_pending_ = false;
  sync_queue_->Dispatch();
  if (sync_queue_->IsEmpty())
    async_queue_->Dispatch();
}

bool V0CustomElementMicrotaskRunQueue::IsEmpty() const {
  return sync_queue_->IsEmpty() && async_queue_->IsEmpty();
}

}  // namespace blink
