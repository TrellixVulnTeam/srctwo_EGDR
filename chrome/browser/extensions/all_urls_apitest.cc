// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "base/macros.h"
#include "build/build_config.h"
#include "chrome/browser/extensions/extension_apitest.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/test/base/ui_test_utils.h"
#include "components/crx_file/id_util.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/common/extension.h"
#include "extensions/common/extensions_client.h"
#include "extensions/test/extension_test_message_listener.h"
#include "net/test/embedded_test_server/embedded_test_server.h"

namespace extensions {

namespace {
const std::string kAllUrlsTarget = "/extensions/api_test/all_urls/index.html";
}

class AllUrlsApiTest : public ExtensionApiTest {
 protected:
  AllUrlsApiTest() {}
  ~AllUrlsApiTest() override {}

  const Extension* content_script() const { return content_script_.get(); }
  const Extension* execute_script() const { return execute_script_.get(); }

  void WhitelistExtensions() {
    ExtensionsClient::ScriptingWhitelist whitelist;
    whitelist.push_back(content_script_->id());
    whitelist.push_back(execute_script_->id());
    ExtensionsClient::Get()->SetScriptingWhitelist(whitelist);
    // Extensions will have certain permissions withheld at initialization if
    // they aren't whitelisted, so we need to reload them.
    ExtensionTestMessageListener listener("execute: ready", false);
    extension_service()->ReloadExtension(content_script_->id());
    extension_service()->ReloadExtension(execute_script_->id());
    ASSERT_TRUE(listener.WaitUntilSatisfied());
  }

  void NavigateAndWait(const std::string& url) {
    ExtensionTestMessageListener listener_a("content script: " + url, false);
    ExtensionTestMessageListener listener_b("execute: " + url, false);
    ui_test_utils::NavigateToURL(browser(), GURL(url));
    ASSERT_TRUE(listener_a.WaitUntilSatisfied());
    ASSERT_TRUE(listener_b.WaitUntilSatisfied());
  }

 private:
  void SetUpOnMainThread() override {
    ExtensionApiTest::SetUpOnMainThread();
    base::FilePath data_dir = test_data_dir_.AppendASCII("all_urls");
    content_script_ = LoadExtension(data_dir.AppendASCII("content_script"));
    ASSERT_TRUE(content_script_);
    execute_script_ = LoadExtension(data_dir.AppendASCII("execute_script"));
    ASSERT_TRUE(execute_script_);
  }

  scoped_refptr<const Extension> content_script_;
  scoped_refptr<const Extension> execute_script_;

  DISALLOW_COPY_AND_ASSIGN(AllUrlsApiTest);
};

IN_PROC_BROWSER_TEST_F(AllUrlsApiTest, WhitelistedExtension) {
  WhitelistExtensions();

  auto* bystander = LoadExtension(
      test_data_dir_.AppendASCII("all_urls").AppendASCII("bystander"));
  ASSERT_TRUE(bystander);

  ASSERT_TRUE(StartEmbeddedTestServer());

  // Now verify that we run content scripts on different URLs, including
  // internal pages, data URLs, regular HTTP pages, and resource URLs from
  // extensions.
  const std::string test_urls[] = {
    "chrome://newtab/",
    "data:text/html;charset=utf-8,<html>asdf</html>",
    "chrome://version/",
    "about:blank",
    embedded_test_server()->GetURL(kAllUrlsTarget).spec(),
    bystander->GetResourceURL("page.html").spec()
  };
  for (const auto& test_url : test_urls)
    NavigateAndWait(test_url);
}

// Test that an extension NOT whitelisted for scripting can ask for <all_urls>
// and run scripts on non-restricted all pages.
IN_PROC_BROWSER_TEST_F(AllUrlsApiTest, RegularExtensions) {
  // Now verify we can script a regular http page.
  ASSERT_TRUE(StartEmbeddedTestServer());
  NavigateAndWait(embedded_test_server()->GetURL(kAllUrlsTarget).spec());
}

}  // namespace extensions
