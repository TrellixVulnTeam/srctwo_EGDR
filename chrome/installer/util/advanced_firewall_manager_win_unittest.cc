// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/installer/util/advanced_firewall_manager_win.h"

#include <stddef.h>

#include "base/macros.h"
#include "base/path_service.h"
#include "base/process/process_info.h"
#include "base/win/scoped_bstr.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace installer {

class AdvancedFirewallManagerTest : public ::testing::Test {
 public:
  AdvancedFirewallManagerTest() : skip_test_(true) {}

 protected:
  // Sets up the test fixture.
  void SetUp() override {
    if (base::GetCurrentProcessIntegrityLevel() != base::HIGH_INTEGRITY) {
      LOG(WARNING) << "Not elevated. Skipping the test.";
      return;
    }
    skip_test_ = false;
    base::FilePath exe_path;
    PathService::Get(base::FILE_EXE, &exe_path);
    EXPECT_TRUE(manager_.Init(L"AdvancedFirewallManagerTest", exe_path));
    manager_.DeleteAllRules();
  }

  // Tears down the test fixture.
  void TearDown() override {
    if (!skip_test_)
      manager_.DeleteAllRules();
  }

  // Forwards calls to |manager_| to avoid making each test a friend of
  // |AdvancedFirewallManager|.
  void GetAllRules(std::vector<base::string16>* rule_names) {
    std::vector<base::win::ScopedComPtr<INetFwRule> > rules;
    manager_.GetAllRules(&rules);
    for (size_t i = 0; i < rules.size(); ++i) {
      base::win::ScopedBstr name;
      EXPECT_TRUE(SUCCEEDED(rules[i]->get_Name(name.Receive())));
      EXPECT_TRUE(name);
      rule_names->push_back(base::string16(name));
    }
  }

  bool skip_test_;
  AdvancedFirewallManager manager_;

 private:
  DISALLOW_COPY_AND_ASSIGN(AdvancedFirewallManagerTest);
};

TEST_F(AdvancedFirewallManagerTest, NoRule) {
  if (skip_test_)
    return;
  std::vector<base::string16> rule_names;
  GetAllRules(&rule_names);
  EXPECT_TRUE(rule_names.empty());
}

TEST_F(AdvancedFirewallManagerTest, AddRule) {
  if (skip_test_)
    return;
  const wchar_t kRuleName[] = L"Port56789";
  EXPECT_TRUE(manager_.AddUDPRule(kRuleName, L"Test Description", 56789));

  std::vector<base::string16> rule_names;
  GetAllRules(&rule_names);
  ASSERT_EQ(1u, rule_names.size());
  EXPECT_EQ(rule_names[0], kRuleName);
  EXPECT_TRUE(manager_.HasAnyRule());

  manager_.DeleteRuleByName(kRuleName);
  rule_names.clear();
  GetAllRules(&rule_names);
  EXPECT_TRUE(rule_names.empty());
  EXPECT_FALSE(manager_.HasAnyRule());
}

}  // namespace installer
