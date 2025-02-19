// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_TEST_SCOPED_FAKE_FULL_KEYBOARD_ACCESS_H_
#define UI_BASE_TEST_SCOPED_FAKE_FULL_KEYBOARD_ACCESS_H_

#include <memory>

#include "base/macros.h"

namespace base {
namespace mac {
class ScopedObjCClassSwizzler;
}
}

namespace ui {
namespace test {

// A singleton class which swizzles [NSApp isFullKeyboardAccessEnabled] so
// that it returns the value set using set_full_keyboard_access_state().
class ScopedFakeFullKeyboardAccess {
 public:
  ScopedFakeFullKeyboardAccess();
  ~ScopedFakeFullKeyboardAccess();

  // Returns the ScopedFakeFullKeyboardAccess singleton or null if one hasn't
  // been created yet.
  static ScopedFakeFullKeyboardAccess* GetInstance();

  bool full_keyboard_access_state() const {
    return full_keyboard_access_state_;
  }

  // Set the value to be returned by [NSApp isFullKeyboardAccessEnabled].
  void set_full_keyboard_access_state(bool new_state) {
    full_keyboard_access_state_ = new_state;
  }

 private:
  bool full_keyboard_access_state_;
  std::unique_ptr<base::mac::ScopedObjCClassSwizzler> swizzler_;

  DISALLOW_COPY_AND_ASSIGN(ScopedFakeFullKeyboardAccess);
};

}  // namespace test
}  // namespace ui

#endif  // UI_BASE_TEST_SCOPED_FAKE_FULL_KEYBOARD_ACCESS_H_
