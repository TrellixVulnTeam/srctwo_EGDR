// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/download/content/public/all_download_item_notifier.h"

namespace download {

AllDownloadItemNotifier::AllDownloadItemNotifier(
    content::DownloadManager* manager,
    AllDownloadItemNotifier::Observer* observer)
    : manager_(manager), observer_(observer) {
  DCHECK(observer_);
  manager_->AddObserver(this);
  content::DownloadManager::DownloadVector items;
  manager_->GetAllDownloads(&items);
  for (content::DownloadManager::DownloadVector::const_iterator it =
           items.begin();
       it != items.end(); ++it) {
    (*it)->AddObserver(this);
    observing_.insert(*it);
  }

  if (manager_->IsManagerInitialized())
    observer_->OnManagerInitialized(manager_);
}

AllDownloadItemNotifier::~AllDownloadItemNotifier() {
  if (manager_)
    manager_->RemoveObserver(this);
  for (std::set<content::DownloadItem*>::const_iterator it = observing_.begin();
       it != observing_.end(); ++it) {
    (*it)->RemoveObserver(this);
  }
  observing_.clear();
}

void AllDownloadItemNotifier::OnManagerInitialized() {
  observer_->OnManagerInitialized(manager_);
}

void AllDownloadItemNotifier::ManagerGoingDown(
    content::DownloadManager* manager) {
  DCHECK_EQ(manager_, manager);
  observer_->OnManagerGoingDown(manager);
  manager_->RemoveObserver(this);
  manager_ = NULL;
}

void AllDownloadItemNotifier::OnDownloadCreated(
    content::DownloadManager* manager,
    content::DownloadItem* item) {
  item->AddObserver(this);
  observing_.insert(item);
  observer_->OnDownloadCreated(manager, item);
}

void AllDownloadItemNotifier::OnDownloadUpdated(content::DownloadItem* item) {
  observer_->OnDownloadUpdated(manager_, item);
}

void AllDownloadItemNotifier::OnDownloadOpened(content::DownloadItem* item) {
  observer_->OnDownloadOpened(manager_, item);
}

void AllDownloadItemNotifier::OnDownloadRemoved(content::DownloadItem* item) {
  observer_->OnDownloadRemoved(manager_, item);
}

void AllDownloadItemNotifier::OnDownloadDestroyed(content::DownloadItem* item) {
  item->RemoveObserver(this);
  observing_.erase(item);
}

}  // namespace download
