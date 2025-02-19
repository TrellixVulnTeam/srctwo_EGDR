// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_BASE_ANDROID_OVERLAY_CONFIG_H_
#define MEDIA_BASE_ANDROID_OVERLAY_CONFIG_H_

#include "base/callback.h"
#include "base/macros.h"
#include "media/base/media_export.h"
#include "ui/gfx/geometry/rect.h"

namespace media {

class AndroidOverlay;

// Configuration used to create an overlay.
struct MEDIA_EXPORT AndroidOverlayConfig {
 public:
  // Called when the overlay is ready for use, via |GetJavaSurface()|.
  using ReadyCB = base::OnceCallback<void(AndroidOverlay*)>;

  // Called when overlay has failed before |ReadyCB| is called.  Will not be
  // called after ReadyCB.  It will be the last callback for the overlay, except
  // for any DeletedCB.
  using FailedCB = base::OnceCallback<void(AndroidOverlay*)>;

  // Called when the surface has been destroyed.  This will not be called unless
  // ReadyCB has been called.  It will be the last callback for the overlay,
  // except for any DeletedCB.  In response, the client is expected to quit
  // using the surface and delete the overlay object.
  using DestroyedCB = base::OnceCallback<void(AndroidOverlay*)>;

  // Called when the overlay object has been deleted.  This is unrelated to
  // any DestroyedCB, which informs us when the surface is no longer usable and
  // that we should delete the AndroidOverlay object.  DeletedCB is called when
  // the AndroidOverlay object is deleted for any reason.  It is guaranteed that
  // the overlay pointer will still be a valid pointer, but it is not safe to
  // access it.  It's provided just to make it easier to tell which overlay is
  // being deleted.
  using DeletedCB = base::OnceCallback<void(AndroidOverlay*)>;

  // Configuration used to create an overlay.
  AndroidOverlayConfig();
  AndroidOverlayConfig(AndroidOverlayConfig&&);
  ~AndroidOverlayConfig();

  // Initial rectangle for the overlay.  May be changed via ScheduleLayout().
  gfx::Rect rect;

  // Require a secure overlay?
  bool secure = false;

  // Convenient helpers since the syntax is weird.
  void is_ready(AndroidOverlay* overlay) { std::move(ready_cb).Run(overlay); }
  void is_failed(AndroidOverlay* overlay) { std::move(failed_cb).Run(overlay); }

  ReadyCB ready_cb;
  FailedCB failed_cb;

  DISALLOW_COPY(AndroidOverlayConfig);
};

// Common factory type.
using AndroidOverlayFactoryCB =
    base::RepeatingCallback<std::unique_ptr<AndroidOverlay>(
        AndroidOverlayConfig)>;

}  // namespace media

#endif  // MEDIA_BASE_ANDROID_OVERLAY_CONFIG_H_
