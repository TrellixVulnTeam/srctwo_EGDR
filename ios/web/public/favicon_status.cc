// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/web/public/favicon_status.h"

namespace web {

FaviconStatus::FaviconStatus() : valid(false) {
  // TODO(rohitrao): Add a GetDefaultFavicon() method to WebClient and call it
  // here.
}

}  // namespace web
