// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_FILE_SYSTEM_PROVIDER_OPERATIONS_GET_ACTIONS_H_
#define CHROME_BROWSER_CHROMEOS_FILE_SYSTEM_PROVIDER_OPERATIONS_GET_ACTIONS_H_

#include <memory>
#include <vector>

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

// Bridge between fileapi get actions operation and providing extension's get
// actions request. Created per request.
class GetActions : public Operation {
 public:
  GetActions(extensions::EventRouter* event_router,
             const ProvidedFileSystemInfo& file_system_info,
             const std::vector<base::FilePath>& entry_paths,
             ProvidedFileSystemInterface::GetActionsCallback callback);
  ~GetActions() override;

  // Operation overrides.
  bool Execute(int request_id) override;
  void OnSuccess(int request_id,
                 std::unique_ptr<RequestValue> result,
                 bool has_more) override;
  void OnError(int request_id,
               std::unique_ptr<RequestValue> result,
               base::File::Error error) override;

 private:
  const std::vector<base::FilePath> entry_paths_;
  ProvidedFileSystemInterface::GetActionsCallback callback_;

  DISALLOW_COPY_AND_ASSIGN(GetActions);
};

}  // namespace operations
}  // namespace file_system_provider
}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_FILE_SYSTEM_PROVIDER_OPERATIONS_GET_ACTIONS_H_
