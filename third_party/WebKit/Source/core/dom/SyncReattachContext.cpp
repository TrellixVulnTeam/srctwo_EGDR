// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/dom/SyncReattachContext.h"

namespace blink {

SyncReattachContext* SyncReattachContext::current_context_ = nullptr;

}  // namespace blink
