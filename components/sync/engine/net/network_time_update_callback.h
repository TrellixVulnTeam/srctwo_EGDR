// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_SYNC_ENGINE_NET_NETWORK_TIME_UPDATE_CALLBACK_H_
#define COMPONENTS_SYNC_ENGINE_NET_NETWORK_TIME_UPDATE_CALLBACK_H_

#include "base/callback.h"
#include "base/time/time.h"

namespace syncer {

// TODO(pvalenzuela): Avoid duplication of this typedef by defining it in a
// common location. This is duplicated here because its original definition in
// NetworkTimeTracker cannot be depended on.
//
// Callback for updating the network time.
// Params:
// const base::Time& network_time - the new network time.
// const base::TimeDelta& resolution - how precise the reading is.
// const base::TimeDelta& latency - the http request's latency.
using NetworkTimeUpdateCallback = base::Callback<
    void(const base::Time&, const base::TimeDelta&, const base::TimeDelta&)>;

}  // namespace syncer

#endif  // COMPONENTS_SYNC_ENGINE_NET_NETWORK_TIME_UPDATE_CALLBACK_H_
