// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/extension_with_management_policy_apitest.h"
#include "components/policy/core/browser/browser_policy_connector.h"
#include "net/dns/mock_host_resolver.h"
#include "net/test/embedded_test_server/http_request.h"

ExtensionApiTestWithManagementPolicy::ExtensionApiTestWithManagementPolicy()
    : ExtensionApiTest() {}

ExtensionApiTestWithManagementPolicy::~ExtensionApiTestWithManagementPolicy() {}

void ExtensionApiTestWithManagementPolicy::SetUpInProcessBrowserTestFixture() {
  ExtensionApiTest::SetUpInProcessBrowserTestFixture();
  embedded_test_server()->RegisterRequestMonitor(
      base::Bind(&ExtensionApiTestWithManagementPolicy::MonitorRequestHandler,
                 base::Unretained(this)));
  EXPECT_CALL(policy_provider_, IsInitializationComplete(testing::_))
      .WillRepeatedly(testing::Return(true));
  policy_provider_.SetAutoRefresh();
  policy::BrowserPolicyConnector::SetPolicyProviderForTesting(
      &policy_provider_);
}

void ExtensionApiTestWithManagementPolicy::SetUpOnMainThread() {
  ExtensionApiTest::SetUpOnMainThread();
  host_resolver()->AddRule("*", "127.0.0.1");
}

void ExtensionApiTestWithManagementPolicy::MonitorRequestHandler(
    const net::test_server::HttpRequest& request) {
  auto host = request.headers.find("Host");
  if (host != request.headers.end()) {
    ManagementPolicyRequestLog log;
    size_t delimiter_pos = host->second.find(":");
    log.host = host->second.substr(0, delimiter_pos);
    request_log_.push_back(log);
  }
}

bool ExtensionApiTestWithManagementPolicy::BrowsedTo(
    const std::string& test_host) {
  return std::find_if(request_log_.begin(), request_log_.end(),
                      [test_host](const ManagementPolicyRequestLog& log) {
                        return log.host == test_host;
                      }) != request_log_.end();
}

void ExtensionApiTestWithManagementPolicy::ClearRequestLog() {
  request_log_.clear();
}
