// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/reading_list/reading_list_menu_notifier.h"

#include <memory>

#include "components/reading_list/core/reading_list_model.h"
#import "ios/chrome/browser/ui/reading_list/reading_list_menu_notification_delegate.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

class ReadingListObserverBridge;

@interface ReadingListMenuNotifier () {
  // Observer for reading list changes.
  std::unique_ptr<ReadingListObserverBridge> _readingListObserverBridge;

  // Backing object for property of the same name.
  __weak id<ReadingListMenuNotificationDelegate> _delegate;

  // Keep a reference to detach before deallocing.
  ReadingListModel* _readingListModel;  // weak
}

// Detach the observer on the reading list.
- (void)detachReadingListModel;

// Handle callbacks from the reading list model observer.
- (void)readingListModelCompletedBatchUpdates:(const ReadingListModel*)model;

@end

// TODO(crbug.com/590725): use the one-and-only protocol-based implementation of
// ReadingListModelObserver
class ReadingListObserverBridge : public ReadingListModelObserver {
 public:
  explicit ReadingListObserverBridge(ReadingListMenuNotifier* owner)
      : owner_(owner) {}

  ~ReadingListObserverBridge() override {}

  void ReadingListModelLoaded(const ReadingListModel* model) override {}

  void ReadingListModelBeganBatchUpdates(
      const ReadingListModel* model) override {}

  void ReadingListModelCompletedBatchUpdates(
      const ReadingListModel* model) override {
    [owner_ readingListModelCompletedBatchUpdates:model];
  }

  void ReadingListModelBeingDeleted(const ReadingListModel* model) override {}

  void ReadingListDidApplyChanges(ReadingListModel* model) override {
    [owner_ readingListModelCompletedBatchUpdates:model];
  }

 private:
  __weak ReadingListMenuNotifier* owner_;  // weak, owns us
};

@implementation ReadingListMenuNotifier
@synthesize delegate = _delegate;

- (instancetype)initWithReadingList:(ReadingListModel*)readingListModel {
  if (self = [super init]) {
    _readingListObserverBridge.reset(new ReadingListObserverBridge(self));
    _readingListModel = readingListModel;
    _readingListModel->AddObserver(_readingListObserverBridge.get());
  }
  return self;
}

- (void)dealloc {
  [self detachReadingListModel];
}

- (void)detachReadingListModel {
  _readingListModel->RemoveObserver(_readingListObserverBridge.get());
  _readingListObserverBridge.reset();
}

- (void)readingListModelCompletedBatchUpdates:(const ReadingListModel*)model {
  [_delegate unreadCountChanged:model->unread_size()];
  [_delegate unseenStateChanged:[self readingListUnseenItemsExist]];
}

- (NSInteger)readingListUnreadCount {
  return _readingListModel->unread_size();
}

- (BOOL)readingListUnseenItemsExist {
  return _readingListModel->unseen_size() > 0;
}

@end
