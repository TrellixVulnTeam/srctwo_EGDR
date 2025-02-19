// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_VIZ_COMMON_QUADS_SINGLE_RELEASE_CALLBACK_H_
#define COMPONENTS_VIZ_COMMON_QUADS_SINGLE_RELEASE_CALLBACK_H_

#include <memory>

#include "base/memory/ptr_util.h"
#include "components/viz/common/quads/release_callback.h"
#include "components/viz/common/viz_common_export.h"

namespace viz {

class VIZ_COMMON_EXPORT SingleReleaseCallback {
 public:
  static std::unique_ptr<SingleReleaseCallback> Create(
      const ReleaseCallback& cb) {
    return base::WrapUnique(new SingleReleaseCallback(cb));
  }

  ~SingleReleaseCallback();

  void Run(const gpu::SyncToken& sync_token, bool is_lost);

 private:
  explicit SingleReleaseCallback(const ReleaseCallback& callback);

  ReleaseCallback callback_;
};

}  // namespace viz

#endif  // COMPONENTS_VIZ_COMMON_QUADS_SINGLE_RELEASE_CALLBACK_H_
