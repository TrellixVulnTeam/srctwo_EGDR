// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_FILTERS_FILE_DATA_SOURCE_H_
#define MEDIA_FILTERS_FILE_DATA_SOURCE_H_

#include <stdint.h>

#include <string>

#include "base/files/file.h"
#include "base/files/file_path.h"
#include "base/files/memory_mapped_file.h"
#include "base/macros.h"
#include "media/base/data_source.h"

namespace media {

// Basic data source that treats the URL as a file path, and uses the file
// system to read data for a media pipeline.
class MEDIA_EXPORT FileDataSource : public DataSource {
 public:
  FileDataSource();
  explicit FileDataSource(base::File file);
  ~FileDataSource() override;

  bool Initialize(const base::FilePath& file_path);

  // Implementation of DataSource.
  void Stop() override;
  void Abort() override;
  void Read(int64_t position,
            int size,
            uint8_t* data,
            const DataSource::ReadCB& read_cb) override;
  bool GetSize(int64_t* size_out) override;
  bool IsStreaming() override;
  void SetBitrate(int bitrate) override;

  // Unit test helpers. Recreate the object if you want the default behaviour.
  void force_read_errors_for_testing() { force_read_errors_ = true; }
  void force_streaming_for_testing() { force_streaming_ = true; }
  uint64_t bytes_read_for_testing() { return bytes_read_; }
  void reset_bytes_read_for_testing() { bytes_read_ = 0; }

 private:
  base::MemoryMappedFile file_;

  bool force_read_errors_;
  bool force_streaming_;
  uint64_t bytes_read_;

  DISALLOW_COPY_AND_ASSIGN(FileDataSource);
};

}  // namespace media

#endif  // MEDIA_FILTERS_FILE_DATA_SOURCE_H_
