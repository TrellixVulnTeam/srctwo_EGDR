// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_FILE_SYSTEM_PROVIDER_OPERATIONS_GET_METADATA_H_
#define CHROME_BROWSER_CHROMEOS_FILE_SYSTEM_PROVIDER_OPERATIONS_GET_METADATA_H_

#include <memory>
#include <string>

#include "base/files/file.h"
#include "base/macros.h"
#include "chrome/browser/chromeos/file_system_provider/operations/operation.h"
#include "chrome/browser/chromeos/file_system_provider/provided_file_system_info.h"
#include "chrome/browser/chromeos/file_system_provider/provided_file_system_interface.h"
#include "chrome/browser/chromeos/file_system_provider/request_value.h"
#include "chrome/common/extensions/api/file_system_provider_internal.h"

namespace base {
class FilePath;
}  // namespace base

namespace extensions {
class EventRouter;
}  // namespace extensions

namespace chromeos {
namespace file_system_provider {
namespace operations {

// Validates the metadata. If it's incorrect (eg. incorrect characters in the
// name or empty for non-root), then returns false.
bool ValidateIDLEntryMetadata(
    const extensions::api::file_system_provider::EntryMetadata& metadata,
    int fields,
    bool root_entry);

// Checks whether the passed name is valid or not.
bool ValidateName(const std::string& name, bool root_entry);

// Bridge between fileapi get metadata operation and providing extension's get
// metadata request. Created per request.
class GetMetadata : public Operation {
 public:
  GetMetadata(extensions::EventRouter* event_router,
              const ProvidedFileSystemInfo& file_system_info,
              const base::FilePath& entry_path,
              ProvidedFileSystemInterface::MetadataFieldMask fields,
              ProvidedFileSystemInterface::GetMetadataCallback callback);
  ~GetMetadata() override;

  // Operation overrides.
  bool Execute(int request_id) override;
  void OnSuccess(int request_id,
                 std::unique_ptr<RequestValue> result,
                 bool has_more) override;
  void OnError(int request_id,
               std::unique_ptr<RequestValue> result,
               base::File::Error error) override;

 private:
  base::FilePath entry_path_;
  ProvidedFileSystemInterface::MetadataFieldMask fields_;
  ProvidedFileSystemInterface::GetMetadataCallback callback_;

  DISALLOW_COPY_AND_ASSIGN(GetMetadata);
};

}  // namespace operations
}  // namespace file_system_provider
}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_FILE_SYSTEM_PROVIDER_OPERATIONS_GET_METADATA_H_
