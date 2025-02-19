// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_RENDERER_HOST_RENDER_WIDGET_HOST_VIEW_BASE_OBSERVER_H_
#define CONTENT_BROWSER_RENDERER_HOST_RENDER_WIDGET_HOST_VIEW_BASE_OBSERVER_H_

#include "base/macros.h"
#include "content/common/content_export.h"

namespace content {

class RenderWidgetHostViewBase;

class CONTENT_EXPORT RenderWidgetHostViewBaseObserver {
 public:
  // All derived classes must de-register as observers when receiving this
  // notification.
  virtual void OnRenderWidgetHostViewBaseDestroyed(
      RenderWidgetHostViewBase* view);

 protected:
  RenderWidgetHostViewBaseObserver() = default;
  virtual ~RenderWidgetHostViewBaseObserver();

 private:
  DISALLOW_COPY_AND_ASSIGN(RenderWidgetHostViewBaseObserver);
};

}  // namespace content

#endif  // CONTENT_BROWSER_RENDERER_HOST_RENDER_WIDGET_HOST_VIEW_BASE_OBSERVER_H_
