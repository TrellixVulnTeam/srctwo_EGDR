// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_FILE_SYSTEM_PROVIDER_PROVIDED_FILE_SYSTEM_INFO_H_
#define CHROME_BROWSER_CHROMEOS_FILE_SYSTEM_PROVIDER_PROVIDED_FILE_SYSTEM_INFO_H_

#include <string>

#include "base/files/file_path.h"
#include "chrome/common/extensions/api/file_system_provider_capabilities/file_system_provider_capabilities_handler.h"

namespace chromeos {
namespace file_system_provider {

// Options for creating the provided file system info.
struct MountOptions {
  MountOptions();

  // Only mandatory fields.
  MountOptions(const std::string& file_system_id,
               const std::string& display_name);

  std::string file_system_id;
  std::string display_name;
  bool writable;
  bool supports_notify_tag;
  int opened_files_limit;
};

// Contains information about the provided file system instance.
class ProvidedFileSystemInfo {
 public:
  ProvidedFileSystemInfo();

  ProvidedFileSystemInfo(const std::string& extension_id,
                         const MountOptions& mount_options,
                         const base::FilePath& mount_path,
                         bool configurable,
                         bool watchable,
                         extensions::FileSystemProviderSource source);

  ProvidedFileSystemInfo(const ProvidedFileSystemInfo& other);

  ~ProvidedFileSystemInfo();

  const std::string& extension_id() const { return extension_id_; }
  const std::string& file_system_id() const { return file_system_id_; }
  const std::string& display_name() const { return display_name_; }
  bool writable() const { return writable_; }
  bool supports_notify_tag() const { return supports_notify_tag_; }
  int opened_files_limit() const { return opened_files_limit_; }
  const base::FilePath& mount_path() const { return mount_path_; }
  bool configurable() const { return configurable_; }
  bool watchable() const { return watchable_; }
  extensions::FileSystemProviderSource source() const { return source_; }

 private:
  // ID of the extension providing this file system.
  std::string extension_id_;

  // ID of the file system.
  std::string file_system_id_;

  // Name of the file system, can be rendered in the UI.
  std::string display_name_;

  // Whether the file system is writable or just read-only.
  bool writable_;

  // Supports tags for file/directory change notifications.
  bool supports_notify_tag_;

  // Limit of opened files in parallel. If unlimited, then 0.
  int opened_files_limit_;

  // Mount path of the underlying file system.
  base::FilePath mount_path_;

  // Whether the file system is configurable.
  bool configurable_;

  // Whether the file system is watchable.
  bool watchable_;

  // Source of the file system's data.
  extensions::FileSystemProviderSource source_;
};

}  // namespace file_system_provider
}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_FILE_SYSTEM_PROVIDER_PROVIDED_FILE_SYSTEM_INFO_H_
