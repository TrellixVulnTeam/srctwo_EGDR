// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXO_FILE_HELPER_H_
#define COMPONENTS_EXO_FILE_HELPER_H_

#include <string>

class GURL;

namespace base {
class FilePath;
}

namespace exo {

class FileHelper {
 public:
  virtual ~FileHelper() {}

  // Returns mime type which is used for list of Uris returned by this
  // FileHelper.
  virtual std::string GetMimeTypeForUriList() const = 0;

  // Convert natife file path to URL which can be used in container.  We don't
  // expose enter file system to a container directly.  Instead we mount
  // specific directory in the containers' namespace.  Thus we need to convert
  // native path to file URL which points mount point in containers.  The
  // conversion should be container specific, now we only have ARC container
  // though.
  virtual bool ConvertPathToUrl(const base::FilePath& path, GURL* out) = 0;
};

}  // namespace exo

#endif  // COMPONENTS_EXO_FILE_HELPER_H_
