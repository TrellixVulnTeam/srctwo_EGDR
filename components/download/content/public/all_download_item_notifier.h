// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_DOWNLOAD_CONTENT_PUBLIC_ALL_DOWNLOAD_ITEM_NOTIFIER_H_
#define COMPONENTS_DOWNLOAD_CONTENT_PUBLIC_ALL_DOWNLOAD_ITEM_NOTIFIER_H_

#include <set>

#include "base/macros.h"
#include "content/public/browser/download_item.h"
#include "content/public/browser/download_manager.h"

// AllDownloadItemNotifier observes ALL the DownloadItems on a given
// DownloadManager.
// Clients should use GetManager() instead of storing their own pointer to the
// manager so that they can be sensitive to managers that have gone down.

// Example Usage:
// class DownloadSystemConsumer : public AllDownloadItemNotifier::Observer {
//  public:
//   DownloadSystemConsumer(DownloadManager* original_manager,
//            DownloadManager* incognito_manager)
//     : original_notifier_(original_manager, this),
//       incognito_notifier_(incognito_manager, this) {
//   }
//
//   virtual void OnDownloadUpdated(
//     DownloadManager* manager, DownloadItem* item) { ... }
//
//  private:
//   AllDownloadItemNotifier original_notifier_;
//   AllDownloadItemNotifier incognito_notifier_;
// };

namespace download {

class AllDownloadItemNotifier : public content::DownloadManager::Observer,
                                public content::DownloadItem::Observer {
 public:
  // All of the methods take the DownloadManager so that subclasses can observe
  // multiple managers at once and easily distinguish which manager a given item
  // belongs to.
  class Observer {
   public:
    Observer() {}
    virtual ~Observer() {}

    virtual void OnManagerInitialized(content::DownloadManager* manager) {}
    virtual void OnManagerGoingDown(content::DownloadManager* manager) {}
    virtual void OnDownloadCreated(content::DownloadManager* manager,
                                   content::DownloadItem* item) {}
    virtual void OnDownloadUpdated(content::DownloadManager* manager,
                                   content::DownloadItem* item) {}
    virtual void OnDownloadOpened(content::DownloadManager* manager,
                                  content::DownloadItem* item) {}
    virtual void OnDownloadRemoved(content::DownloadManager* manager,
                                   content::DownloadItem* item) {}

   private:
    DISALLOW_COPY_AND_ASSIGN(Observer);
  };

  AllDownloadItemNotifier(content::DownloadManager* manager,
                          Observer* observer);

  ~AllDownloadItemNotifier() override;

  // Returns NULL if the manager has gone down.
  content::DownloadManager* GetManager() const { return manager_; }

 private:
  // DownloadManager::Observer
  void OnManagerInitialized() override;
  void ManagerGoingDown(content::DownloadManager* manager) override;
  void OnDownloadCreated(content::DownloadManager* manager,
                         content::DownloadItem* item) override;

  // DownloadItem::Observer
  void OnDownloadUpdated(content::DownloadItem* item) override;
  void OnDownloadOpened(content::DownloadItem* item) override;
  void OnDownloadRemoved(content::DownloadItem* item) override;
  void OnDownloadDestroyed(content::DownloadItem* item) override;

  content::DownloadManager* manager_;
  AllDownloadItemNotifier::Observer* observer_;
  std::set<content::DownloadItem*> observing_;

  DISALLOW_COPY_AND_ASSIGN(AllDownloadItemNotifier);
};

}  // namespace download

#endif  // COMPONENTS_DOWNLOAD_CONTENT_PUBLIC_ALL_DOWNLOAD_ITEM_NOTIFIER_H_
