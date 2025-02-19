// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This file provides MIME related utilities.

#ifndef EXTENSIONS_BROWSER_API_FILE_HANDLERS_MIME_UTIL_H_
#define EXTENSIONS_BROWSER_API_FILE_HANDLERS_MIME_UTIL_H_

#include <stddef.h>

#include <memory>
#include <string>
#include <vector>

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"

namespace content {
class BrowserContext;
}

namespace base {
class FilePath;
}  // namespace base

namespace storage {
class FileSystemURL;
}  // namespace storage

namespace extensions {
namespace app_file_handler_util {

// Gets a MIME type for a local path and returns it with |callback|. If not
// found, then the MIME type is an empty string.
void GetMimeTypeForLocalPath(
    content::BrowserContext* context,
    const base::FilePath& local_path,
    const base::Callback<void(const std::string&)>& callback);

// Collects MIME types for files passed in the input vector. For non-native
// file systems tries to fetch the MIME type from metadata. For native ones,
// tries to sniff or guess by looking at the extension. If MIME type is not
// available, then an empty string is returned in the result vector.
class MimeTypeCollector {
 public:
  typedef base::Callback<void(std::unique_ptr<std::vector<std::string>>)>
      CompletionCallback;

  explicit MimeTypeCollector(content::BrowserContext* context);
  virtual ~MimeTypeCollector();

  // Collects all mime types asynchronously for a vector of URLs and upon
  // completion, calls the |callback|. It can be called only once.
  void CollectForURLs(const std::vector<storage::FileSystemURL>& urls,
                      const CompletionCallback& callback);

  // Collects all mime types asynchronously for a vector of local file paths and
  // upon completion, calls the |callback|. It can be called only once.
  void CollectForLocalPaths(const std::vector<base::FilePath>& local_paths,
                            const CompletionCallback& callback);

 private:
  // Called, when the |index|-th input file (or URL) got processed.
  void OnMimeTypeCollected(size_t index, const std::string& mime_type);

  content::BrowserContext* context_;
  std::unique_ptr<std::vector<std::string>> result_;
  size_t left_;
  CompletionCallback callback_;
  base::WeakPtrFactory<MimeTypeCollector> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(MimeTypeCollector);
};

}  // namespace app_file_handler_util
}  // namespace extensions

#endif  // EXTENSIONS_BROWSER_API_FILE_HANDLERS_MIME_UTIL_H_
