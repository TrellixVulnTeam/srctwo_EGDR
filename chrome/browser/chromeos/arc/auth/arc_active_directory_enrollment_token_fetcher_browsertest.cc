// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <string>

#include "base/bind.h"
#include "base/callback.h"
#include "base/command_line.h"
#include "base/memory/ptr_util.h"
#include "base/run_loop.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/chromeos/arc/arc_support_host.h"
#include "chrome/browser/chromeos/arc/auth/arc_active_directory_enrollment_token_fetcher.h"
#include "chrome/browser/chromeos/arc/extensions/fake_arc_support.h"
#include "chrome/browser/chromeos/policy/browser_policy_connector_chromeos.h"
#include "chrome/browser/chromeos/policy/dm_token_storage.h"
#include "chrome/browser/chromeos/profiles/profile_helper.h"
#include "chrome/browser/policy/cloud/test_request_interceptor.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chromeos/dbus/dbus_thread_manager.h"
#include "chromeos/dbus/fake_cryptohome_client.h"
#include "components/arc/arc_util.h"
#include "components/policy/core/common/cloud/device_management_service.h"
#include "components/policy/core/common/policy_switches.h"
#include "components/user_manager/user_manager.h"
#include "content/public/browser/browser_thread.h"
#include "net/base/network_delegate.h"
#include "net/base/upload_bytes_element_reader.h"
#include "net/base/upload_data_stream.h"
#include "net/base/url_util.h"
#include "net/http/http_request_headers.h"
#include "net/url_request/url_request.h"
#include "net/url_request/url_request_test_job.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace em = enterprise_management;

namespace arc {
namespace {

constexpr char kFakeDmToken[] = "fake-dm-token";
constexpr char kFakeEnrollmentToken[] = "fake-enrollment-token";
constexpr char kFakeUserId[] = "fake-user-id";
constexpr char kFakeAuthSessionId[] = "fake-auth-session-id";
constexpr char kFakeAdfsServerUrl[] = "http://example.com/adfs/ls/awesome.aspx";
constexpr char kNotYetFetched[] = "NOT-YET-FETCHED";
constexpr char kBadRequestHeader[] = "HTTP/1.1 400 Bad Request";

using Status = ArcActiveDirectoryEnrollmentTokenFetcher::Status;

std::string GetDmServerUrl() {
  policy ::BrowserPolicyConnectorChromeOS* const connector =
      g_browser_process->platform_part()->browser_policy_connector_chromeos();
  return connector->device_management_service()->GetServerUrl();
}

// Observer for FakeArcSupport.
class FakeArcSupportObserverBase : public FakeArcSupport::Observer {
 public:
  explicit FakeArcSupportObserverBase(FakeArcSupport* fake_arc_support)
      : fake_arc_support_(fake_arc_support) {}

  // Called when the Active Directory auth page is shown.
  virtual void OnAuthPageShown() = 0;

  void OnPageChanged(ArcSupportHost::UIPage page) override {
    if (page != ArcSupportHost::UIPage::ACTIVE_DIRECTORY_AUTH)
      return;
    EXPECT_EQ(kFakeAdfsServerUrl,
              fake_arc_support_->active_directory_auth_federation_url());
    EXPECT_EQ(GetDmServerUrl(),
              fake_arc_support_
                  ->active_directory_auth_device_management_url_prefix());
    OnAuthPageShown();
  }

 protected:
  ~FakeArcSupportObserverBase() override = default;
  FakeArcSupport* const fake_arc_support_;  // Not owned.
};

// Simulates SAML authentication success.
class SimulateAuthSucceedsObserver : public FakeArcSupportObserverBase {
 public:
  explicit SimulateAuthSucceedsObserver(FakeArcSupport* fake_arc_support)
      : FakeArcSupportObserverBase(fake_arc_support) {}

  void OnAuthPageShown() override {
    fake_arc_support_->EmulateAuthSuccess("" /* auth_code unused */);
  }
};

// Simulates pressing the Cancel button or closing the window.
class SimulateAuthCancelledObserver : public FakeArcSupportObserverBase {
 public:
  SimulateAuthCancelledObserver(FakeArcSupport* fake_arc_support,
                                base::RunLoop* run_loop)
      : FakeArcSupportObserverBase(fake_arc_support), run_loop_(run_loop) {}

  void OnAuthPageShown() override {
    fake_arc_support_->Close();

    // Since ArcActiveDirectoryEnrollmentTokenFetcher won't call the
    // FetchCallback in case of an error, we break the runloop manually.
    run_loop_->Quit();
  }

 private:
  base::RunLoop* const run_loop_;  // Not owned.
};

// Simulates SAML authentication failure.
class SimulateAuthFailsObserver : public FakeArcSupportObserverBase {
 public:
  SimulateAuthFailsObserver(FakeArcSupport* fake_arc_support,
                            base::RunLoop* run_loop)
      : FakeArcSupportObserverBase(fake_arc_support), run_loop_(run_loop) {}

  void OnAuthPageShown() override {
    fake_arc_support_->EmulateAuthFailure("error");

    // Since ArcActiveDirectoryEnrollmentTokenFetcher won't call the
    // FetchCallback in case of an error, we break the runloop manually.
    run_loop_->Quit();
  }

 private:
  base::RunLoop* const run_loop_;  // Not owned.
};

// Simulates SAML authentication retry.
class SimulateAuthRetryObserver : public FakeArcSupportObserverBase {
 public:
  SimulateAuthRetryObserver(FakeArcSupport* fake_arc_support,
                            base::RunLoop* run_loop)
      : FakeArcSupportObserverBase(fake_arc_support), run_loop_(run_loop) {}

  void OnAuthPageShown() override {
    saml_auth_count_++;

    if (saml_auth_count_ == 1) {
      // First saml auth attempt, trigger error and retry.
      fake_arc_support_->EmulateAuthFailure("error");
      EXPECT_EQ(ArcSupportHost::UIPage::ERROR, fake_arc_support_->ui_page());
      fake_arc_support_->ClickRetryButton();
    } else if (saml_auth_count_ == 2) {
      // Second saml auth attempt, trigger success.
      fake_arc_support_->EmulateAuthSuccess("" /* auth_code unused */);
    } else {
      ADD_FAILURE() << "Auth page should only be shown twice";
      run_loop_->Quit();
    }
  }

 private:
  base::RunLoop* const run_loop_;  // Not owned.
  int saml_auth_count_ = 0;
};

}  // namespace

// Checks whether |request| is a valid request to enroll a play user and returns
// the corresponding protobuf.
em::ActiveDirectoryEnrollPlayUserRequest CheckRequestAndGetEnrollRequest(
    net::URLRequest* request) {
  // Check the operation.
  std::string request_type;
  EXPECT_TRUE(
      net::GetValueForKeyInQuery(request->url(), "request", &request_type));
  EXPECT_EQ("active_directory_enroll_play_user", request_type);

  // Check content of request.
  const net::UploadDataStream* upload = request->get_upload();
  EXPECT_TRUE(upload);
  EXPECT_TRUE(upload->GetElementReaders());
  EXPECT_EQ(1u, upload->GetElementReaders()->size());
  EXPECT_TRUE((*upload->GetElementReaders())[0]->AsBytesReader());

  const net::UploadBytesElementReader* bytes_reader =
      (*upload->GetElementReaders())[0]->AsBytesReader();

  // Check the DMToken.
  net::HttpRequestHeaders request_headers = request->extra_request_headers();
  std::string value;
  EXPECT_TRUE(request_headers.GetHeader("Authorization", &value));
  EXPECT_EQ("GoogleDMToken token=" + std::string(kFakeDmToken), value);

  // Extract the actual request proto.
  em::DeviceManagementRequest parsed_request;
  EXPECT_TRUE(parsed_request.ParseFromArray(bytes_reader->bytes(),
                                            bytes_reader->length()));
  EXPECT_TRUE(parsed_request.has_active_directory_enroll_play_user_request());

  return parsed_request.active_directory_enroll_play_user_request();
}

net::URLRequestJob* SendResponse(net::URLRequest* request,
                                 net::NetworkDelegate* network_delegate,
                                 const em::DeviceManagementResponse& response) {
  std::string response_data;
  EXPECT_TRUE(response.SerializeToString(&response_data));
  return new net::URLRequestTestJob(request, network_delegate,
                                    net::URLRequestTestJob::test_headers(),
                                    response_data, true);
}

// If this gets called, the test will fail.
net::URLRequestJob* NotReachedResponseJob(
    net::URLRequest* request,
    net::NetworkDelegate* network_delegate) {
  ADD_FAILURE() << "DMServer called when not expected";
  return nullptr;
}

// JobCallback for the interceptor to test non-SAML flow.
net::URLRequestJob* NonSamlResponseJob(net::URLRequest* request,
                                       net::NetworkDelegate* network_delegate) {
  CheckRequestAndGetEnrollRequest(request);

  // Response contains the enrollment token and user id.
  em::DeviceManagementResponse response;
  em::ActiveDirectoryEnrollPlayUserResponse* enroll_response =
      response.mutable_active_directory_enroll_play_user_response();
  enroll_response->set_enrollment_token(kFakeEnrollmentToken);
  enroll_response->set_user_id(kFakeUserId);

  return SendResponse(request, network_delegate, response);
}

// JobCallback for the interceptor to start the SAML flow.
net::URLRequestJob* InitiateSamlResponseJob(
    net::URLRequest* request,
    net::NetworkDelegate* network_delegate) {
  em::ActiveDirectoryEnrollPlayUserRequest enroll_request =
      CheckRequestAndGetEnrollRequest(request);

  EXPECT_FALSE(enroll_request.has_auth_session_id());

  // Response contains only SAML parameters to initialize the SAML flow.
  em::DeviceManagementResponse response;
  em::ActiveDirectoryEnrollPlayUserResponse* enroll_response =
      response.mutable_active_directory_enroll_play_user_response();
  em::SamlParametersProto* saml_parameters =
      enroll_response->mutable_saml_parameters();
  saml_parameters->set_auth_session_id(kFakeAuthSessionId);
  saml_parameters->set_auth_redirect_url(kFakeAdfsServerUrl);

  return SendResponse(request, network_delegate, response);
}

// JobCallback returns a 400 Bad Request.
net::URLRequestJob* BadRequestResponseJob(
    net::URLRequest* request,
    net::NetworkDelegate* network_delegate) {
  return new net::URLRequestTestJob(request, network_delegate,
                                    kBadRequestHeader, std::string(), true);
}

// JobCallback for the interceptor to end the SAML flow.
net::URLRequestJob* FinishSamlResponseJob(
    net::URLRequest* request,
    net::NetworkDelegate* network_delegate) {
  DCHECK(request);
  em::ActiveDirectoryEnrollPlayUserRequest enroll_request =
      CheckRequestAndGetEnrollRequest(request);

  EXPECT_TRUE(enroll_request.has_auth_session_id());
  EXPECT_EQ(kFakeAuthSessionId, enroll_request.auth_session_id());

  // Response contains the enrollment token and user id.
  em::DeviceManagementResponse response;
  em::ActiveDirectoryEnrollPlayUserResponse* enroll_response =
      response.mutable_active_directory_enroll_play_user_response();
  enroll_response->set_enrollment_token(kFakeEnrollmentToken);
  enroll_response->set_user_id(kFakeUserId);

  return SendResponse(request, network_delegate, response);
}

class ArcActiveDirectoryEnrollmentTokenFetcherBrowserTest
    : public InProcessBrowserTest,
      public ArcSupportHost::ErrorDelegate {
 protected:
  ArcActiveDirectoryEnrollmentTokenFetcherBrowserTest() = default;
  ~ArcActiveDirectoryEnrollmentTokenFetcherBrowserTest() override = default;

  void SetUpCommandLine(base::CommandLine* command_line) override {
    command_line->AppendSwitchASCII(policy::switches::kDeviceManagementUrl,
                                    "http://localhost");
    SetArcAvailableCommandLineForTesting(command_line);
  }

  void SetUpInProcessBrowserTestFixture() override {
    // Set fake cryptohome, because we want to fail DMToken retrieval
    auto cryptohome_client = base::MakeUnique<chromeos::FakeCryptohomeClient>();
    fake_cryptohome_client_ = cryptohome_client.get();
    chromeos::DBusThreadManager::GetSetterForTesting()->SetCryptohomeClient(
        std::move(cryptohome_client));
  }

  void SetUpOnMainThread() override {
    interceptor_ = base::MakeUnique<policy::TestRequestInterceptor>(
        "localhost", content::BrowserThread::GetTaskRunnerForThread(
                         content::BrowserThread::IO));

    support_host_ = base::MakeUnique<ArcSupportHost>(browser()->profile());
    support_host_->SetErrorDelegate(this);
    fake_arc_support_ = base::MakeUnique<FakeArcSupport>(support_host_.get());
    token_fetcher_ = base::MakeUnique<ArcActiveDirectoryEnrollmentTokenFetcher>(
        support_host_.get());
  }

  void TearDownOnMainThread() override {
    token_fetcher_.reset();
    fake_arc_support_.reset();
    support_host_->SetErrorDelegate(nullptr);
    support_host_.reset();
    interceptor_.reset();
  }

  // Stores a correct (fake) DM token.
  // ArcActiveDirectoryEnrollmentTokenFetcher will succeed to fetch the DM
  // token.
  void StoreCorrectDmToken() {
    fake_cryptohome_client_->set_system_salt(
        chromeos::FakeCryptohomeClient::GetStubSystemSalt());
    fake_cryptohome_client_->SetServiceIsAvailable(true);
    // Store a fake DM token.
    base::RunLoop run_loop;
    auto dm_token_storage = base::MakeUnique<policy::DMTokenStorage>(
        g_browser_process->local_state());
    dm_token_storage->StoreDMToken(
        kFakeDmToken, base::BindOnce(
                          [](base::RunLoop* run_loop, bool success) {
                            EXPECT_TRUE(success);
                            run_loop->Quit();
                          },
                          &run_loop));
    // Because the StoreDMToken() operation interacts with the I/O thread,
    // RunUntilIdle() won't work here. Instead, use Run() and Quit()
    // explicitly in the callback.
    run_loop.Run();
  }

  // Does not store a correct DM token.
  // ArcActiveDirectoryEnrollmentTokenFetcher will fail to fetch the DM token.
  void FailDmToken() {
    fake_cryptohome_client_->set_system_salt(std::vector<uint8_t>());
    fake_cryptohome_client_->SetServiceIsAvailable(true);
  }

  void FetchEnrollmentToken(base::RunLoop* run_loop,
                            Status* out_fetch_status,
                            std::string* out_enrollment_token,
                            std::string* out_user_id) {
    token_fetcher_->Fetch(base::Bind(
        [](base::RunLoop* run_loop, Status* out_fetch_status,
           std::string* out_enrollment_token, std::string* out_user_id,
           Status fetch_status, const std::string& enrollment_token,
           const std::string& user_id) {
          *out_fetch_status = fetch_status;
          *out_enrollment_token = enrollment_token;
          *out_user_id = user_id;
          run_loop->Quit();
        },
        run_loop, out_fetch_status, out_enrollment_token, out_user_id));

    // Because the Fetch() operation needs to interact with other threads,
    // RunUntilIdle() won't work here. Instead, use Run() and Quit()
    // explicitly in the callback.
    run_loop->Run();
  }

  void FetchEnrollmentTokenAndExpectCallbackNotReached(
      base::RunLoop* run_loop) {
    token_fetcher_->Fetch(base::BindOnce(
        [](Status fetch_status, const std::string& enrollment_token,
           const std::string& user_id) { FAIL() << "Should not be called"; }));
    run_loop->Run();
  }

  void ExpectEnrollmentTokenFetchSucceeds(base::RunLoop* run_loop) {
    Status fetch_status = Status::FAILURE;
    std::string enrollment_token;
    std::string user_id;

    FetchEnrollmentToken(run_loop, &fetch_status, &enrollment_token, &user_id);

    // Verify expectations.
    EXPECT_EQ(Status::SUCCESS, fetch_status);
    EXPECT_EQ(kFakeEnrollmentToken, enrollment_token);
    EXPECT_EQ(kFakeUserId, user_id);
  }

  void ExpectEnrollmentTokenFetchFails(base::RunLoop* run_loop,
                                       Status expected_status) {
    EXPECT_NE(expected_status, Status::SUCCESS);
    Status fetch_status = Status::SUCCESS;
    // The expected strings are empty, o initialize with non-empty value.
    std::string enrollment_token = kNotYetFetched;
    std::string user_id = kNotYetFetched;

    FetchEnrollmentToken(run_loop, &fetch_status, &enrollment_token, &user_id);

    // Verify expectations.
    EXPECT_EQ(expected_status, fetch_status);
    EXPECT_TRUE(enrollment_token.empty());
    EXPECT_TRUE(user_id.empty());
  }

  std::unique_ptr<policy::TestRequestInterceptor> interceptor_;
  std::unique_ptr<FakeArcSupport> fake_arc_support_;
  std::unique_ptr<ArcActiveDirectoryEnrollmentTokenFetcher> token_fetcher_;

 private:
  ArcSupportHost::AuthDelegate* GetAuthDelegate() {
    return static_cast<ArcSupportHost::AuthDelegate*>(token_fetcher_.get());
  }

  // ArcSupportHost:::ErrorDelegate:
  // Settings these prevents some DCHECK failures.
  void OnWindowClosed() override {}
  void OnRetryClicked() override {}
  void OnSendFeedbackClicked() override {}

  std::unique_ptr<ArcSupportHost> support_host_;
  // DBusThreadManager owns this.
  chromeos::FakeCryptohomeClient* fake_cryptohome_client_;

  DISALLOW_COPY_AND_ASSIGN(ArcActiveDirectoryEnrollmentTokenFetcherBrowserTest);
};

// Non-SAML flow fetches valid enrollment token and user id.
IN_PROC_BROWSER_TEST_F(ArcActiveDirectoryEnrollmentTokenFetcherBrowserTest,
                       RequestAccountInfoSuccess) {
  StoreCorrectDmToken();
  interceptor_->PushJobCallback(base::Bind(&NonSamlResponseJob));
  base::RunLoop run_loop;
  ExpectEnrollmentTokenFetchSucceeds(&run_loop);
}

// Failure to fetch DM token leads to token fetch failure.
IN_PROC_BROWSER_TEST_F(ArcActiveDirectoryEnrollmentTokenFetcherBrowserTest,
                       DmTokenRetrievalFailed) {
  FailDmToken();
  interceptor_->PushJobCallback(base::Bind(NotReachedResponseJob));
  base::RunLoop run_loop;
  ExpectEnrollmentTokenFetchFails(&run_loop, Status::FAILURE);
}

// Server responds with bad request and fails token fetch.
IN_PROC_BROWSER_TEST_F(ArcActiveDirectoryEnrollmentTokenFetcherBrowserTest,
                       RequestAccountInfoError) {
  StoreCorrectDmToken();
  interceptor_->PushJobCallback(
      policy::TestRequestInterceptor::BadRequestJob());
  base::RunLoop run_loop;
  ExpectEnrollmentTokenFetchFails(&run_loop, Status::FAILURE);
}

// ARC disabled leads to failed token fetch.
IN_PROC_BROWSER_TEST_F(ArcActiveDirectoryEnrollmentTokenFetcherBrowserTest,
                       ArcDisabled) {
  StoreCorrectDmToken();
  interceptor_->PushJobCallback(
      policy::TestRequestInterceptor::HttpErrorJob("904 ARC Disabled"));
  base::RunLoop run_loop;
  ExpectEnrollmentTokenFetchFails(&run_loop, Status::ARC_DISABLED);
}

// Successful enrollment token fetch including SAML authentication.
// SAML flow works as follows:
//   1) |token_fetcher_| sends a request to device management (DM) server to
//      fetch the token.
//   2) DM server responds with an auth session id and a redirect URL to the
//      ADFS server. This is emulated in InitiateSamlResponseJob.
//   3) |token_fetcher_| sets the redirect URL in |fake_arc_support_| and
//      opens the Active Directory auth page, which triggers
//      SimulateAuthSucceedsObserver's OnAuthPageShown() handler.
//   4) SimulateAuthSucceedsObserver triggers the onAuthSucceeded event, which
//      causes |arc_support_host_| to call into |token_fetcher_|'s
//      OnAuthSucceeded.
//   5) |token_fetcher_| sends another request to DM server to fetch the token,
//      this time with the auth session id from 2).
//   6) DM server responds with the token. This is emulated in
//      FinishSamlResponseJob.
IN_PROC_BROWSER_TEST_F(ArcActiveDirectoryEnrollmentTokenFetcherBrowserTest,
                       SamlFlowSuccess) {
  StoreCorrectDmToken();
  interceptor_->PushJobCallback(base::Bind(&InitiateSamlResponseJob));
  interceptor_->PushJobCallback(base::Bind(&FinishSamlResponseJob));
  base::RunLoop run_loop;
  SimulateAuthSucceedsObserver observer(fake_arc_support_.get());
  fake_arc_support_->AddObserver(&observer);
  ExpectEnrollmentTokenFetchSucceeds(&run_loop);
  fake_arc_support_->RemoveObserver(&observer);
}

// SAML flow fails since the user closed the window or clicked the Cancel
// button.
IN_PROC_BROWSER_TEST_F(ArcActiveDirectoryEnrollmentTokenFetcherBrowserTest,
                       SamlFlowFailsUserCancelled) {
  StoreCorrectDmToken();
  interceptor_->PushJobCallback(base::Bind(&InitiateSamlResponseJob));
  base::RunLoop run_loop;
  SimulateAuthCancelledObserver observer(fake_arc_support_.get(), &run_loop);
  fake_arc_support_->AddObserver(&observer);

  // On user cancel, the FetchCallback should not be called (in fact, the
  // window closes silently).
  ASSERT_NO_FATAL_FAILURE(
      FetchEnrollmentTokenAndExpectCallbackNotReached(&run_loop));
  fake_arc_support_->RemoveObserver(&observer);
}

// SAML flow fails because of an error.
IN_PROC_BROWSER_TEST_F(ArcActiveDirectoryEnrollmentTokenFetcherBrowserTest,
                       SamlFlowFailsError) {
  StoreCorrectDmToken();
  interceptor_->PushJobCallback(base::Bind(&InitiateSamlResponseJob));
  base::RunLoop run_loop;
  SimulateAuthFailsObserver observer(fake_arc_support_.get(), &run_loop);
  fake_arc_support_->AddObserver(&observer);

  // Similar to user cancel, the callback to Fetch should never be called.
  // Instead, we should end up on the error page.
  ASSERT_NO_FATAL_FAILURE(
      FetchEnrollmentTokenAndExpectCallbackNotReached(&run_loop));
  EXPECT_EQ(ArcSupportHost::UIPage::ERROR, fake_arc_support_->ui_page());
  fake_arc_support_->RemoveObserver(&observer);
}

// SAML flow fails first during initial DM server request, but the retry
// works.
IN_PROC_BROWSER_TEST_F(ArcActiveDirectoryEnrollmentTokenFetcherBrowserTest,
                       SamlFlowSucceedsWithDmRetry) {
  StoreCorrectDmToken();
  interceptor_->PushJobCallback(base::Bind(&BadRequestResponseJob));
  base::RunLoop failure_run_loop;
  ExpectEnrollmentTokenFetchFails(&failure_run_loop, Status::FAILURE);

  interceptor_->PushJobCallback(base::Bind(&InitiateSamlResponseJob));
  interceptor_->PushJobCallback(base::Bind(&FinishSamlResponseJob));
  base::RunLoop success_run_loop;
  SimulateAuthSucceedsObserver observer(fake_arc_support_.get());
  fake_arc_support_->AddObserver(&observer);
  ExpectEnrollmentTokenFetchSucceeds(&success_run_loop);
  fake_arc_support_->RemoveObserver(&observer);
}

// SAML flow fails first during SAML auth, but the retry works.
IN_PROC_BROWSER_TEST_F(ArcActiveDirectoryEnrollmentTokenFetcherBrowserTest,
                       SamlFlowSucceedsWithAuthRetry) {
  StoreCorrectDmToken();
  interceptor_->PushJobCallback(base::Bind(&InitiateSamlResponseJob));
  interceptor_->PushJobCallback(base::Bind(&InitiateSamlResponseJob));
  interceptor_->PushJobCallback(base::Bind(&FinishSamlResponseJob));
  base::RunLoop run_loop;
  SimulateAuthRetryObserver observer(fake_arc_support_.get(), &run_loop);
  fake_arc_support_->AddObserver(&observer);
  ExpectEnrollmentTokenFetchSucceeds(&run_loop);
  EXPECT_EQ(0u, interceptor_->GetPendingSize());
  fake_arc_support_->RemoveObserver(&observer);
}

}  // namespace arc
