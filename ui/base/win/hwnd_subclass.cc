// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/base/win/hwnd_subclass.h"

#include <algorithm>

#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/memory/singleton.h"
#include "base/stl_util.h"
#include "ui/gfx/win/hwnd_util.h"

namespace {
const char kHWNDSubclassKey[] = "__UI_BASE_WIN_HWND_SUBCLASS_PROC__";

LRESULT CALLBACK WndProc(HWND hwnd,
                         UINT message,
                         WPARAM w_param,
                         LPARAM l_param) {
  ui::HWNDSubclass* wrapped_wnd_proc =
      reinterpret_cast<ui::HWNDSubclass*>(
          ui::ViewProp::GetValue(hwnd, kHWNDSubclassKey));
  return wrapped_wnd_proc ? wrapped_wnd_proc->OnWndProc(hwnd,
                                                        message,
                                                        w_param,
                                                        l_param)
                          : DefWindowProc(hwnd, message, w_param, l_param);
}

WNDPROC GetCurrentWndProc(HWND target) {
  return reinterpret_cast<WNDPROC>(GetWindowLongPtr(target, GWLP_WNDPROC));
}

// Not defined before Win7
BOOL GetTouchInputInfoWrapper(HTOUCHINPUT handle, UINT count,
                              PTOUCHINPUT pointer, int size) {
  typedef BOOL(WINAPI *GetTouchInputInfoPtr)(HTOUCHINPUT, UINT,
                                             PTOUCHINPUT, int);
  GetTouchInputInfoPtr get_touch_input_info_func =
      reinterpret_cast<GetTouchInputInfoPtr>(
          GetProcAddress(GetModuleHandleA("user32.dll"), "GetTouchInputInfo"));
  if (get_touch_input_info_func)
    return get_touch_input_info_func(handle, count, pointer, size);
  return FALSE;
}

}  // namespace

namespace ui {

// Singleton factory that creates and manages the lifetime of all
// ui::HWNDSubclass objects.
class HWNDSubclass::HWNDSubclassFactory {
 public:
  static HWNDSubclassFactory* GetInstance() {
    return base::Singleton<
        HWNDSubclassFactory,
        base::LeakySingletonTraits<HWNDSubclassFactory>>::get();
  }

  // Returns a non-null HWNDSubclass corresponding to the HWND |target|. Creates
  // one if none exists. Retains ownership of the returned pointer.
  HWNDSubclass* GetHwndSubclassForTarget(HWND target) {
    DCHECK(target);
    HWNDSubclass* subclass = reinterpret_cast<HWNDSubclass*>(
        ui::ViewProp::GetValue(target, kHWNDSubclassKey));
    if (!subclass) {
      subclass = new ui::HWNDSubclass(target);
      hwnd_subclasses_.push_back(base::WrapUnique(subclass));
    }
    return subclass;
  }

  const std::vector<std::unique_ptr<HWNDSubclass>>& hwnd_subclasses() {
    return hwnd_subclasses_;
  }

 private:
  friend struct base::DefaultSingletonTraits<HWNDSubclassFactory>;

  HWNDSubclassFactory() {}

  std::vector<std::unique_ptr<HWNDSubclass>> hwnd_subclasses_;

  DISALLOW_COPY_AND_ASSIGN(HWNDSubclassFactory);
};

// static
void HWNDSubclass::AddFilterToTarget(HWND target, HWNDMessageFilter* filter) {
  HWNDSubclassFactory::GetInstance()->GetHwndSubclassForTarget(
      target)->AddFilter(filter);
}

// static
void HWNDSubclass::RemoveFilterFromAllTargets(HWNDMessageFilter* filter) {
  HWNDSubclassFactory* factory = HWNDSubclassFactory::GetInstance();
  for (const auto& subclass : factory->hwnd_subclasses())
    subclass->RemoveFilter(filter);
}

// static
HWNDSubclass* HWNDSubclass::GetHwndSubclassForTarget(HWND target) {
  return HWNDSubclassFactory::GetInstance()->GetHwndSubclassForTarget(target);
}

void HWNDSubclass::AddFilter(HWNDMessageFilter* filter) {
  DCHECK(filter);
  if (!base::ContainsValue(filters_, filter))
    filters_.push_back(filter);
}

void HWNDSubclass::RemoveFilter(HWNDMessageFilter* filter) {
  std::vector<HWNDMessageFilter*>::iterator it =
      std::find(filters_.begin(), filters_.end(), filter);
  if (it != filters_.end())
    filters_.erase(it);
}

HWNDSubclass::HWNDSubclass(HWND target)
    : target_(target),
      original_wnd_proc_(GetCurrentWndProc(target)),
      prop_(target, kHWNDSubclassKey, this) {
  gfx::SetWindowProc(target_, &WndProc);
}

HWNDSubclass::~HWNDSubclass() {
}

LRESULT HWNDSubclass::OnWndProc(HWND hwnd,
                                UINT message,
                                WPARAM w_param,
                                LPARAM l_param) {

  // Touch messages are always passed in screen coordinates. If the OS is
  // scaled, but the app is not DPI aware, then then WM_TOUCH might be
  // intended for a different window.
  if (message == WM_TOUCH) {
    TOUCHINPUT point;

    if (GetTouchInputInfoWrapper(reinterpret_cast<HTOUCHINPUT>(l_param), 1,
                                 &point, sizeof(TOUCHINPUT))) {
      POINT touch_location = {TOUCH_COORD_TO_PIXEL(point.x),
                              TOUCH_COORD_TO_PIXEL(point.y)};
      HWND actual_target = WindowFromPoint(touch_location);
      if (actual_target != hwnd) {
        return SendMessage(actual_target, message, w_param, l_param);
      }
    }
  }

  for (std::vector<HWNDMessageFilter*>::iterator it = filters_.begin();
      it != filters_.end(); ++it) {
    LRESULT l_result = 0;
    if ((*it)->FilterMessage(hwnd, message, w_param, l_param, &l_result))
      return l_result;
  }

  // In most cases, |original_wnd_proc_| will take care of calling
  // DefWindowProc.
  return CallWindowProc(original_wnd_proc_, hwnd, message, w_param, l_param);
}

HWNDMessageFilter::~HWNDMessageFilter() {
  HWNDSubclass::RemoveFilterFromAllTargets(this);
}

}  // namespace ui
