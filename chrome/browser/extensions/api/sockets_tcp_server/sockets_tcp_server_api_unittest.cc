// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "base/memory/ptr_util.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "base/values.h"
#include "chrome/browser/browser_process_impl.h"
#include "chrome/browser/extensions/extension_api_unittest.h"
#include "chrome/browser/extensions/test_extension_system.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/test/base/testing_browser_process.h"
#include "extensions/browser/api/api_resource_manager.h"
#include "extensions/browser/api/socket/socket.h"
#include "extensions/browser/api/socket/tcp_socket.h"
#include "extensions/browser/api/sockets_tcp_server/sockets_tcp_server_api.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace extensions {
namespace api {

static std::unique_ptr<KeyedService> ApiResourceManagerTestFactory(
    content::BrowserContext* context) {
  return base::MakeUnique<ApiResourceManager<ResumableTCPSocket>>(context);
}

static std::unique_ptr<KeyedService> ApiResourceManagerTestServerFactory(
    content::BrowserContext* context) {
  return base::MakeUnique<ApiResourceManager<ResumableTCPServerSocket>>(
      context);
}

class SocketsTcpServerUnitTest : public ExtensionApiUnittest {
 public:
  void SetUp() override {
    ExtensionApiUnittest::SetUp();

    ApiResourceManager<ResumableTCPSocket>::GetFactoryInstance()
        ->SetTestingFactoryAndUse(browser()->profile(),
                                  ApiResourceManagerTestFactory);

    ApiResourceManager<ResumableTCPServerSocket>::GetFactoryInstance()
        ->SetTestingFactoryAndUse(browser()->profile(),
                                  ApiResourceManagerTestServerFactory);
  }
};

TEST_F(SocketsTcpServerUnitTest, Create) {
  // Create SocketCreateFunction and put it on BrowserThread
  SocketsTcpServerCreateFunction* function =
      new SocketsTcpServerCreateFunction();
  function->set_work_task_runner(base::SequencedTaskRunnerHandle::Get());

  // Run tests
  std::unique_ptr<base::DictionaryValue> result(RunFunctionAndReturnDictionary(
      function, "[{\"persistent\": true, \"name\": \"foo\"}]"));
  ASSERT_TRUE(result.get());
}

}  // namespace api
}  // namespace extensions
