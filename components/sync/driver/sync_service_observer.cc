// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/sync/driver/sync_service_observer.h"

namespace syncer {

void SyncServiceObserver::OnSyncCycleCompleted(SyncService* sync) {
  OnStateChanged(sync);
}

}  // namespace syncer
