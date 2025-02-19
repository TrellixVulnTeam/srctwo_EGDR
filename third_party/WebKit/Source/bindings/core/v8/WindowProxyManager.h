// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WindowProxyManager_h
#define WindowProxyManager_h

#include <utility>

#include "bindings/core/v8/LocalWindowProxy.h"
#include "bindings/core/v8/RemoteWindowProxy.h"
#include "core/CoreExport.h"
#include "core/frame/LocalFrame.h"
#include "core/frame/RemoteFrame.h"
#include "platform/heap/Handle.h"
#include "v8/include/v8.h"

namespace blink {

class DOMWrapperWorld;
class SecurityOrigin;

class WindowProxyManager : public GarbageCollected<WindowProxyManager> {
 public:
  DECLARE_TRACE();

  v8::Isolate* GetIsolate() const { return isolate_; }

  void ClearForClose();
  void CORE_EXPORT ClearForNavigation();
  void ClearForSwap();

  // Global proxies are passed in a vector to maintain their order: global proxy
  // object for the main world is always first. This is needed to prevent bugs
  // like https://crbug.com/700077 .
  using GlobalProxyVector =
      Vector<std::pair<DOMWrapperWorld*, v8::Local<v8::Object>>>;
  void CORE_EXPORT ReleaseGlobalProxies(GlobalProxyVector&);
  void CORE_EXPORT SetGlobalProxies(const GlobalProxyVector&);

  WindowProxy* MainWorldProxy() {
    window_proxy_->InitializeIfNeeded();
    return window_proxy_;
  }

  WindowProxy* GetWindowProxy(DOMWrapperWorld& world) {
    WindowProxy* window_proxy = WindowProxyMaybeUninitialized(world);
    window_proxy->InitializeIfNeeded();
    return window_proxy;
  }

 protected:
  using IsolatedWorldMap = HeapHashMap<int, Member<WindowProxy>>;
  enum class FrameType { kLocal, kRemote };

  WindowProxyManager(Frame&, FrameType);

 private:
  // Creates an uninitialized WindowProxy.
  WindowProxy* CreateWindowProxy(DOMWrapperWorld&);
  WindowProxy* WindowProxyMaybeUninitialized(DOMWrapperWorld&);

  v8::Isolate* const isolate_;
  const Member<Frame> frame_;
  const FrameType frame_type_;

 protected:
  const Member<WindowProxy> window_proxy_;
  IsolatedWorldMap isolated_worlds_;
};

template <typename FrameType, typename ProxyType>
class WindowProxyManagerImplHelper : public WindowProxyManager {
 private:
  using Base = WindowProxyManager;

 public:
  ProxyType* MainWorldProxy() {
    return static_cast<ProxyType*>(Base::MainWorldProxy());
  }
  ProxyType* WindowProxy(DOMWrapperWorld& world) {
    return static_cast<ProxyType*>(Base::GetWindowProxy(world));
  }

 protected:
  WindowProxyManagerImplHelper(Frame& frame, FrameType frame_type)
      : WindowProxyManager(frame, frame_type) {}
};

class LocalWindowProxyManager
    : public WindowProxyManagerImplHelper<LocalFrame, LocalWindowProxy> {
 public:
  static LocalWindowProxyManager* Create(LocalFrame& frame) {
    return new LocalWindowProxyManager(frame);
  }

  // TODO(yukishiino): Remove this method.
  LocalWindowProxy* MainWorldProxyMaybeUninitialized() {
    return static_cast<LocalWindowProxy*>(window_proxy_.Get());
  }

  // Sets the given security origin to the main world's context.  Also updates
  // the security origin of the context for each isolated world.
  void UpdateSecurityOrigin(SecurityOrigin*);

 private:
  explicit LocalWindowProxyManager(LocalFrame& frame)
      : WindowProxyManagerImplHelper<LocalFrame, LocalWindowProxy>(
            frame,
            FrameType::kLocal) {}
};

class RemoteWindowProxyManager
    : public WindowProxyManagerImplHelper<RemoteFrame, RemoteWindowProxy> {
 public:
  static RemoteWindowProxyManager* Create(RemoteFrame& frame) {
    return new RemoteWindowProxyManager(frame);
  }

 private:
  explicit RemoteWindowProxyManager(RemoteFrame& frame)
      : WindowProxyManagerImplHelper<RemoteFrame, RemoteWindowProxy>(
            frame,
            FrameType::kRemote) {}
};

}  // namespace blink

#endif  // WindowProxyManager_h
