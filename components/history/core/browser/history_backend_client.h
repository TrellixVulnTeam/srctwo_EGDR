// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_HISTORY_CORE_BROWSER_HISTORY_BACKEND_CLIENT_H_
#define COMPONENTS_HISTORY_CORE_BROWSER_HISTORY_BACKEND_CLIENT_H_

#include <vector>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "build/build_config.h"
#include "url/gurl.h"

namespace base {
class FilePath;
}

namespace history {

class HistoryBackend;
class HistoryDatabase;
class ThumbnailDatabase;

struct URLAndTitle {
  GURL url;
  base::string16 title;
};

class HistoryBackendClient {
 public:
  HistoryBackendClient() {}
  virtual ~HistoryBackendClient() {}

  // Returns true if the specified URL is bookmarked.
  virtual bool IsBookmarked(const GURL& url) = 0;

  // Returns, by reference in |bookmarks|, the set of bookmarked URLs and their
  // title. This returns the unique set of URLs. For example, if two bookmarks
  // reference the same URL only one entry is added even if the title are not
  // the same.
  virtual void GetBookmarks(std::vector<URLAndTitle>* bookmarks) = 0;

  // Returns whether database errors should be reported to the crash server.
  virtual bool ShouldReportDatabaseError() = 0;

  // Returns whether |url| should be considered web-safe (see
  // content::ChildProcessSecurityPolicy).
  virtual bool IsWebSafe(const GURL& url) = 0;

#if defined(OS_ANDROID)
  // Called upon initialization of the HistoryBackend.
  virtual void OnHistoryBackendInitialized(
      HistoryBackend* history_backend,
      HistoryDatabase* history_database,
      ThumbnailDatabase* thumbnail_database,
      const base::FilePath& history_dir) = 0;

  // Called upon destruction of the HistoryBackend.
  virtual void OnHistoryBackendDestroyed(HistoryBackend* history_backend,
                                         const base::FilePath& history_dir) = 0;
#endif  // defined(OS_ANDROID)

 private:
  DISALLOW_COPY_AND_ASSIGN(HistoryBackendClient);
};

}  // namespace history

#endif  // COMPONENTS_HISTORY_CORE_BROWSER_HISTORY_BACKEND_CLIENT_H_
