// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STORAGE_BROWSER_FILEAPI_FILE_SYSTEM_BACKEND_H_
#define STORAGE_BROWSER_FILEAPI_FILE_SYSTEM_BACKEND_H_

#include <stdint.h>

#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "base/callback_forward.h"
#include "base/files/file.h"
#include "base/files/file_path.h"
#include "storage/browser/fileapi/file_permission_policy.h"
#include "storage/browser/fileapi/open_file_system_mode.h"
#include "storage/browser/fileapi/task_runner_bound_observer_list.h"
#include "storage/browser/storage_browser_export.h"
#include "storage/common/fileapi/file_system_types.h"

class GURL;

namespace storage {

class AsyncFileUtil;
class CopyOrMoveFileValidatorFactory;
class FileSystemURL;
class FileStreamReader;
class FileStreamWriter;
class FileSystemContext;
class FileSystemOperation;
class FileSystemQuotaUtil;
class WatcherManager;

// Callback to take GURL.
typedef base::Callback<void(const GURL& url)> URLCallback;

// Maximum numer of bytes to be read by FileStreamReader classes. Used in
// FileSystemBackend::CreateFileStreamReader(), when it's not known how many
// bytes will be fetched in total.
const int64_t kMaximumLength = INT64_MAX;

// An interface for defining a file system backend.
//
// NOTE: when you implement a new FileSystemBackend for your own
// FileSystem module, please contact to kinuko@chromium.org.
//
class STORAGE_EXPORT FileSystemBackend {
 public:
  // Callback for InitializeFileSystem.
  using OpenFileSystemCallback =
      base::OnceCallback<void(const GURL& root_url,
                              const std::string& name,
                              base::File::Error error)>;
  virtual ~FileSystemBackend() {}

  // Returns true if this filesystem backend can handle |type|.
  // One filesystem backend may be able to handle multiple filesystem types.
  virtual bool CanHandleType(FileSystemType type) const = 0;

  // This method is called right after the backend is registered in the
  // FileSystemContext and before any other methods are called. Each backend can
  // do additional initialization which depends on FileSystemContext here.
  virtual void Initialize(FileSystemContext* context) = 0;

  // Resolves the filesystem root URL and the name for the given |url|.
  // This verifies if it is allowed to request (or create) the filesystem and if
  // it can access (or create) the root directory.
  // If |mode| is CREATE_IF_NONEXISTENT calling this may also create the root
  // directory (and/or related database entries etc) for the filesystem if it
  // doesn't exist.
  virtual void ResolveURL(const FileSystemURL& url,
                          OpenFileSystemMode mode,
                          OpenFileSystemCallback callback) = 0;

  // Returns the specialized AsyncFileUtil for this backend.
  virtual AsyncFileUtil* GetAsyncFileUtil(FileSystemType type) = 0;

  // Returns the specialized WatcherManager for this backend.
  virtual WatcherManager* GetWatcherManager(FileSystemType type) = 0;

  // Returns the specialized CopyOrMoveFileValidatorFactory for this backend
  // and |type|.  If |error_code| is File::FILE_OK and the result is NULL,
  // then no validator is required.
  virtual CopyOrMoveFileValidatorFactory* GetCopyOrMoveFileValidatorFactory(
      FileSystemType type, base::File::Error* error_code) = 0;

  // Returns a new instance of the specialized FileSystemOperation for this
  // backend based on the given triplet of |origin_url|, |file_system_type|
  // and |virtual_path|. On failure to create a file system operation, set
  // |error_code| correspondingly.
  // This method is usually dispatched by
  // FileSystemContext::CreateFileSystemOperation.
  virtual FileSystemOperation* CreateFileSystemOperation(
      const FileSystemURL& url,
      FileSystemContext* context,
      base::File::Error* error_code) const = 0;

  // Returns true if Blobs accessing |url| should use FileStreamReader.
  // If false, Blobs are accessed using a snapshot file by calling
  // AsyncFileUtil::CreateSnapshotFile.
  virtual bool SupportsStreaming(const FileSystemURL& url) const = 0;

  // Returns true if specified |type| of filesystem can handle Copy()
  // of the files in the same file system instead of streaming
  // read/write implementation.
  virtual bool HasInplaceCopyImplementation(FileSystemType type) const = 0;

  // Creates a new file stream reader for a given filesystem URL |url| with an
  // offset |offset|. |expected_modification_time| specifies the expected last
  // modification if the value is non-null, the reader will check the underlying
  // file's actual modification time to see if the file has been modified, and
  // if it does any succeeding read operations should fail with
  // ERR_UPLOAD_FILE_CHANGED error.
  // This method itself does *not* check if the given path exists and is a
  // regular file. At most |max_bytes_to_read| can be fetched from the file
  // stream reader.
  virtual std::unique_ptr<storage::FileStreamReader> CreateFileStreamReader(
      const FileSystemURL& url,
      int64_t offset,
      int64_t max_bytes_to_read,
      const base::Time& expected_modification_time,
      FileSystemContext* context) const = 0;

  // Creates a new file stream writer for a given filesystem URL |url| with an
  // offset |offset|.
  // This method itself does *not* check if the given path exists and is a
  // regular file.
  virtual std::unique_ptr<FileStreamWriter> CreateFileStreamWriter(
      const FileSystemURL& url,
      int64_t offset,
      FileSystemContext* context) const = 0;

  // Returns the specialized FileSystemQuotaUtil for this backend.
  // This could return NULL if this backend does not support quota.
  virtual FileSystemQuotaUtil* GetQuotaUtil() = 0;

  // Returns the update observer list for |type|. It may return NULL when no
  // observers are added.
  virtual const UpdateObserverList* GetUpdateObservers(
      FileSystemType type) const = 0;

  // Returns the change observer list for |type|. It may return NULL when no
  // observers are added.
  virtual const ChangeObserverList* GetChangeObservers(
      FileSystemType type) const = 0;

  // Returns the access observer list for |type|. It may return NULL when no
  // observers are added.
  virtual const AccessObserverList* GetAccessObservers(
      FileSystemType type) const = 0;
};

// An interface to control external file system access permissions.
// TODO(satorux): Move this out of 'storage/browser/fileapi'. crbug.com/257279
class ExternalFileSystemBackend : public FileSystemBackend {
 public:
  // Returns true if |url| is allowed to be accessed.
  // This is supposed to perform ExternalFileSystem-specific security
  // checks.
  virtual bool IsAccessAllowed(const storage::FileSystemURL& url) const = 0;
  // Returns the list of top level directories that are exposed by this
  // provider. This list is used to set appropriate child process file access
  // permissions.
  virtual std::vector<base::FilePath> GetRootDirectories() const = 0;
  // Grants access to |virtual_path| from |origin_url|.
  virtual void GrantFileAccessToExtension(
      const std::string& extension_id,
      const base::FilePath& virtual_path) = 0;
  // Revokes file access from extension identified with |extension_id|.
  virtual void RevokeAccessForExtension(
        const std::string& extension_id) = 0;
  // Gets virtual path by known filesystem path. Returns false when filesystem
  // path is not exposed by this provider.
  virtual bool GetVirtualPath(const base::FilePath& file_system_path,
                              base::FilePath* virtual_path) const = 0;
  // Gets a redirect URL for contents. e.g. Google Drive URL for hosted
  // documents. Returns empty URL if the entry does not have the redirect URL.
  virtual void GetRedirectURLForContents(
      const storage::FileSystemURL& url,
      const storage::URLCallback& callback) const = 0;
  // Creates an internal File System URL for performing internal operations such
  // as confirming if a file or a directory exist before granting the final
  // permission to the entry. The path must be an absolute path.
  virtual storage::FileSystemURL CreateInternalURL(
      storage::FileSystemContext* context,
      const base::FilePath& entry_path) const = 0;
};

}  // namespace storage

#endif  // STORAGE_BROWSER_FILEAPI_FILE_SYSTEM_BACKEND_H_
