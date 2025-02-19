// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/sync_file_system/syncable_file_system_util.h"

#include <vector>

#include "base/command_line.h"
#include "base/location.h"
#include "base/single_thread_task_runner.h"
#include "base/strings/string_util.h"
#include "base/threading/thread_task_runner_handle.h"
#include "storage/browser/fileapi/external_mount_points.h"
#include "storage/browser/fileapi/file_observers.h"
#include "storage/browser/fileapi/file_system_context.h"
#include "storage/common/fileapi/file_system_util.h"

using storage::ExternalMountPoints;
using storage::FileSystemContext;
using storage::FileSystemURL;

namespace sync_file_system {

namespace {

const char kSyncableMountName[] = "syncfs";
const char kSyncableMountNameForInternalSync[] = "syncfs-internal";

const base::FilePath::CharType kSyncFileSystemDir[] =
    FILE_PATH_LITERAL("Sync FileSystem");

void Noop() {}

}  // namespace

void RegisterSyncableFileSystem() {
  ExternalMountPoints::GetSystemInstance()->RegisterFileSystem(
      kSyncableMountName,
      storage::kFileSystemTypeSyncable,
      storage::FileSystemMountOption(),
      base::FilePath());
  ExternalMountPoints::GetSystemInstance()->RegisterFileSystem(
      kSyncableMountNameForInternalSync,
      storage::kFileSystemTypeSyncableForInternalSync,
      storage::FileSystemMountOption(),
      base::FilePath());
}

void RevokeSyncableFileSystem() {
  ExternalMountPoints::GetSystemInstance()->RevokeFileSystem(
      kSyncableMountName);
  ExternalMountPoints::GetSystemInstance()->RevokeFileSystem(
      kSyncableMountNameForInternalSync);
}

GURL GetSyncableFileSystemRootURI(const GURL& origin) {
  return GURL(
      storage::GetExternalFileSystemRootURIString(origin, kSyncableMountName));
}

FileSystemURL CreateSyncableFileSystemURL(const GURL& origin,
                                          const base::FilePath& path) {
  base::FilePath path_for_url = path;
  if (storage::VirtualPath::IsAbsolute(path.value()))
    path_for_url = base::FilePath(path.value().substr(1));

  return ExternalMountPoints::GetSystemInstance()->CreateExternalFileSystemURL(
      origin, kSyncableMountName, path_for_url);
}

FileSystemURL CreateSyncableFileSystemURLForSync(
    storage::FileSystemContext* file_system_context,
    const FileSystemURL& syncable_url) {
  return ExternalMountPoints::GetSystemInstance()->CreateExternalFileSystemURL(
      syncable_url.origin(),
      kSyncableMountNameForInternalSync,
      syncable_url.path());
}

bool SerializeSyncableFileSystemURL(const FileSystemURL& url,
                                    std::string* serialized_url) {
  if (!url.is_valid() || url.type() != storage::kFileSystemTypeSyncable)
    return false;
  *serialized_url =
      GetSyncableFileSystemRootURI(url.origin()).spec() +
      url.path().AsUTF8Unsafe();
  return true;
}

bool DeserializeSyncableFileSystemURL(
    const std::string& serialized_url, FileSystemURL* url) {
#if !defined(FILE_PATH_USES_WIN_SEPARATORS)
  DCHECK(serialized_url.find('\\') == std::string::npos);
#endif  // FILE_PATH_USES_WIN_SEPARATORS

  FileSystemURL deserialized =
      ExternalMountPoints::GetSystemInstance()->CrackURL(GURL(serialized_url));
  if (!deserialized.is_valid() ||
      deserialized.type() != storage::kFileSystemTypeSyncable) {
    return false;
  }

  *url = deserialized;
  return true;
}

base::FilePath GetSyncFileSystemDir(const base::FilePath& profile_base_dir) {
  return profile_base_dir.Append(kSyncFileSystemDir);
}

void RunSoon(const tracked_objects::Location& from_here,
             const base::Closure& callback) {
  base::ThreadTaskRunnerHandle::Get()->PostTask(from_here, callback);
}

base::Closure NoopClosure() {
  return base::Bind(&Noop);
}

}  // namespace sync_file_system
