// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_COMMON_MEDIA_CDM_HOST_FILE_H_
#define CONTENT_COMMON_MEDIA_CDM_HOST_FILE_H_

#include <memory>

#include "base/files/file.h"
#include "base/files/file_path.h"
#include "base/macros.h"

namespace cdm {
struct HostFile;
}

namespace content {

// Represents a file that participated in hosting the CDM.
class CdmHostFile {
 public:
  // Opens the file at |file_path| and the corresponding signature file at
  // |sig_file_path|. Upon success, constructs and returns a CdmHostFile object.
  // Otherwise returns nullptr. The opened files are closed when |this| is
  // destructed unless TakePlatformFile() was called, in which case the caller
  // must make sure the files are closed properly.
  static std::unique_ptr<CdmHostFile> Create(
      const base::FilePath& file_path,
      const base::FilePath& sig_file_path);

  // Takes the PlatformFile of the |file_| and |sig_file_| and put them in the
  // returned cdm::HostFile. The caller must make sure the PlatformFiles are
  // properly closed after use.
  cdm::HostFile TakePlatformFile();

 private:
  CdmHostFile(const base::FilePath& file_path,
              base::File file,
              base::File sig_file);

  base::FilePath file_path_;
  base::File file_;

  // The signature file associated with |file_|.
  base::File sig_file_;

  DISALLOW_COPY_AND_ASSIGN(CdmHostFile);
};

}  // namespace content

#endif  // CONTENT_COMMON_MEDIA_CDM_HOST_FILE_H_
