// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "chrome/browser/ui/cocoa/ssl_client_certificate_selector_cocoa.h"

#import <SecurityInterface/SFChooseIdentityPanel.h>

#include "base/bind.h"
#import "base/mac/mac_util.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "chrome/browser/ssl/ssl_client_certificate_selector.h"
#include "chrome/browser/ssl/ssl_client_certificate_selector_test.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_commands.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "components/web_modal/web_contents_modal_dialog_manager.h"
#include "content/public/browser/client_certificate_delegate.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/test_utils.h"
#include "net/cert/x509_certificate.h"
#include "net/ssl/client_cert_identity_mac.h"
#include "net/ssl/ssl_private_key_test_util.h"
#include "net/test/cert_test_util.h"
#include "net/test/keychain_test_util_mac.h"
#include "net/test/test_data_directory.h"
#import "testing/gtest_mac.h"
#include "ui/base/cocoa/window_size_constants.h"

using web_modal::WebContentsModalDialogManager;

@interface SFChooseIdentityPanel (SystemPrivate)
// A system-private interface that dismisses a panel whose sheet was started by
// -beginSheetForWindow:modalDelegate:didEndSelector:contextInfo:identities:message:
// as though the user clicked the button identified by returnCode. Verified
// present in 10.5 through 10.12.
- (void)_dismissWithCode:(NSInteger)code;
@end

namespace {

struct TestClientCertificateDelegateResults {
  bool destroyed = false;
  bool continue_with_certificate_called = false;
  scoped_refptr<net::X509Certificate> cert;
  scoped_refptr<net::SSLPrivateKey> key;
};

class TestClientCertificateDelegate
    : public content::ClientCertificateDelegate {
 public:
  // Creates a ClientCertificateDelegate that sets |*destroyed| to true on
  // destruction.
  explicit TestClientCertificateDelegate(
      TestClientCertificateDelegateResults* results)
      : results_(results) {}

  ~TestClientCertificateDelegate() override { results_->destroyed = true; }

  // content::ClientCertificateDelegate.
  void ContinueWithCertificate(scoped_refptr<net::X509Certificate> cert,
                               scoped_refptr<net::SSLPrivateKey> key) override {
    EXPECT_FALSE(results_->continue_with_certificate_called);
    results_->cert = cert;
    results_->key = key;
    results_->continue_with_certificate_called = true;
    // TODO(mattm): Add a test of selecting the 2nd certificate (if possible).
  }

 private:
  TestClientCertificateDelegateResults* results_;

  DISALLOW_COPY_AND_ASSIGN(TestClientCertificateDelegate);
};

}  // namespace

class SSLClientCertificateSelectorCocoaTest
    : public SSLClientCertificateSelectorTestBase {
 public:
  ~SSLClientCertificateSelectorCocoaTest() override;

  // InProcessBrowserTest:
  void SetUpInProcessBrowserTestFixture() override;

  net::ClientCertIdentityList GetTestCertificateList();

 protected:
  scoped_refptr<net::X509Certificate> client_cert1_;
  scoped_refptr<net::X509Certificate> client_cert2_;
  std::string pkcs8_key1_;
  std::string pkcs8_key2_;
  net::ScopedTestKeychain scoped_keychain_;
  base::ScopedCFTypeRef<SecIdentityRef> sec_identity1_;
  base::ScopedCFTypeRef<SecIdentityRef> sec_identity2_;
};

SSLClientCertificateSelectorCocoaTest::
    ~SSLClientCertificateSelectorCocoaTest() = default;

void SSLClientCertificateSelectorCocoaTest::SetUpInProcessBrowserTestFixture() {
  SSLClientCertificateSelectorTestBase::SetUpInProcessBrowserTestFixture();

  base::FilePath certs_dir = net::GetTestCertsDirectory();

  client_cert1_ = net::ImportCertFromFile(certs_dir, "client_1.pem");
  ASSERT_TRUE(client_cert1_);
  client_cert2_ = net::ImportCertFromFile(certs_dir, "client_2.pem");
  ASSERT_TRUE(client_cert2_);

  ASSERT_TRUE(base::ReadFileToString(certs_dir.AppendASCII("client_1.pk8"),
                                     &pkcs8_key1_));
  ASSERT_TRUE(base::ReadFileToString(certs_dir.AppendASCII("client_2.pk8"),
                                     &pkcs8_key2_));

  ASSERT_TRUE(scoped_keychain_.Initialize());

  sec_identity1_ = net::ImportCertAndKeyToKeychain(
      client_cert1_.get(), pkcs8_key1_, scoped_keychain_.keychain());
  ASSERT_TRUE(sec_identity1_);
  sec_identity2_ = net::ImportCertAndKeyToKeychain(
      client_cert2_.get(), pkcs8_key2_, scoped_keychain_.keychain());
  ASSERT_TRUE(sec_identity2_);
}

net::ClientCertIdentityList
SSLClientCertificateSelectorCocoaTest::GetTestCertificateList() {
  net::ClientCertIdentityList client_cert_list;
  client_cert_list.push_back(base::MakeUnique<net::ClientCertIdentityMac>(
      client_cert1_, base::ScopedCFTypeRef<SecIdentityRef>(sec_identity1_)));
  client_cert_list.push_back(base::MakeUnique<net::ClientCertIdentityMac>(
      client_cert2_, base::ScopedCFTypeRef<SecIdentityRef>(sec_identity2_)));
  return client_cert_list;
}

IN_PROC_BROWSER_TEST_F(SSLClientCertificateSelectorCocoaTest, Basic) {
  content::WebContents* web_contents =
      browser()->tab_strip_model()->GetActiveWebContents();
  WebContentsModalDialogManager* web_contents_modal_dialog_manager =
      WebContentsModalDialogManager::FromWebContents(web_contents);
  EXPECT_FALSE(web_contents_modal_dialog_manager->IsDialogActive());

  TestClientCertificateDelegateResults results;
  SSLClientCertificateSelectorCocoa* selector = [
      [SSLClientCertificateSelectorCocoa alloc]
      initWithBrowserContext:web_contents->GetBrowserContext()
             certRequestInfo:auth_requestor_->cert_request_info_.get()
                    delegate:base::WrapUnique(
                                 new TestClientCertificateDelegate(&results))];
  [selector displayForWebContents:web_contents
                      clientCerts:GetTestCertificateList()];
  content::RunAllPendingInMessageLoop();
  EXPECT_TRUE([selector panel]);
  EXPECT_TRUE(web_contents_modal_dialog_manager->IsDialogActive());

  WebContentsModalDialogManager::TestApi test_api(
      web_contents_modal_dialog_manager);
  test_api.CloseAllDialogs();
  content::RunAllPendingInMessageLoop();
  EXPECT_FALSE(web_contents_modal_dialog_manager->IsDialogActive());

  EXPECT_TRUE(results.destroyed);
  EXPECT_FALSE(results.continue_with_certificate_called);
}

IN_PROC_BROWSER_TEST_F(SSLClientCertificateSelectorCocoaTest, Cancel) {
  content::WebContents* web_contents =
      browser()->tab_strip_model()->GetActiveWebContents();
  WebContentsModalDialogManager* web_contents_modal_dialog_manager =
      WebContentsModalDialogManager::FromWebContents(web_contents);
  EXPECT_FALSE(web_contents_modal_dialog_manager->IsDialogActive());

  TestClientCertificateDelegateResults results;
  SSLClientCertificateSelectorCocoa* selector = [
      [SSLClientCertificateSelectorCocoa alloc]
      initWithBrowserContext:web_contents->GetBrowserContext()
             certRequestInfo:auth_requestor_->cert_request_info_.get()
                    delegate:base::WrapUnique(
                                 new TestClientCertificateDelegate(&results))];
  [selector displayForWebContents:web_contents
                      clientCerts:GetTestCertificateList()];
  content::RunAllPendingInMessageLoop();
  EXPECT_TRUE([selector panel]);
  EXPECT_TRUE(web_contents_modal_dialog_manager->IsDialogActive());

  // Cancel the selector. Dunno if there is a better way to do this.
  [[selector panel] _dismissWithCode:NSFileHandlingPanelCancelButton];
  content::RunAllPendingInMessageLoop();
  EXPECT_FALSE(web_contents_modal_dialog_manager->IsDialogActive());

  // ContinueWithCertificate(nullptr, nullptr) should have been called.
  EXPECT_TRUE(results.destroyed);
  EXPECT_TRUE(results.continue_with_certificate_called);
  EXPECT_FALSE(results.cert);
  EXPECT_FALSE(results.key);
}

IN_PROC_BROWSER_TEST_F(SSLClientCertificateSelectorCocoaTest, Accept) {
  content::WebContents* web_contents =
      browser()->tab_strip_model()->GetActiveWebContents();
  WebContentsModalDialogManager* web_contents_modal_dialog_manager =
      WebContentsModalDialogManager::FromWebContents(web_contents);
  EXPECT_FALSE(web_contents_modal_dialog_manager->IsDialogActive());

  TestClientCertificateDelegateResults results;
  SSLClientCertificateSelectorCocoa* selector = [
      [SSLClientCertificateSelectorCocoa alloc]
      initWithBrowserContext:web_contents->GetBrowserContext()
             certRequestInfo:auth_requestor_->cert_request_info_.get()
                    delegate:base::WrapUnique(
                                 new TestClientCertificateDelegate(&results))];
  [selector displayForWebContents:web_contents
                      clientCerts:GetTestCertificateList()];
  content::RunAllPendingInMessageLoop();
  EXPECT_TRUE([selector panel]);
  EXPECT_TRUE(web_contents_modal_dialog_manager->IsDialogActive());

  // Accept the selection. Dunno if there is a better way to do this.
  [[selector panel] _dismissWithCode:NSFileHandlingPanelOKButton];
  content::RunAllPendingInMessageLoop();
  EXPECT_FALSE(web_contents_modal_dialog_manager->IsDialogActive());

  // The first cert in the list should have been selected.
  EXPECT_TRUE(results.destroyed);
  EXPECT_TRUE(results.continue_with_certificate_called);
  EXPECT_EQ(client_cert1_, results.cert);
  ASSERT_TRUE(results.key);

  // All Mac keys are expected to have the same hash preferences.
  std::vector<net::SSLPrivateKey::Hash> expected_hashes = {
      net::SSLPrivateKey::Hash::SHA512, net::SSLPrivateKey::Hash::SHA384,
      net::SSLPrivateKey::Hash::SHA256, net::SSLPrivateKey::Hash::SHA1,
  };
  EXPECT_EQ(expected_hashes, results.key->GetDigestPreferences());

  TestSSLPrivateKeyMatches(results.key.get(), pkcs8_key1_);
}

// Test that switching to another tab correctly hides the sheet.
IN_PROC_BROWSER_TEST_F(SSLClientCertificateSelectorCocoaTest, HideShow) {
  content::WebContents* web_contents =
      browser()->tab_strip_model()->GetActiveWebContents();
  TestClientCertificateDelegateResults results;
  SSLClientCertificateSelectorCocoa* selector = [
      [SSLClientCertificateSelectorCocoa alloc]
      initWithBrowserContext:web_contents->GetBrowserContext()
             certRequestInfo:auth_requestor_->cert_request_info_.get()
                    delegate:base::WrapUnique(
                                 new TestClientCertificateDelegate(&results))];
  [selector displayForWebContents:web_contents
                      clientCerts:GetTestCertificateList()];
  content::RunAllPendingInMessageLoop();

  NSWindow* sheetWindow = [[selector overlayWindow] attachedSheet];
  EXPECT_EQ(1.0, [sheetWindow alphaValue]);

  // Switch to another tab and verify that the sheet is hidden. Interaction with
  // the tab underneath should not be blocked.
  AddBlankTabAndShow(browser());
  EXPECT_EQ(0.0, [sheetWindow alphaValue]);
  EXPECT_TRUE([[selector overlayWindow] ignoresMouseEvents]);

  // Switch back and verify that the sheet is shown. Interaction with the tab
  // underneath should be blocked while the sheet is showing.
  chrome::SelectNumberedTab(browser(), 0);
  EXPECT_EQ(1.0, [sheetWindow alphaValue]);
  EXPECT_FALSE([[selector overlayWindow] ignoresMouseEvents]);

  EXPECT_FALSE(results.destroyed);
  EXPECT_FALSE(results.continue_with_certificate_called);

  // Close the tab. Delegate should be destroyed without continuing.
  chrome::CloseTab(browser());
  EXPECT_TRUE(results.destroyed);
  EXPECT_FALSE(results.continue_with_certificate_called);
}

@interface DeallocTrackingSSLClientCertificateSelectorCocoa
    : SSLClientCertificateSelectorCocoa
@property(nonatomic) BOOL* wasDeallocated;
@end

@implementation DeallocTrackingSSLClientCertificateSelectorCocoa
@synthesize wasDeallocated = wasDeallocated_;

- (void)dealloc {
  *wasDeallocated_ = true;
  [super dealloc];
}

@end

// Test that we can't trigger the crash from https://crbug.com/653093
IN_PROC_BROWSER_TEST_F(SSLClientCertificateSelectorCocoaTest,
                       WorkaroundCrashySierra) {
  BOOL selector_was_deallocated = false;

  @autoreleasepool {
    content::WebContents* web_contents =
        browser()->tab_strip_model()->GetActiveWebContents();
    DeallocTrackingSSLClientCertificateSelectorCocoa* selector =
        [[DeallocTrackingSSLClientCertificateSelectorCocoa alloc]
            initWithBrowserContext:web_contents->GetBrowserContext()
                   certRequestInfo:auth_requestor_->cert_request_info_.get()
                          delegate:nil];
    [selector displayForWebContents:web_contents
                        clientCerts:GetTestCertificateList()];
    content::RunAllPendingInMessageLoop();

    selector.wasDeallocated = &selector_was_deallocated;

    [selector.overlayWindow endSheet:selector.overlayWindow.attachedSheet];
    content::RunAllPendingInMessageLoop();
  }

  EXPECT_TRUE(selector_was_deallocated);

  // Without the workaround, this will crash on Sierra.
  [[NSNotificationCenter defaultCenter]
      postNotificationName:NSPreferredScrollerStyleDidChangeNotification
                    object:nil
                  userInfo:@{
                    @"NSScrollerStyle" : @(NSScrollerStyleLegacy)
                  }];
  [[NSNotificationCenter defaultCenter]
      postNotificationName:NSPreferredScrollerStyleDidChangeNotification
                    object:nil
                  userInfo:@{
                    @"NSScrollerStyle" : @(NSScrollerStyleOverlay)
                  }];
}
