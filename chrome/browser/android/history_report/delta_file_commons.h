// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ANDROID_HISTORY_REPORT_DELTA_FILE_COMMONS_H_
#define CHROME_BROWSER_ANDROID_HISTORY_REPORT_DELTA_FILE_COMMONS_H_

#include <stdint.h>

#include "base/strings/string16.h"
#include "chrome/browser/android/proto/delta_file.pb.h"
#include "components/bookmarks/browser/bookmark_model.h"
#include "components/history/core/browser/history_types.h"

namespace history_report {

// Represents a single delta file entry with additional data.
class DeltaFileEntryWithData {
 public:
  explicit DeltaFileEntryWithData(DeltaFileEntry entry);
  DeltaFileEntryWithData(const DeltaFileEntryWithData& other);
  ~DeltaFileEntryWithData();

  // Returns sequence number of this delta file entry.
  int64_t SeqNo() const;

  // Returns type of delta file entry ('add' or 'del').
  std::string Type() const;

  // Returns ID of delta file entry.
  std::string Id() const;

  // Returns url of delta file entry.
  std::string Url() const;

  // Returns title of delta file entry.
  base::string16 Title() const;

  // Returns ranking score of url described by delta file entry.
  int32_t Score() const;

  // Returns part of delta file entry's url
  // which will be used as a key in search index.
  std::string IndexedUrl() const;

  // Checks if delta file entry is valid and can be returned as a result.
  // Entries with missing data or the one that are hidden are not valid.
  bool Valid() const;

  // Adds additional data to delta file entry.
  void SetData(const history::URLRow& data);

  // Marks delta file entry as bookmark.
  void MarkAsBookmark(const bookmarks::BookmarkModel::URLAndTitle& bookmark);

  // Whether given url can be used as Id.
  static bool IsValidId(const std::string& url);

  // Generates a unique id for a given url
  static std::string UrlToId(const std::string& url);

 private:
  DeltaFileEntry entry_;
  bool data_set_;
  history::URLRow data_;
  bool is_bookmark_;
  base::string16 bookmark_title_;
};

}  // namespace history_report

#endif  // CHROME_BROWSER_ANDROID_HISTORY_REPORT_DELTA_FILE_COMMONS_H_
