// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_FILE_SYSTEM_PROVIDER_FILEAPI_FILE_STREAM_READER_H_
#define CHROME_BROWSER_CHROMEOS_FILE_SYSTEM_PROVIDER_FILEAPI_FILE_STREAM_READER_H_

#include <stdint.h>

#include <memory>

#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "storage/browser/fileapi/file_stream_reader.h"
#include "storage/browser/fileapi/file_system_url.h"

namespace chromeos {
namespace file_system_provider {

struct EntryMetadata;
class ProvidedFileSystemInterface;

// Implements a streamed file reader. It is lazily initialized by the first call
// to Read().
class FileStreamReader : public storage::FileStreamReader {
 public:
  typedef base::Callback<
      void(base::WeakPtr<ProvidedFileSystemInterface> file_system,
           const base::FilePath& file_path,
           int file_handle,
           base::File::Error result)> OpenFileCompletedCallback;

  FileStreamReader(storage::FileSystemContext* context,
                   const storage::FileSystemURL& url,
                   int64_t initial_offset,
                   const base::Time& expected_modification_time);

  ~FileStreamReader() override;

  // storage::FileStreamReader overrides.
  int Read(net::IOBuffer* buf,
           int buf_len,
           const net::CompletionCallback& callback) override;
  int64_t GetLength(const net::Int64CompletionCallback& callback) override;

 private:
  // Helper class for executing operations on the provided file system. All
  // of its methods must be called on UI thread. Callbacks are called on IO
  // thread.
  class OperationRunner;

  // State of the file stream reader.
  enum State { NOT_INITIALIZED, INITIALIZING, INITIALIZED, FAILED };

  // Called when Read() operation is completed with either a success of an
  // error.
  void OnReadCompleted(net::CompletionCallback callback, int result);

  // Initializes the reader by opening the file. When completed with success,
  // runs the |pending_closure|. Otherwise, calls the |error_callback|.
  void Initialize(const base::Closure& pending_closure,
                  const net::Int64CompletionCallback& error_callback);

  // Called when opening a file is completed with either a success or an error.
  void OnOpenFileCompleted(
      const base::Closure& pending_closure,
      const net::Int64CompletionCallback& error_callback,
      base::File::Error result);

  // Called when initialization is completed with either a success or an error.
  void OnInitializeCompleted(const base::Closure& pending_closure,
                             const net::Int64CompletionCallback& error_callback,
                             std::unique_ptr<EntryMetadata> metadata,
                             base::File::Error result);

  // Called when a file system provider returns chunk of read data. Note, that
  // this may be called multiple times per single Read() call, as long as
  // |has_more| is set to true. |result| is set to success only if reading is
  // successful, and the file has not changed while reading.
  void OnReadChunkReceived(const net::CompletionCallback& callback,
                           int chunk_length,
                           bool has_more,
                           base::File::Error result);

  // Called when fetching length of the file is completed with either a success
  // or an error.
  void OnGetMetadataForGetLengthReceived(
      const net::Int64CompletionCallback& callback,
      std::unique_ptr<EntryMetadata> metadata,
      base::File::Error result);

  // Same as Read(), but called after initializing is completed.
  void ReadAfterInitialized(scoped_refptr<net::IOBuffer> buffer,
                            int buffer_length,
                            const net::CompletionCallback& callback);

  // Same as GetLength(), but called after initializing is completed.
  void GetLengthAfterInitialized(const net::Int64CompletionCallback& callback);

  storage::FileSystemURL url_;
  int64_t current_offset_;
  int64_t current_length_;
  base::Time expected_modification_time_;
  scoped_refptr<OperationRunner> runner_;
  State state_;

  base::WeakPtrFactory<FileStreamReader> weak_ptr_factory_;
  DISALLOW_COPY_AND_ASSIGN(FileStreamReader);
};

}  // namespace file_system_provider
}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_FILE_SYSTEM_PROVIDER_FILEAPI_FILE_STREAM_READER_H_
