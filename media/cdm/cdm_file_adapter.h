// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_CDM_CDM_FILE_ADAPTER_H_
#define MEDIA_CDM_CDM_FILE_ADAPTER_H_

#include <stdint.h>

#include <string>
#include <vector>

#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/macros.h"
#include "media/base/media_export.h"
#include "media/cdm/api/content_decryption_module.h"
#include "media/cdm/cdm_file_io.h"

namespace media {

// This class provides the ability to read and write a file using cdm::FileIO.
class MEDIA_EXPORT CdmFileAdapter : public cdm::FileIOClient {
 public:
  enum class Status { kSuccess, kInUse, kError };
  using FileOpenedCB = base::OnceCallback<void(Status status)>;
  using ReadCB =
      base::OnceCallback<void(bool success, const std::vector<uint8_t>& data)>;
  using WriteCB = base::OnceCallback<void(bool success)>;

  explicit CdmFileAdapter(cdm::ContentDecryptionModule_9::Host* host);
  ~CdmFileAdapter() override;

  // Open the file with |name|. |open_cb| will be called when the file is
  // available.
  void Open(const std::string& name, FileOpenedCB open_cb);

  // Read the contents of the file, calling |read_cb| with the contents when
  // done. If the file does not exist Read() will succeed but |data| will
  // be empty.
  void Read(ReadCB read_cb);

  // Write |data| into the file, replacing any existing contents. Due to the
  // way Read() works, files will be considered deleted if the file is empty,
  // so write 0 bytes to "delete" the file.
  void Write(const std::vector<uint8_t>& data, WriteCB write_cb);

 private:
  // cdm::FileIOClient implementation.
  // These are private as they should be called by the cdm::FileIO
  // implementation only.
  void OnOpenComplete(cdm::FileIOClient::Status status) override;
  void OnReadComplete(cdm::FileIOClient::Status status,
                      const uint8_t* data,
                      uint32_t data_size) override;
  void OnWriteComplete(cdm::FileIOClient::Status status) override;

  FileOpenedCB open_cb_;
  ReadCB read_cb_;
  WriteCB write_cb_;
  cdm::FileIO* file_io_;

  DISALLOW_COPY_AND_ASSIGN(CdmFileAdapter);
};

}  // namespace media

#endif  // MEDIA_CDM_CDM_FILE_ADAPTER_H_
