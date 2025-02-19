// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/signin/signin_error_handler.h"

#include "base/memory/ptr_util.h"
#include "base/values.h"
#include "chrome/browser/ui/browser_commands.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/browser/ui/webui/signin/signin_error_ui.h"
#include "chrome/common/url_constants.h"
#include "chrome/test/base/browser_with_test_window_test.h"
#include "chrome/test/base/dialog_test_browser_window.h"
#include "chrome/test/base/testing_profile.h"
#include "content/public/test/test_web_ui.h"

namespace {
const char kSigninErrorLearnMoreUrl[] =
    "https://support.google.com/chrome/answer/1181420?";

class TestingSigninErrorHandler : public SigninErrorHandler {
 public:
  TestingSigninErrorHandler(Browser* browser,
                            bool is_system_profile,
                            content::WebUI* web_ui)
      : SigninErrorHandler(browser, is_system_profile),
        browser_modal_dialog_did_close_(false),
        user_manager_profile_dialog_did_close_(false) {
    set_web_ui(web_ui);
  }

  void CloseBrowserModalSigninDialog() override {
    browser_modal_dialog_did_close_ = true;
    SigninErrorHandler::CloseBrowserModalSigninDialog();
  }

  void CloseUserManagerProfileDialog() override {
    user_manager_profile_dialog_did_close_ = true;
    SigninErrorHandler::CloseUserManagerProfileDialog();
  }

  using SigninErrorHandler::HandleSwitchToExistingProfile;
  using SigninErrorHandler::HandleConfirm;
  using SigninErrorHandler::HandleLearnMore;
  using SigninErrorHandler::HandleInitializedWithSize;

  bool browser_modal_dialog_did_close() {
    return browser_modal_dialog_did_close_;
  }

  bool user_manager_profile_dialog_did_close() {
    return user_manager_profile_dialog_did_close_;
  }

 private:
  bool browser_modal_dialog_did_close_;
  bool user_manager_profile_dialog_did_close_;

  DISALLOW_COPY_AND_ASSIGN(TestingSigninErrorHandler);
};

class SigninErrorHandlerTest : public BrowserWithTestWindowTest {
 public:
  SigninErrorHandlerTest()
      : web_ui_(new content::TestWebUI), handler_(nullptr) {}

  void SetUp() override {
    BrowserWithTestWindowTest::SetUp();
    chrome::NewTab(browser());
    web_ui()->set_web_contents(
        browser()->tab_strip_model()->GetActiveWebContents());
    signin_error_ui_.reset(new SigninErrorUI(web_ui()));
  }

  void TearDown() override {
    signin_error_ui_.reset();
    web_ui_.reset();
    BrowserWithTestWindowTest::TearDown();
  }

  void CreateHandlerInBrowser() {
    DCHECK(!handler_);
    auto handler = base::MakeUnique<TestingSigninErrorHandler>(
        browser(), false /* is_system_profile */, web_ui());
    handler_ = handler.get();
    signin_error_ui_.reset(new SigninErrorUI(web_ui()));
    web_ui()->AddMessageHandler(std::move(handler));
  }

  void CreateHandlerInUserManager() {
    DCHECK(!handler_);
    auto handler = base::MakeUnique<TestingSigninErrorHandler>(
        nullptr /* browser */, true /* is_system_profile */, web_ui());
    handler_ = handler.get();
    web_ui()->AddMessageHandler(std::move(handler));
  }

  TestingSigninErrorHandler* handler() { return handler_; }

  content::TestWebUI* web_ui() { return web_ui_.get(); }

  // BrowserWithTestWindowTest
  BrowserWindow* CreateBrowserWindow() override {
    return new DialogTestBrowserWindow;
  }

 private:
  std::unique_ptr<content::TestWebUI> web_ui_;
  std::unique_ptr<SigninErrorUI> signin_error_ui_;
  TestingSigninErrorHandler* handler_;  // Not owned.

  DISALLOW_COPY_AND_ASSIGN(SigninErrorHandlerTest);
};

TEST_F(SigninErrorHandlerTest, InBrowserHandleLearnMore) {
  // Before the test, there is only one new tab opened.
  TabStripModel* tab_strip_model = browser()->tab_strip_model();
  EXPECT_EQ(1, tab_strip_model->count());
  EXPECT_EQ(GURL(chrome::kChromeUINewTabURL),
            tab_strip_model->GetActiveWebContents()->GetURL());

  // Open learn more
  CreateHandlerInBrowser();
  base::ListValue args;
  handler()->HandleLearnMore(&args);

  // Dialog should be closed now.
  EXPECT_TRUE(handler()->browser_modal_dialog_did_close());

  // Verify that the learn more URL was opened.
  EXPECT_EQ(2, tab_strip_model->count());
  EXPECT_EQ(GURL(kSigninErrorLearnMoreUrl),
            tab_strip_model->GetActiveWebContents()->GetURL());
}

TEST_F(SigninErrorHandlerTest, InBrowserHandleLearnMoreAfterBrowserRemoved) {
  // Before the test, there is only one new tab opened.
  TabStripModel* tab_strip_model = browser()->tab_strip_model();
  EXPECT_EQ(1, tab_strip_model->count());
  EXPECT_EQ(GURL(chrome::kChromeUINewTabURL),
            tab_strip_model->GetActiveWebContents()->GetURL());

  // Inform the handler that the browser was removed;
  CreateHandlerInBrowser();
  handler()->OnBrowserRemoved(browser());

  // Open learn more
  base::ListValue args;
  handler()->HandleLearnMore(&args);

  // Dialog is not closed if the browser was removed.
  EXPECT_FALSE(handler()->browser_modal_dialog_did_close());

  // Verify that the learn more URL was not opened as the browser was removed.
  EXPECT_EQ(1, tab_strip_model->count());
  EXPECT_EQ(GURL(chrome::kChromeUINewTabURL),
            tab_strip_model->GetActiveWebContents()->GetURL());
}

TEST_F(SigninErrorHandlerTest, InBrowserTestConfirm) {
  CreateHandlerInBrowser();
  base::ListValue args;
  handler()->HandleConfirm(&args);

  // Confirm simply closes the dialog.
  EXPECT_TRUE(handler()->browser_modal_dialog_did_close());
}

TEST_F(SigninErrorHandlerTest, InUserManagerTestConfirm) {
  CreateHandlerInUserManager();
  base::ListValue args;
  handler()->HandleConfirm(&args);

  // Confirm simply closes the dialog.
  EXPECT_TRUE(handler()->user_manager_profile_dialog_did_close());
}

}  // namespace
