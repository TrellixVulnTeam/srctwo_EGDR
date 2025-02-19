// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_ANDROID_COMPOSITOR_H_
#define CONTENT_PUBLIC_BROWSER_ANDROID_COMPOSITOR_H_

#include "base/callback.h"
#include "cc/resources/ui_resource_bitmap.h"
#include "content/common/content_export.h"
#include "gpu/ipc/common/surface_handle.h"
#include "ui/android/resources/ui_resource_provider.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/native_widget_types.h"

namespace cc {
class Layer;
}

namespace gpu {
namespace gles2 {
struct ContextCreationAttribHelper;
}  // namespace gles2

struct SharedMemoryLimits;
}

namespace ui {
class ResourceManager;
class UIResourceProvider;
}

namespace viz {
class ContextProvider;
}

namespace content {
class CompositorClient;

// An interface to the browser-side compositor.
class CONTENT_EXPORT Compositor {
 public:
  virtual ~Compositor() {}

  // Performs the global initialization needed before any compositor
  // instance can be used. This should be called only once.
  static void Initialize();

  // Creates a GL context for the provided |handle|. If a null handle is passed,
  // an offscreen context is created. This must be called on the UI thread.
  using ContextProviderCallback =
      base::Callback<void(scoped_refptr<viz::ContextProvider>)>;
  static void CreateContextProvider(
      gpu::SurfaceHandle handle,
      gpu::gles2::ContextCreationAttribHelper attributes,
      gpu::SharedMemoryLimits shared_memory_limits,
      ContextProviderCallback callback);

  // Creates and returns a compositor instance.  |root_window| needs to outlive
  // the compositor as it manages callbacks on the compositor.
  static Compositor* Create(CompositorClient* client,
                            gfx::NativeWindow root_window);

  // Attaches the layer tree.
  virtual void SetRootLayer(scoped_refptr<cc::Layer> root) = 0;

  // Set the output surface bounds.
  virtual void SetWindowBounds(const gfx::Size& size) = 0;

  // Set the output surface which the compositor renders into.
  virtual void SetSurface(jobject surface) = 0;

  // Tells the view tree to assume a transparent background when rendering.
  virtual void SetHasTransparentBackground(bool flag) = 0;

  // Tells the compositor to allocate an alpha channel.  This won't take effect
  // until the compositor selects a new egl config, usually when the underlying
  // Android Surface changes format.
  virtual void SetRequiresAlphaChannel(bool flag) = 0;

  // Request layout and draw. You only need to call this if you need to trigger
  // Composite *without* having modified the layer tree.
  virtual void SetNeedsComposite() = 0;

  // Returns the UI resource provider associated with the compositor.
  virtual ui::UIResourceProvider& GetUIResourceProvider() = 0;

  // Returns the resource manager associated with the compositor.
  virtual ui::ResourceManager& GetResourceManager() = 0;

 protected:
  Compositor() {}
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_ANDROID_COMPOSITOR_H_
