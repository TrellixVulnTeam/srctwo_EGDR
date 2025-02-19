// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_HISTORY_CORE_BROWSER_VISIT_DELEGATE_H_
#define COMPONENTS_HISTORY_CORE_BROWSER_VISIT_DELEGATE_H_

#include <vector>

#include "base/macros.h"

class GURL;

namespace history {

class HistoryService;

// VisitDelegate gets notified about URLs recorded as visited by the
// HistoryService.
class VisitDelegate {
 public:
  VisitDelegate();
  virtual ~VisitDelegate();

  // Called once HistoryService initialization is complete. Returns true if the
  // initialization succeeded, false otherwise.
  virtual bool Init(HistoryService* history_service) = 0;

  // Called when an URL is recorded by HistoryService.
  virtual void AddURL(const GURL& url) = 0;

  // Called when a list of URLs are recorded by HistoryService.
  virtual void AddURLs(const std::vector<GURL>& urls) = 0;

  // Called when a list of URLs are removed from HistoryService.
  virtual void DeleteURLs(const std::vector<GURL>& urls) = 0;

  // Called when all URLs are removed from HistoryService.
  virtual void DeleteAllURLs() = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(VisitDelegate);
};

}  // namespace history

#endif  // COMPONENTS_HISTORY_CORE_BROWSER_VISIT_DELEGATE_H_
