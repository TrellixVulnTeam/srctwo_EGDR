// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/favicon/core/favicon_service.h"

namespace favicon {

// static
void FaviconService::FaviconResultsCallbackRunner(
    const favicon_base::FaviconResultsCallback& callback,
    const std::vector<favicon_base::FaviconRawBitmapResult>* results) {
  callback.Run(*results);
}

}  // namespace favicon
