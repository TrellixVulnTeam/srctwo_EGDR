// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_OZONE_PLATFORM_WAYLAND_CLIENT_NATIVE_PIXMAP_FACTORY_WAYLAND_H_
#define UI_OZONE_PLATFORM_WAYLAND_CLIENT_NATIVE_PIXMAP_FACTORY_WAYLAND_H_

namespace gfx {
class ClientNativePixmapFactory;
}

namespace ui {

// Constructor hook for use in constructor_list.cc
gfx::ClientNativePixmapFactory* CreateClientNativePixmapFactoryWayland();

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_CLIENT_NATIVE_PIXMAP_FACTORY_WAYLAND_H_
