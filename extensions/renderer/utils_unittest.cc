// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/strings/stringprintf.h"
#include "extensions/common/feature_switch.h"
#include "extensions/grit/extensions_renderer_resources.h"
#include "extensions/renderer/module_system_test.h"
#include "gin/dictionary.h"

namespace extensions {
namespace {

class UtilsUnittest : public ModuleSystemTest {
 public:
  void SetUp() override {
    ModuleSystemTest::SetUp();

    env()->RegisterModule("utils", IDR_UTILS_JS);
    env()->RegisterTestFile("utils_unittest", "utils_unittest.js");
    env()->OverrideNativeHandler("schema_registry",
                                 "exports.$set('GetSchema', function() {});");
    env()->OverrideNativeHandler("logging",
                                 "exports.$set('CHECK', function() {});\n"
                                 "exports.$set('DCHECK', function() {});\n"
                                 "exports.$set('WARNING', function() {});");
    env()->OverrideNativeHandler("v8_context", "");

    // Native bindings set up the chrome.runtime accessor, so we don't need to
    // stub it out.
    if (FeatureSwitch::native_crx_bindings()->IsEnabled())
      return;

    gin::Dictionary chrome(env()->isolate(), env()->CreateGlobal("chrome"));
    gin::Dictionary chrome_runtime(
        gin::Dictionary::CreateEmpty(env()->isolate()));
    chrome.Set("runtime", chrome_runtime);
  }

  void RunTest(const std::string& test_name) { RunTestImpl(test_name, false); }

  void RunTestWithPromises(const std::string& test_name) {
    RunTestImpl(test_name, true);
  }

 private:
  void RunTestImpl(const std::string& test_name, bool run_promises) {
    ModuleSystem::NativesEnabledScope natives_enabled_scope(
        env()->module_system());
    ASSERT_FALSE(env()
                     ->module_system()
                     ->Require("utils_unittest")
                     .ToLocalChecked()
                     .IsEmpty());
    env()->module_system()->CallModuleMethodSafe("utils_unittest", test_name);
    if (run_promises)
      RunResolvedPromises();
  }
};

TEST_F(UtilsUnittest, TestNothing) {
  ExpectNoAssertionsMade();
}

TEST_F(UtilsUnittest, SuperClass) {
  RunTest("testSuperClass");
}

}  // namespace
}  // namespace extensions
