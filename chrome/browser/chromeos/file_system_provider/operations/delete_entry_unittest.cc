// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/file_system_provider/operations/delete_entry.h"

#include <memory>
#include <string>
#include <vector>

#include "base/files/file.h"
#include "base/files/file_path.h"
#include "chrome/browser/chromeos/file_system_provider/operations/test_util.h"
#include "chrome/browser/chromeos/file_system_provider/provided_file_system_interface.h"
#include "chrome/common/extensions/api/file_system_provider.h"
#include "chrome/common/extensions/api/file_system_provider_capabilities/file_system_provider_capabilities_handler.h"
#include "chrome/common/extensions/api/file_system_provider_internal.h"
#include "extensions/browser/event_router.h"
#include "storage/browser/fileapi/async_file_util.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace chromeos {
namespace file_system_provider {
namespace operations {
namespace {

const char kExtensionId[] = "mbflcebpggnecokmikipoihdbecnjfoj";
const char kFileSystemId[] = "testing-file-system";
const int kRequestId = 2;
const base::FilePath::CharType kEntryPath[] =
    FILE_PATH_LITERAL("/kitty/and/puppy/happy");

}  // namespace

class FileSystemProviderOperationsDeleteEntryTest : public testing::Test {
 protected:
  FileSystemProviderOperationsDeleteEntryTest() {}
  ~FileSystemProviderOperationsDeleteEntryTest() override {}

  void SetUp() override {
    MountOptions mount_options(kFileSystemId, "" /* display_name */);
    mount_options.writable = true;
    file_system_info_ = ProvidedFileSystemInfo(
        kExtensionId, mount_options, base::FilePath(), false /* configurable */,
        true /* watchable */, extensions::SOURCE_FILE);
  }

  ProvidedFileSystemInfo file_system_info_;
};

TEST_F(FileSystemProviderOperationsDeleteEntryTest, Execute) {
  using extensions::api::file_system_provider::DeleteEntryRequestedOptions;

  util::LoggingDispatchEventImpl dispatcher(true /* dispatch_reply */);
  util::StatusCallbackLog callback_log;

  DeleteEntry delete_entry(NULL, file_system_info_, base::FilePath(kEntryPath),
                           true /* recursive */,
                           base::Bind(&util::LogStatusCallback, &callback_log));
  delete_entry.SetDispatchEventImplForTesting(
      base::Bind(&util::LoggingDispatchEventImpl::OnDispatchEventImpl,
                 base::Unretained(&dispatcher)));

  EXPECT_TRUE(delete_entry.Execute(kRequestId));

  ASSERT_EQ(1u, dispatcher.events().size());
  extensions::Event* event = dispatcher.events()[0].get();
  EXPECT_EQ(
      extensions::api::file_system_provider::OnDeleteEntryRequested::kEventName,
      event->event_name);
  base::ListValue* event_args = event->event_args.get();
  ASSERT_EQ(1u, event_args->GetSize());

  const base::DictionaryValue* options_as_value = NULL;
  ASSERT_TRUE(event_args->GetDictionary(0, &options_as_value));

  DeleteEntryRequestedOptions options;
  ASSERT_TRUE(
      DeleteEntryRequestedOptions::Populate(*options_as_value, &options));
  EXPECT_EQ(kFileSystemId, options.file_system_id);
  EXPECT_EQ(kRequestId, options.request_id);
  EXPECT_EQ(kEntryPath, options.entry_path);
  EXPECT_TRUE(options.recursive);
}

TEST_F(FileSystemProviderOperationsDeleteEntryTest, Execute_NoListener) {
  util::LoggingDispatchEventImpl dispatcher(false /* dispatch_reply */);
  util::StatusCallbackLog callback_log;

  DeleteEntry delete_entry(NULL, file_system_info_, base::FilePath(kEntryPath),
                           true /* recursive */,
                           base::Bind(&util::LogStatusCallback, &callback_log));
  delete_entry.SetDispatchEventImplForTesting(
      base::Bind(&util::LoggingDispatchEventImpl::OnDispatchEventImpl,
                 base::Unretained(&dispatcher)));

  EXPECT_FALSE(delete_entry.Execute(kRequestId));
}

TEST_F(FileSystemProviderOperationsDeleteEntryTest, Execute_ReadOnly) {
  util::LoggingDispatchEventImpl dispatcher(true /* dispatch_reply */);
  util::StatusCallbackLog callback_log;

  const ProvidedFileSystemInfo read_only_file_system_info(
      kExtensionId, MountOptions(kFileSystemId, "" /* display_name */),
      base::FilePath() /* mount_path */, false /* configurable */,
      true /* watchable */, extensions::SOURCE_FILE);

  DeleteEntry delete_entry(NULL, read_only_file_system_info,
                           base::FilePath(kEntryPath), true /* recursive */,
                           base::Bind(&util::LogStatusCallback, &callback_log));
  delete_entry.SetDispatchEventImplForTesting(
      base::Bind(&util::LoggingDispatchEventImpl::OnDispatchEventImpl,
                 base::Unretained(&dispatcher)));

  EXPECT_FALSE(delete_entry.Execute(kRequestId));
}

TEST_F(FileSystemProviderOperationsDeleteEntryTest, OnSuccess) {
  util::LoggingDispatchEventImpl dispatcher(true /* dispatch_reply */);
  util::StatusCallbackLog callback_log;

  DeleteEntry delete_entry(NULL, file_system_info_, base::FilePath(kEntryPath),
                           true /* recursive */,
                           base::Bind(&util::LogStatusCallback, &callback_log));
  delete_entry.SetDispatchEventImplForTesting(
      base::Bind(&util::LoggingDispatchEventImpl::OnDispatchEventImpl,
                 base::Unretained(&dispatcher)));

  EXPECT_TRUE(delete_entry.Execute(kRequestId));

  delete_entry.OnSuccess(kRequestId,
                         std::unique_ptr<RequestValue>(new RequestValue()),
                         false /* has_more */);
  ASSERT_EQ(1u, callback_log.size());
  EXPECT_EQ(base::File::FILE_OK, callback_log[0]);
}

TEST_F(FileSystemProviderOperationsDeleteEntryTest, OnError) {
  util::LoggingDispatchEventImpl dispatcher(true /* dispatch_reply */);
  util::StatusCallbackLog callback_log;

  DeleteEntry delete_entry(NULL, file_system_info_, base::FilePath(kEntryPath),
                           true /* recursive */,
                           base::Bind(&util::LogStatusCallback, &callback_log));
  delete_entry.SetDispatchEventImplForTesting(
      base::Bind(&util::LoggingDispatchEventImpl::OnDispatchEventImpl,
                 base::Unretained(&dispatcher)));

  EXPECT_TRUE(delete_entry.Execute(kRequestId));

  delete_entry.OnError(kRequestId,
                       std::unique_ptr<RequestValue>(new RequestValue()),
                       base::File::FILE_ERROR_TOO_MANY_OPENED);
  ASSERT_EQ(1u, callback_log.size());
  EXPECT_EQ(base::File::FILE_ERROR_TOO_MANY_OPENED, callback_log[0]);
}

}  // namespace operations
}  // namespace file_system_provider
}  // namespace chromeos
