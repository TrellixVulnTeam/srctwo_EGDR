// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_RENDER_PROCESS_HOST_FACTORY_H_
#define CONTENT_PUBLIC_BROWSER_RENDER_PROCESS_HOST_FACTORY_H_

#include "content/common/content_export.h"

namespace content {
class BrowserContext;
class RenderProcessHost;

// Factory object for RenderProcessHosts. Using this factory allows tests to
// swap out a different one to use a TestRenderProcessHost.
class RenderProcessHostFactory {
 public:
  virtual ~RenderProcessHostFactory() {}
  virtual RenderProcessHost* CreateRenderProcessHost(
      BrowserContext* browser_context) const = 0;
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_RENDER_PROCESS_HOST_FACTORY_H_
