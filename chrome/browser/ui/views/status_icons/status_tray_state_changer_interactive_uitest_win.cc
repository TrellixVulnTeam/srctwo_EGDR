// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <utility>

#include "base/macros.h"
#include "base/win/scoped_com_initializer.h"
#include "chrome/browser/status_icons/status_icon.h"
#include "chrome/browser/ui/views/status_icons/status_icon_win.h"
#include "chrome/browser/ui/views/status_icons/status_tray_state_changer_win.h"
#include "chrome/browser/ui/views/status_icons/status_tray_win.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/gfx/image/image_skia.h"

class StatusTrayStateChangerWinTest : public testing::Test {
 public:
  StatusTrayStateChangerWinTest() {}

  void SetUp() override {
    testing::Test::SetUp();
    com_.reset(new base::win::ScopedCOMInitializer());
    status_tray_.reset(new StatusTrayWin());
    SkBitmap bitmap;

    // Put a real bitmap into "bitmap".  2x2 bitmap of green 32 bit pixels.
    bitmap.allocN32Pixels(16, 16);
    bitmap.eraseColor(SK_ColorGREEN);
    status_icon_win_ = (StatusIconWin*)status_tray_->CreateStatusIcon(
        StatusTray::OTHER_ICON,
        gfx::ImageSkia::CreateFrom1xBitmap(bitmap),
        base::string16());
    tray_watcher_ = new StatusTrayStateChangerWin(status_icon_win_->icon_id(),
                                                  status_icon_win_->window());
  }

  void TearDown() override {
    tray_watcher_ = NULL;
    status_tray_.reset();
    com_.reset();
    testing::Test::TearDown();
  }

 protected:
  HWND icon_window() { return status_icon_win_->window(); }
  UINT icon_id() { return status_icon_win_->icon_id(); }

  std::unique_ptr<NOTIFYITEM> SetupAndGetCurrentNotifyItem() {
    EXPECT_TRUE(CallCreateTrayNotify());

    EXPECT_TRUE(IsInterfaceKnown());

    std::unique_ptr<NOTIFYITEM> notify_item = GetNotifyItem();
    EXPECT_TRUE(notify_item.get() != NULL);
    DCHECK_EQ(notify_item->hwnd, icon_window());
    DCHECK_EQ(notify_item->id, icon_id());

    return notify_item;
  }

  bool CallCreateTrayNotify() { return tray_watcher_->CreateTrayNotify(); }

  bool IsInterfaceKnown() {
    return StatusTrayStateChangerWin::INTERFACE_VERSION_UNKNOWN !=
           tray_watcher_->interface_version_;
  }

  void SendNotifyItemUpdate(std::unique_ptr<NOTIFYITEM> notify_item) {
    tray_watcher_->SendNotifyItemUpdate(std::move(notify_item));
  }

  std::unique_ptr<NOTIFYITEM> GetNotifyItem() {
    return tray_watcher_->RegisterCallback();
  }

  std::unique_ptr<base::win::ScopedCOMInitializer> com_;
  std::unique_ptr<StatusTrayWin> status_tray_;
  scoped_refptr<StatusTrayStateChangerWin> tray_watcher_;

  StatusIconWin* status_icon_win_;

  DISALLOW_COPY_AND_ASSIGN(StatusTrayStateChangerWinTest);
};

// Test is disabled due to multiple COM initialization errors.  See
// http//crbug.com/367199 for details.
TEST_F(StatusTrayStateChangerWinTest, DISABLED_Setup) {
  // This tests the code path that will read the NOTIFYITEM data structure for
  // use in future tests.
  std::unique_ptr<NOTIFYITEM> notify_item = SetupAndGetCurrentNotifyItem();
  EXPECT_FALSE(notify_item.get() == NULL);
}

// Test is disabled due to multiple COM initialization errors.  See
// http//crbug.com/367199 for details.
TEST_F(StatusTrayStateChangerWinTest, DISABLED_ComApiTest) {

  // Setup code to read the current preference.
  std::unique_ptr<NOTIFYITEM> notify_item = SetupAndGetCurrentNotifyItem();
  ASSERT_TRUE(notify_item.get() != NULL);

  // Store the current pref.
  DWORD current_preference = notify_item->preference;

  // Ensure that running our code will do something.
  if (notify_item->preference != PREFERENCE_SHOW_WHEN_ACTIVE) {
    std::unique_ptr<NOTIFYITEM> notify_item_copy(new NOTIFYITEM(*notify_item));
    notify_item_copy->preference = PREFERENCE_SHOW_WHEN_ACTIVE;
    SendNotifyItemUpdate(std::move(notify_item_copy));
  }

  // Run the interesting code.
  tray_watcher_->EnsureTrayIconVisible();

  EXPECT_EQ(PREFERENCE_SHOW_ALWAYS, GetNotifyItem()->preference);
  SendNotifyItemUpdate(std::move(notify_item));
  notify_item = GetNotifyItem();

  EXPECT_EQ(notify_item->preference, current_preference);
}

// Test is disabled due to multiple COM initialization errors.  See
// http//crbug.com/367199 for details.
TEST_F(StatusTrayStateChangerWinTest, DISABLED_TraySizeApiTest) {
  // Used to reset operating system state afterwards.
  std::unique_ptr<NOTIFYITEM> notify_item = SetupAndGetCurrentNotifyItem();
  // We can't actually run this test if we're already showing the icon.
  if (notify_item->preference == PREFERENCE_SHOW_ALWAYS)
    return;

  // This test can only run if the tray window structure conforms to what I've
  // seen in Win7 and Win8.
  HWND shell_tray_hwnd = ::FindWindow(L"Shell_TrayWnd", NULL);
  if (shell_tray_hwnd == NULL)
    return;

  HWND tray_notify_hwnd =
      ::FindWindowEx(shell_tray_hwnd, NULL, L"TrayNotifyWnd", NULL);
  ASSERT_TRUE(tray_notify_hwnd != NULL);

  RECT original_tray_notify_rect;
  ::GetWindowRect(tray_notify_hwnd, &original_tray_notify_rect);

  LONG width = original_tray_notify_rect.right - original_tray_notify_rect.left;
  ASSERT_GT(width, 0);

  tray_watcher_->EnsureTrayIconVisible();

  RECT new_tray_notify_rect;
  ::GetWindowRect(tray_notify_hwnd, &new_tray_notify_rect);

  LONG new_width = new_tray_notify_rect.right - new_tray_notify_rect.left;

  EXPECT_GT(new_width, width);

  SendNotifyItemUpdate(std::move(notify_item));
  ::GetWindowRect(tray_notify_hwnd, &new_tray_notify_rect);
  new_width = new_tray_notify_rect.right - new_tray_notify_rect.left;
  EXPECT_EQ(width, new_width);
}
