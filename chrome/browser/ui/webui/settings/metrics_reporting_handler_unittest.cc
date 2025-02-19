// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if defined(GOOGLE_CHROME_BUILD) && !defined(OS_CHROMEOS)

#include "chrome/browser/ui/webui/settings/metrics_reporting_handler.h"

#include "base/memory/ptr_util.h"
#include "base/run_loop.h"
#include "base/values.h"
#include "chrome/test/base/scoped_testing_local_state.h"
#include "chrome/test/base/testing_browser_process.h"
#include "components/metrics/metrics_pref_names.h"
#include "components/policy/core/browser/browser_policy_connector.h"
#include "components/policy/core/common/mock_configuration_policy_provider.h"
#include "components/policy/core/common/policy_map.h"
#include "components/policy/core/common/policy_types.h"
#include "components/policy/policy_constants.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/web_ui.h"
#include "content/public/test/test_browser_thread_bundle.h"
#include "content/public/test/test_web_ui.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace settings {

class TestingMetricsReportingHandler : public MetricsReportingHandler {
 public:
  using MetricsReportingHandler::set_web_ui;
  using MetricsReportingHandler::HandleGetMetricsReporting;
};

class MetricsReportingHandlerTest : public testing::Test {
 public:
  MetricsReportingHandlerTest() {
    // Local state must be set up before |handler_|.
    local_state_.reset(new ScopedTestingLocalState(
        TestingBrowserProcess::GetGlobal()));

    handler_.reset(new TestingMetricsReportingHandler);
    handler_->set_web_ui(&test_web_ui_);

    EXPECT_CALL(provider_, IsInitializationComplete(testing::_)).WillRepeatedly(
        testing::Return(true));
    policy::BrowserPolicyConnector::SetPolicyProviderForTesting(&provider_);
  }

  void SetUp() override {
    ASSERT_EQ(local_state(), g_browser_process->local_state());
    EXPECT_TRUE(test_web_ui()->call_data().empty());

    base::ListValue args;
    args.Append(base::MakeUnique<base::Value>(1));
    handler()->HandleGetMetricsReporting(&args);

    EXPECT_TRUE(handler()->IsJavascriptAllowed());
    EXPECT_EQ(1u, test_web_ui()->call_data().size());

    test_web_ui()->ClearTrackedCalls();
  }

  void TearDown() override {
    // For crbug.com/637068 which only run on official bots with no try jobs.
    base::RunLoop().RunUntilIdle();
    handler_.reset();
    base::RunLoop().RunUntilIdle();
    local_state_.reset();
    base::RunLoop().RunUntilIdle();
  }

  PrefService* local_state() { return local_state_->Get(); }
  TestingMetricsReportingHandler* handler() { return handler_.get(); }
  content::TestWebUI* test_web_ui() { return &test_web_ui_; }
  policy::PolicyMap* map() { return &map_; }
  policy::MockConfigurationPolicyProvider* provider() { return &provider_; }

 private:
  content::TestBrowserThreadBundle thread_bundle_;
  content::TestWebUI test_web_ui_;
  std::unique_ptr<ScopedTestingLocalState> local_state_;
  std::unique_ptr<TestingMetricsReportingHandler> handler_;

  policy::MockConfigurationPolicyProvider provider_;
  policy::PolicyMap map_;
};

TEST_F(MetricsReportingHandlerTest, PrefChangesNotifyPage) {
  // Toggle the pref.
  local_state()->SetBoolean(
      metrics::prefs::kMetricsReportingEnabled,
      !local_state()->GetBoolean(metrics::prefs::kMetricsReportingEnabled));
  EXPECT_EQ(1u, test_web_ui()->call_data().size());

  test_web_ui()->ClearTrackedCalls();
  handler()->DisallowJavascript();

  // Toggle the pref again, while JavaScript is disabled.
  local_state()->SetBoolean(
      metrics::prefs::kMetricsReportingEnabled,
      !local_state()->GetBoolean(metrics::prefs::kMetricsReportingEnabled));
  EXPECT_TRUE(test_web_ui()->call_data().empty());
}

TEST_F(MetricsReportingHandlerTest, PolicyChangesNotifyPage) {
  // Change the policy, check that the page was notified.
  map()->Set(policy::key::kMetricsReportingEnabled,
             policy::POLICY_LEVEL_MANDATORY, policy::POLICY_SCOPE_USER,
             policy::POLICY_SOURCE_CLOUD, base::MakeUnique<base::Value>(true),
             nullptr);
  provider()->UpdateChromePolicy(*map());
  EXPECT_EQ(1u, test_web_ui()->call_data().size());

  test_web_ui()->ClearTrackedCalls();
  handler()->DisallowJavascript();

  // Policies changing while JavaScript is disabled shouldn't notify the page.
  map()->Set(policy::key::kMetricsReportingEnabled,
             policy::POLICY_LEVEL_MANDATORY, policy::POLICY_SCOPE_USER,
             policy::POLICY_SOURCE_CLOUD, base::MakeUnique<base::Value>(false),
             nullptr);
  provider()->UpdateChromePolicy(*map());
  EXPECT_TRUE(test_web_ui()->call_data().empty());
}

}  // namespace settings

#endif  // defined(GOOGLE_CHROME_BUILD) && !defined(OS_CHROMEOS)
