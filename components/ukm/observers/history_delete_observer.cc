// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/ukm/observers/history_delete_observer.h"

#include "components/history/core/browser/history_service.h"

namespace ukm {

HistoryDeleteObserver::HistoryDeleteObserver() : history_observer_(this) {}

HistoryDeleteObserver::~HistoryDeleteObserver() {}

void HistoryDeleteObserver::ObserveServiceForDeletions(
    history::HistoryService* history_service) {
  if (history_service) {
    history_observer_.Add(history_service);
  }
}

void HistoryDeleteObserver::OnURLsDeleted(
    history::HistoryService* history_service,
    bool all_history,
    bool expired,
    const history::URLRows& deleted_rows,
    const std::set<GURL>& favicon_urls) {
  if (!expired)
    OnHistoryDeleted();
}

void HistoryDeleteObserver::HistoryServiceBeingDeleted(
    history::HistoryService* history_service) {
  history_observer_.Remove(history_service);
}

}  // namespace ukm
