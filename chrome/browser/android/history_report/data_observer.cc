// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/android/history_report/data_observer.h"

#include "base/time/time.h"
#include "chrome/browser/android/history_report/delta_file_commons.h"
#include "chrome/browser/android/history_report/delta_file_service.h"
#include "chrome/browser/android/history_report/usage_report_util.h"
#include "chrome/browser/android/history_report/usage_reports_buffer_service.h"
#include "components/bookmarks/browser/bookmark_model.h"
#include "components/history/core/browser/history_service.h"

using bookmarks::BookmarkModel;
using bookmarks::BookmarkNode;

namespace history_report {

DataObserver::DataObserver(
    base::Callback<void(void)> data_changed_callback,
    base::Callback<void(void)> data_cleared_callback,
    base::Callback<void(void)> stop_reporting_callback,
    DeltaFileService* delta_file_service,
    UsageReportsBufferService* usage_reports_buffer_service,
    BookmarkModel* bookmark_model,
    history::HistoryService* history_service)
    : data_changed_callback_(data_changed_callback),
      data_cleared_callback_(data_cleared_callback),
      stop_reporting_callback_(stop_reporting_callback),
      delta_file_service_(delta_file_service),
      usage_reports_buffer_service_(usage_reports_buffer_service) {
  bookmark_model->AddObserver(this);
  history_service->AddObserver(this);
}

DataObserver::~DataObserver() {}

void DataObserver::BookmarkModelLoaded(BookmarkModel* model,
                                       bool ids_reassigned) {}

void DataObserver::BookmarkModelBeingDeleted(BookmarkModel* model) {}

void DataObserver::BookmarkNodeMoved(BookmarkModel* model,
                                     const BookmarkNode* old_parent,
                                     int old_index,
                                     const BookmarkNode* new_parent,
                                     int new_index) {}

void DataObserver::BookmarkNodeAdded(BookmarkModel* model,
                                     const BookmarkNode* parent,
                                     int index) {
  delta_file_service_->PageAdded(parent->GetChild(index)->url());
  data_changed_callback_.Run();
}

void DataObserver::BookmarkNodeRemoved(BookmarkModel* model,
                                       const BookmarkNode* parent,
                                       int old_index,
                                       const BookmarkNode* node,
                                       const std::set<GURL>& removed_urls) {
  DeleteBookmarks(removed_urls);
}

void DataObserver::BookmarkAllUserNodesRemoved(
    BookmarkModel* model,
    const std::set<GURL>& removed_urls) {
  DeleteBookmarks(removed_urls);
}

void DataObserver::BookmarkNodeChanged(BookmarkModel* model,
                                       const BookmarkNode* node) {
  delta_file_service_->PageAdded(node->url());
  data_changed_callback_.Run();
}

void DataObserver::BookmarkNodeFaviconChanged(BookmarkModel* model,
                                              const BookmarkNode* node) {}

void DataObserver::BookmarkNodeChildrenReordered(BookmarkModel* model,
                                                 const BookmarkNode* node) {}

void DataObserver::DeleteBookmarks(const std::set<GURL>& removed_urls) {
  for (std::set<GURL>::const_iterator it = removed_urls.begin();
       it != removed_urls.end();
       ++it) {
    delta_file_service_->PageDeleted(*it);
  }
  if (!removed_urls.empty())
    data_changed_callback_.Run();
}

void DataObserver::OnURLVisited(history::HistoryService* history_service,
                                ui::PageTransition transition,
                                const history::URLRow& row,
                                const history::RedirectList& redirects,
                                base::Time visit_time) {
  if (row.hidden() || ui::PageTransitionIsRedirect(transition))
    return;

  delta_file_service_->PageAdded(row.url());
  // TODO(haaawk): check if this is really a data change not just a
  //               visit of already seen page.
  data_changed_callback_.Run();
  std::string id = DeltaFileEntryWithData::UrlToId(row.url().spec());
  usage_reports_buffer_service_->AddVisit(
      id,
      visit_time.ToJavaTime(),
      usage_report_util::IsTypedVisit(transition));
  // We stop any usage reporting to wait for gmscore to query the provider
  // for this url. We do not want to report usage for a URL which might
  // not be known to gmscore.
  stop_reporting_callback_.Run();
}

void DataObserver::OnURLsModified(history::HistoryService* history_service,
                                  const history::URLRows& changed_urls) {
  for (const auto& row : changed_urls) {
    if (!row.hidden())
      delta_file_service_->PageAdded(row.url());
  }

  data_changed_callback_.Run();
}

void DataObserver::OnURLsDeleted(history::HistoryService* history_service,
                                 bool all_history,
                                 bool expired,
                                 const history::URLRows& deleted_rows,
                                 const std::set<GURL>& favicon_urls) {
  if (all_history) {
    delta_file_service_->Clear();
    data_cleared_callback_.Run();
  } else {
    for (const auto& row : deleted_rows) {
      if (!row.hidden())
        delta_file_service_->PageDeleted(row.url());
    }

    data_changed_callback_.Run();
  }
}

}  // namespace history_report
