// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "google_apis/drive/base_requests.h"

#include <memory>

#include "base/bind.h"
#include "base/files/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/memory/ptr_util.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "google_apis/drive/dummy_auth_service.h"
#include "google_apis/drive/request_sender.h"
#include "google_apis/drive/task_util.h"
#include "google_apis/drive/test_util.h"
#include "net/test/embedded_test_server/embedded_test_server.h"
#include "net/test/embedded_test_server/http_request.h"
#include "net/test/embedded_test_server/http_response.h"
#include "net/traffic_annotation/network_traffic_annotation_test_helper.h"
#include "net/url_request/url_request_test_util.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace google_apis {

namespace {

const char kTestUserAgent[] = "test-user-agent";

}  // namespace

class BaseRequestsServerTest : public testing::Test {
 protected:
  BaseRequestsServerTest() {
  }

  void SetUp() override {
    ASSERT_TRUE(temp_dir_.CreateUniqueTempDir());

    request_context_getter_ = new net::TestURLRequestContextGetter(
        message_loop_.task_runner());

    request_sender_.reset(
        new RequestSender(new DummyAuthService, request_context_getter_.get(),
                          message_loop_.task_runner(), kTestUserAgent,
                          TRAFFIC_ANNOTATION_FOR_TESTS));

    ASSERT_TRUE(test_server_.InitializeAndListen());
    test_server_.RegisterRequestHandler(
        base::Bind(&test_util::HandleDownloadFileRequest,
                   test_server_.base_url(),
                   base::Unretained(&http_request_)));
    test_server_.StartAcceptingConnections();
  }

  // Returns a temporary file path suitable for storing the cache file.
  base::FilePath GetTestCachedFilePath(const base::FilePath& file_name) {
    return temp_dir_.GetPath().Append(file_name);
  }

  base::MessageLoopForIO message_loop_;  // Test server needs IO thread.
  net::EmbeddedTestServer test_server_;
  std::unique_ptr<RequestSender> request_sender_;
  scoped_refptr<net::TestURLRequestContextGetter> request_context_getter_;
  base::ScopedTempDir temp_dir_;

  // The incoming HTTP request is saved so tests can verify the request
  // parameters like HTTP method (ex. some requests should use DELETE
  // instead of GET).
  net::test_server::HttpRequest http_request_;
};

TEST_F(BaseRequestsServerTest, DownloadFileRequest_ValidFile) {
  DriveApiErrorCode result_code = DRIVE_OTHER_ERROR;
  base::FilePath temp_file;
  {
    base::RunLoop run_loop;
    std::unique_ptr<DownloadFileRequestBase> request =
        base::MakeUnique<DownloadFileRequestBase>(
            request_sender_.get(),
            test_util::CreateQuitCallback(
                &run_loop,
                test_util::CreateCopyResultCallback(&result_code, &temp_file)),
            GetContentCallback(), ProgressCallback(),
            test_server_.GetURL("/files/drive/testfile.txt"),
            GetTestCachedFilePath(
                base::FilePath::FromUTF8Unsafe("cached_testfile.txt")));
    request_sender_->StartRequestWithAuthRetry(std::move(request));
    run_loop.Run();
  }

  std::string contents;
  base::ReadFileToString(temp_file, &contents);
  base::DeleteFile(temp_file, false);

  EXPECT_EQ(HTTP_SUCCESS, result_code);
  EXPECT_EQ(net::test_server::METHOD_GET, http_request_.method);
  EXPECT_EQ("/files/drive/testfile.txt", http_request_.relative_url);

  const base::FilePath expected_path =
      test_util::GetTestFilePath("drive/testfile.txt");
  std::string expected_contents;
  base::ReadFileToString(expected_path, &expected_contents);
  EXPECT_EQ(expected_contents, contents);
}

TEST_F(BaseRequestsServerTest, DownloadFileRequest_NonExistentFile) {
  DriveApiErrorCode result_code = DRIVE_OTHER_ERROR;
  base::FilePath temp_file;
  {
    base::RunLoop run_loop;
    std::unique_ptr<DownloadFileRequestBase> request =
        base::MakeUnique<DownloadFileRequestBase>(
            request_sender_.get(),
            test_util::CreateQuitCallback(
                &run_loop,
                test_util::CreateCopyResultCallback(&result_code, &temp_file)),
            GetContentCallback(), ProgressCallback(),
            test_server_.GetURL("/files/gdata/no-such-file.txt"),
            GetTestCachedFilePath(
                base::FilePath::FromUTF8Unsafe("cache_no-such-file.txt")));
    request_sender_->StartRequestWithAuthRetry(std::move(request));
    run_loop.Run();
  }
  EXPECT_EQ(HTTP_NOT_FOUND, result_code);
  EXPECT_EQ(net::test_server::METHOD_GET, http_request_.method);
  EXPECT_EQ("/files/gdata/no-such-file.txt",
            http_request_.relative_url);
  // Do not verify the not found message.
}

}  // namespace google_apis
