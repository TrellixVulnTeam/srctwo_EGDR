// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/favicon/core/favicon_util.h"

#include "base/bind.h"
#include "base/callback.h"
#include "components/favicon/core/favicon_service.h"

namespace favicon {
namespace {

// Adapter to convert a FaviconImageCallback into a FaviconRawBitmapCallback
// for GetFaviconImageForPageURL.
void RunCallbackWithImage(
    const favicon_base::FaviconImageCallback& callback,
    const favicon_base::FaviconRawBitmapResult& bitmap_result) {
  favicon_base::FaviconImageResult result;
  if (bitmap_result.is_valid()) {
    result.image = gfx::Image::CreateFrom1xPNGBytes(
        bitmap_result.bitmap_data->front(), bitmap_result.bitmap_data->size());
    result.icon_url = bitmap_result.icon_url;
    callback.Run(result);
    return;
  }
  callback.Run(result);
}

}  // namespace

base::CancelableTaskTracker::TaskId GetFaviconImageForPageURL(
    FaviconService* favicon_service,
    const GURL& page_url,
    favicon_base::IconType type,
    const favicon_base::FaviconImageCallback& callback,
    base::CancelableTaskTracker* tracker) {
  if (!favicon_service)
    return base::CancelableTaskTracker::kBadTaskId;

  if (type == favicon_base::FAVICON) {
    return favicon_service->GetFaviconImageForPageURL(page_url, callback,
                                                      tracker);
  }

  return favicon_service->GetRawFaviconForPageURL(
      page_url, type, 0, base::Bind(&RunCallbackWithImage, callback), tracker);
}

}  // namespace favicon
