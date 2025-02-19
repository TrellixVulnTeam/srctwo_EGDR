// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_EXCLUSIVE_ACCESS_FULLSCREEN_CONTROLLER_TEST_H_
#define CHROME_BROWSER_UI_EXCLUSIVE_ACCESS_FULLSCREEN_CONTROLLER_TEST_H_

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/chrome_notification_types.h"
#include "chrome/browser/ui/exclusive_access/exclusive_access_bubble_hide_callback.h"
#include "chrome/browser/ui/exclusive_access/exclusive_access_bubble_type.h"
#include "chrome/browser/ui/exclusive_access/fullscreen_controller.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "content/public/browser/notification_service.h"
#include "content/public/test/test_utils.h"

// Observer for NOTIFICATION_FULLSCREEN_CHANGED notifications.
class FullscreenNotificationObserver
    : public content::WindowedNotificationObserver {
 public:
  FullscreenNotificationObserver() : WindowedNotificationObserver(
      chrome::NOTIFICATION_FULLSCREEN_CHANGED,
      content::NotificationService::AllSources()) {}
 protected:
  DISALLOW_COPY_AND_ASSIGN(FullscreenNotificationObserver);
};

// Observer for NOTIFICATION_MOUSE_LOCK_CHANGED notifications.
class MouseLockNotificationObserver
    : public content::WindowedNotificationObserver {
 public:
  MouseLockNotificationObserver() : WindowedNotificationObserver(
      chrome::NOTIFICATION_MOUSE_LOCK_CHANGED,
      content::NotificationService::AllSources()) {}
 protected:
  DISALLOW_COPY_AND_ASSIGN(MouseLockNotificationObserver);
};

// Test fixture with convenience functions for fullscreen and mouse lock.
class FullscreenControllerTest : public InProcessBrowserTest {
 protected:
  FullscreenControllerTest();
  ~FullscreenControllerTest() override;

  void SetUpOnMainThread() override;
  void TearDownOnMainThread() override;

  void RequestToLockMouse(bool user_gesture,
                          bool last_unlocked_by_target);
  void SetWebContentsGrantedSilentMouseLockPermission();
  void LostMouseLock();
  bool SendEscapeToFullscreenController();
  bool IsFullscreenForBrowser();
  bool IsWindowFullscreenForTabOrPending();
  ExclusiveAccessBubbleType GetExclusiveAccessBubbleType();
  bool IsFullscreenBubbleDisplayed();
  void GoBack();
  void Reload();
  void SetPrivilegedFullscreen(bool is_privileged);
  void EnterActiveTabFullscreen();

  static const char kFullscreenMouseLockHTML[];
  FullscreenController* GetFullscreenController();
  ExclusiveAccessManager* GetExclusiveAccessManager();

  void OnBubbleHidden(ExclusiveAccessBubbleHideReason);

  int InitialBubbleDelayMs() const;

  std::vector<ExclusiveAccessBubbleHideReason>
      mouse_lock_bubble_hide_reason_recorder_;

 private:
  void ToggleTabFullscreen_Internal(bool enter_fullscreen,
                                    bool retry_until_success);

  base::WeakPtrFactory<FullscreenControllerTest> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(FullscreenControllerTest);
};

#endif  // CHROME_BROWSER_UI_EXCLUSIVE_ACCESS_FULLSCREEN_CONTROLLER_TEST_H_
