// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/common/media/cdm_host_file.h"

#include <memory>

#include "base/command_line.h"
#include "base/feature_list.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "media/cdm/api/content_decryption_module_ext.h"

namespace content {

// static
std::unique_ptr<CdmHostFile> CdmHostFile::Create(
    const base::FilePath& file_path,
    const base::FilePath& sig_file_path) {
  DVLOG(1) << __func__;

  // Open file at |file_path|.
  base::File file(file_path, base::File::FLAG_OPEN | base::File::FLAG_READ);
  DVLOG(1) << "  " << file.IsValid() << ": " << file_path.MaybeAsASCII();

  // Also open the sig file at |sig_file_path|.
  base::File sig_file(sig_file_path,
                      base::File::FLAG_OPEN | base::File::FLAG_READ);
  DVLOG(1) << "  " << sig_file.IsValid() << ": "
           << sig_file_path.MaybeAsASCII();

  return std::unique_ptr<CdmHostFile>(
      new CdmHostFile(file_path, std::move(file), std::move(sig_file)));
}

cdm::HostFile CdmHostFile::TakePlatformFile() {
  return cdm::HostFile(file_path_.value().c_str(), file_.TakePlatformFile(),
                       sig_file_.TakePlatformFile());
}

CdmHostFile::CdmHostFile(const base::FilePath& file_path,
                         base::File file,
                         base::File sig_file)
    : file_path_(file_path),
      file_(std::move(file)),
      sig_file_(std::move(sig_file)) {
  DCHECK(!file_path_.empty());
}

}  // namespace content
