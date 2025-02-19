// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_ARC_NOTIFICATION_ARC_CUSTOM_DELEGATE_H_
#define UI_ARC_NOTIFICATION_ARC_CUSTOM_DELEGATE_H_

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "ui/message_center/notification_delegate.h"

namespace message_center {

class MessageCenterController;
class MessageView;
class Notification;

}  // namespace message_center

namespace arc {

class ArcNotificationItem;

// Implementation of NotificationDelegate for ARC notifications.
class ArcNotificationDelegate : public message_center::NotificationDelegate {
 public:
  explicit ArcNotificationDelegate(base::WeakPtr<ArcNotificationItem> item);

  // message_center::NotificationDelegate overrides:
  std::unique_ptr<message_center::MessageView> CreateCustomMessageView(
      message_center::MessageCenterController* controller,
      const message_center::Notification& notification) override;
  void Close(bool by_user) override;
  void Click() override;
  bool SettingsClick() override;
  bool ShouldDisplaySettingsButton() override;

 private:
  // The destructor is private since this class is ref-counted.
  ~ArcNotificationDelegate() override;

  // We use weak ptr to detect potential use-after-free. The lives of objects
  // around ARC notification is somewhat complex so we want to use it until
  // it gets stable.
  base::WeakPtr<ArcNotificationItem> item_;

  DISALLOW_COPY_AND_ASSIGN(ArcNotificationDelegate);
};

}  // namespace arc

#endif  // UI_ARC_NOTIFICATION_ARC_CUSTOM_DELEGATE_H_
