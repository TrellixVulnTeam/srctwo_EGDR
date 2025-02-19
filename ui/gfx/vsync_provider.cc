// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gfx/vsync_provider.h"

namespace gfx {

void FixedVSyncProvider::GetVSyncParameters(
    const UpdateVSyncCallback& callback) {
  callback.Run(timebase_, interval_);
}

}  // namespace gfx
