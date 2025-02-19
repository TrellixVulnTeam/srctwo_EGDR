// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_WIN_SINGLETON_HWND_H_
#define UI_GFX_WIN_SINGLETON_HWND_H_

#include <windows.h>
#include <vector>

#include "base/macros.h"
#include "base/observer_list.h"
#include "ui/gfx/gfx_export.h"
#include "ui/gfx/win/window_impl.h"

namespace base {
template<typename T> struct DefaultSingletonTraits;
}

namespace gfx {

class SingletonHwndObserver;

// Singleton message-only HWND that allows interested clients to receive WM_*
// notifications.
class GFX_EXPORT SingletonHwnd : public WindowImpl {
 public:
  static SingletonHwnd* GetInstance();

  // Windows callback for WM_* notifications.
  BOOL ProcessWindowMessage(HWND window,
                            UINT message,
                            WPARAM wparam,
                            LPARAM lparam,
                            LRESULT& result,
                            DWORD msg_map_id) override;

 private:
  friend class SingletonHwndObserver;
  friend struct base::DefaultSingletonTraits<SingletonHwnd>;

  SingletonHwnd();
  ~SingletonHwnd() override;

  // Add/remove SingletonHwndObserver to forward WM_* notifications.
  void AddObserver(SingletonHwndObserver* observer);
  void RemoveObserver(SingletonHwndObserver* observer);

  // List of registered observers.
  base::ObserverList<SingletonHwndObserver, true> observer_list_;

  DISALLOW_COPY_AND_ASSIGN(SingletonHwnd);
};

}  // namespace gfx

#endif  // UI_GFX_WIN_SINGLETON_HWND_H_
