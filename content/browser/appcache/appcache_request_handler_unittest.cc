// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/appcache/appcache_request_handler.h"

#include <stdint.h>

#include <stack>
#include <string>
#include <utility>
#include <vector>

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/callback.h"
#include "base/location.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/memory/weak_ptr.h"
#include "base/message_loop/message_loop.h"
#include "base/single_thread_task_runner.h"
#include "base/strings/stringprintf.h"
#include "base/synchronization/waitable_event.h"
#include "base/test/scoped_feature_list.h"
#include "base/test/scoped_task_environment.h"
#include "base/threading/thread.h"
#include "base/threading/thread_task_runner_handle.h"
#include "content/browser/appcache/appcache.h"
#include "content/browser/appcache/appcache_backend_impl.h"
#include "content/browser/appcache/appcache_job.h"
#include "content/browser/appcache/appcache_url_loader_request.h"
#include "content/browser/appcache/appcache_url_request.h"
#include "content/browser/appcache/appcache_url_request_job.h"
#include "content/browser/appcache/mock_appcache_policy.h"
#include "content/browser/appcache/mock_appcache_service.h"
#include "content/public/common/content_features.h"
#include "content/public/common/resource_request.h"
#include "net/base/net_errors.h"
#include "net/base/request_priority.h"
#include "net/http/http_response_headers.h"
#include "net/http/http_util.h"
#include "net/traffic_annotation/network_traffic_annotation_test_helper.h"
#include "net/url_request/url_request.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_error_job.h"
#include "net/url_request/url_request_job_factory.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace content {

static const int kMockProcessId = 1;

// Controls whether we instantiate the URLRequest based AppCache handler or
// the URLLoader based one.
enum RequestHandlerType {
  URLREQUEST,
  URLLOADER,
};

// TODO(michaeln/ananta)
// Build on the abstractions provided by the request and the job classes to
// provide mock request and job classes to the AppCacheRequestHandler class
// which would make it testable. It would also allow us to avoid the URLRequest
// and URLLoader semantics in the test cases here,
class AppCacheRequestHandlerTest
    : public testing::TestWithParam<RequestHandlerType> {
 public:
  class MockFrontend : public AppCacheFrontend {
   public:
    void OnCacheSelected(int host_id, const AppCacheInfo& info) override {}

    void OnStatusChanged(const std::vector<int>& host_ids,
                         AppCacheStatus status) override {}

    void OnEventRaised(const std::vector<int>& host_ids,
                       AppCacheEventID event_id) override {}

    void OnErrorEventRaised(const std::vector<int>& host_ids,
                            const AppCacheErrorDetails& details) override {}

    void OnProgressEventRaised(const std::vector<int>& host_ids,
                               const GURL& url,
                               int num_total,
                               int num_complete) override {}

    void OnLogMessage(int host_id,
                      AppCacheLogLevel log_level,
                      const std::string& message) override {}

    void OnContentBlocked(int host_id, const GURL& manifest_url) override {}

    void OnSetSubresourceFactory(
        int host_id,
        mojo::MessagePipeHandle loader_factory_pipe_handle) override {}
  };

  // Helper callback to run a test on our io_thread. The io_thread is spun up
  // once and reused for all tests.
  template <class Method>
  void MethodWrapper(Method method) {
    SetUpTest();
    (this->*method)();
  }

  // Subclasses to simulate particular responses so test cases can
  // exercise fallback code paths.

  class MockURLRequestDelegate : public net::URLRequest::Delegate {
   public:
    MockURLRequestDelegate() : request_status_(1) {}

    void OnResponseStarted(net::URLRequest* request, int net_error) override {
      DCHECK_NE(net::ERR_IO_PENDING, net_error);
      request_status_ = net_error;
    }

    void OnReadCompleted(net::URLRequest* request, int bytes_read) override {
      DCHECK_NE(net::ERR_IO_PENDING, bytes_read);
      if (bytes_read >= 0)
        request_status_ = net::OK;
      else
        request_status_ = bytes_read;
    }

    int request_status() const { return request_status_; }

   private:
    int request_status_;
  };

  class MockURLRequestJob : public net::URLRequestJob {
   public:
    MockURLRequestJob(net::URLRequest* request,
                      net::NetworkDelegate* network_delegate,
                      const net::HttpResponseInfo& info)
        : net::URLRequestJob(request, network_delegate),
          has_response_info_(true),
          response_info_(info) {}

    ~MockURLRequestJob() override {}

   protected:
    void Start() override { NotifyHeadersComplete(); }
    void GetResponseInfo(net::HttpResponseInfo* info) override {
      if (!has_response_info_)
        return;
      *info = response_info_;
    }

   private:
    bool has_response_info_;
    net::HttpResponseInfo response_info_;
  };

  class MockURLRequestJobFactory : public net::URLRequestJobFactory {
   public:
    MockURLRequestJobFactory() {}

    ~MockURLRequestJobFactory() override { DCHECK(!job_); }

    void SetJob(std::unique_ptr<net::URLRequestJob> job) {
      job_ = std::move(job);
    }

    net::URLRequestJob* MaybeCreateJobWithProtocolHandler(
        const std::string& scheme,
        net::URLRequest* request,
        net::NetworkDelegate* network_delegate) const override {
      if (job_)
        return job_.release();

      // Some of these tests trigger UpdateJobs which start URLRequests.
      // We short circuit those be returning error jobs.
      return new net::URLRequestErrorJob(request, network_delegate,
                                         net::ERR_INTERNET_DISCONNECTED);
    }

    net::URLRequestJob* MaybeInterceptRedirect(
        net::URLRequest* request,
        net::NetworkDelegate* network_delegate,
        const GURL& location) const override {
      return nullptr;
    }

    net::URLRequestJob* MaybeInterceptResponse(
        net::URLRequest* request,
        net::NetworkDelegate* network_delegate) const override {
      return nullptr;
    }

    bool IsHandledProtocol(const std::string& scheme) const override {
      return scheme == "http";
    };

    bool IsSafeRedirectTarget(const GURL& location) const override {
      return false;
    }

   private:
    mutable std::unique_ptr<net::URLRequestJob> job_;
  };

  static void SetUpTestCase() {
    scoped_task_environment_.reset(new base::test::ScopedTaskEnvironment);
    io_thread_.reset(new base::Thread("AppCacheRequestHandlerTest Thread"));
    base::Thread::Options options(base::MessageLoop::TYPE_IO, 0);
    io_thread_->StartWithOptions(options);
  }

  static void TearDownTestCase() {
    io_thread_.reset();
    scoped_task_environment_.reset();
  }

  // Test harness --------------------------------------------------

  AppCacheRequestHandlerTest()
      : host_(NULL), request_(nullptr), request_handler_type_(GetParam()) {
    AppCacheRequestHandler::SetRunningInTests(true);
    if (request_handler_type_ == URLLOADER)
      feature_list_.InitAndEnableFeature(features::kNetworkService);
  }

  ~AppCacheRequestHandlerTest() {
    AppCacheRequestHandler::SetRunningInTests(false);
  }

  template <class Method>
  void RunTestOnIOThread(Method method) {
    test_finished_event_.reset(new base::WaitableEvent(
        base::WaitableEvent::ResetPolicy::AUTOMATIC,
        base::WaitableEvent::InitialState::NOT_SIGNALED));
    io_thread_->task_runner()->PostTask(
        FROM_HERE,
        base::BindOnce(&AppCacheRequestHandlerTest::MethodWrapper<Method>,
                       base::Unretained(this), method));
    test_finished_event_->Wait();
  }

  void SetUpTest() {
    DCHECK(io_thread_->task_runner()->BelongsToCurrentThread());
    mock_service_.reset(new MockAppCacheService);
    // Initializes URLRequestContext on the IO thread.
    empty_context_.reset(new net::URLRequestContext);
    mock_service_->set_request_context(empty_context_.get());
    mock_policy_.reset(new MockAppCachePolicy);
    mock_service_->set_appcache_policy(mock_policy_.get());
    mock_frontend_.reset(new MockFrontend);
    backend_impl_.reset(new AppCacheBackendImpl);
    backend_impl_->Initialize(mock_service_.get(), mock_frontend_.get(),
                              kMockProcessId);
    const int kHostId = 1;
    backend_impl_->RegisterHost(kHostId);
    host_ = backend_impl_->GetHost(kHostId);
    job_factory_.reset(new MockURLRequestJobFactory());
    empty_context_->set_job_factory(job_factory_.get());
  }

  void TearDownTest() {
    DCHECK(io_thread_->task_runner()->BelongsToCurrentThread());
    job_ = NULL;
    handler_.reset();
    request_ = nullptr;
    url_request_.reset();
    backend_impl_.reset();
    mock_frontend_.reset();
    mock_service_.reset();
    mock_policy_.reset();
    job_factory_.reset();
    empty_context_.reset();
    host_ = NULL;
  }

  void TestFinished() {
    // We unwind the stack prior to finishing up to let stack
    // based objects get deleted.
    DCHECK(io_thread_->task_runner()->BelongsToCurrentThread());
    base::ThreadTaskRunnerHandle::Get()->PostTask(
        FROM_HERE,
        base::BindOnce(&AppCacheRequestHandlerTest::TestFinishedUnwound,
                       base::Unretained(this)));
  }

  void TestFinishedUnwound() {
    TearDownTest();
    test_finished_event_->Signal();
  }

  void PushNextTask(base::OnceClosure task) {
    task_stack_.push(std::move(task));
  }

  void ScheduleNextTask() {
    DCHECK(io_thread_->task_runner()->BelongsToCurrentThread());
    if (task_stack_.empty()) {
      TestFinished();
      return;
    }
    base::ThreadTaskRunnerHandle::Get()->PostTask(FROM_HERE,
                                                  std::move(task_stack_.top()));
    task_stack_.pop();
  }

  // MainResource_Miss --------------------------------------------------

  void MainResource_Miss() {
    PushNextTask(
        base::BindOnce(&AppCacheRequestHandlerTest::Verify_MainResource_Miss,
                       base::Unretained(this)));

    EXPECT_TRUE(CreateRequestAndHandler(GURL("http://blah"), host_,
                                        RESOURCE_TYPE_MAIN_FRAME));
    EXPECT_TRUE(handler_.get());

    job_.reset(handler_->MaybeLoadResource(nullptr));
    EXPECT_TRUE(job_.get());
    EXPECT_TRUE(job_->IsWaiting());

    // We have to wait for completion of storage->FindResponseForMainRequest.
    ScheduleNextTask();
  }

  void Verify_MainResource_Miss() {
    EXPECT_FALSE(job_->IsWaiting());
    EXPECT_TRUE(job_->IsDeliveringNetworkResponse());

    int64_t cache_id = kAppCacheNoCacheId;
    GURL manifest_url;
    handler_->GetExtraResponseInfo(&cache_id, &manifest_url);
    EXPECT_EQ(kAppCacheNoCacheId, cache_id);
    EXPECT_EQ(GURL(), manifest_url);
    EXPECT_EQ(0, handler_->found_group_id_);

    std::unique_ptr<AppCacheJob> fallback_job(
        handler_->MaybeLoadFallbackForRedirect(nullptr,
                                               GURL("http://blah/redirect")));
    EXPECT_FALSE(fallback_job);
    fallback_job.reset(handler_->MaybeLoadFallbackForResponse(nullptr));
    EXPECT_FALSE(fallback_job);

    EXPECT_TRUE(host_->preferred_manifest_url().is_empty());

    TestFinished();
  }

  // MainResource_Hit --------------------------------------------------

  void MainResource_Hit() {
    PushNextTask(
        base::BindOnce(&AppCacheRequestHandlerTest::Verify_MainResource_Hit,
                       base::Unretained(this)));

    EXPECT_TRUE(CreateRequestAndHandler(GURL("http://blah"), host_,
                                        RESOURCE_TYPE_MAIN_FRAME));
    EXPECT_TRUE(handler_.get());

    mock_storage()->SimulateFindMainResource(
        AppCacheEntry(AppCacheEntry::EXPLICIT, 1),
        GURL(), AppCacheEntry(),
        1, 2, GURL("http://blah/manifest/"));

    job_.reset(handler_->MaybeLoadResource(nullptr));
    EXPECT_TRUE(job_.get());
    EXPECT_TRUE(job_->IsWaiting());

    // We have to wait for completion of storage->FindResponseForMainRequest.
    ScheduleNextTask();
  }

  void Verify_MainResource_Hit() {
    EXPECT_FALSE(job_->IsWaiting());
    EXPECT_TRUE(job_->IsDeliveringAppCacheResponse());

    int64_t cache_id = kAppCacheNoCacheId;
    GURL manifest_url;
    handler_->GetExtraResponseInfo(&cache_id, &manifest_url);
    EXPECT_EQ(1, cache_id);
    EXPECT_EQ(GURL("http://blah/manifest/"), manifest_url);
    EXPECT_EQ(2, handler_->found_group_id_);

    std::unique_ptr<AppCacheJob> fallback_job(
        handler_->MaybeLoadFallbackForResponse(nullptr));
    EXPECT_FALSE(fallback_job);

    EXPECT_EQ(GURL("http://blah/manifest/"),
              host_->preferred_manifest_url());

    TestFinished();
  }

  // MainResource_Fallback --------------------------------------------------

  void MainResource_Fallback() {
    PushNextTask(base::BindOnce(
        &AppCacheRequestHandlerTest::Verify_MainResource_Fallback,
        base::Unretained(this)));

    EXPECT_TRUE(CreateRequestAndHandler(GURL("http://blah"), host_,
                                        RESOURCE_TYPE_MAIN_FRAME));
    EXPECT_TRUE(handler_.get());

    mock_storage()->SimulateFindMainResource(
        AppCacheEntry(),
        GURL("http://blah/fallbackurl"),
        AppCacheEntry(AppCacheEntry::EXPLICIT, 1),
        1, 2, GURL("http://blah/manifest/"));

    job_.reset(handler_->MaybeLoadResource(nullptr));
    EXPECT_TRUE(job_.get());
    EXPECT_TRUE(job_->IsWaiting());

    // We have to wait for completion of storage->FindResponseForMainRequest.
    ScheduleNextTask();
  }

  void SimulateResponseCode(int response_code) {
    net::HttpResponseInfo info;
    std::string headers =
        base::StringPrintf("HTTP/1.1 %i Muffin\r\n\r\n", response_code);
    info.headers = new net::HttpResponseHeaders(
        net::HttpUtil::AssembleRawHeaders(headers.c_str(), headers.length()));

    if (request_handler_type_ == URLREQUEST) {
      job_factory_->SetJob(base::MakeUnique<MockURLRequestJob>(
          url_request_.get(), nullptr, info));
      request_->AsURLRequest()->GetURLRequest()->Start();
      // All our simulation needs to satisfy are the DCHECK's for the request
      // status and the response code.
      DCHECK_EQ(net::OK, delegate_.request_status());
    } else {
      ResourceResponseHead response;
      response.headers = info.headers;
      request_->AsURLLoaderRequest()->set_response(response);
    }
    DCHECK_EQ(response_code, request_->GetResponseCode());
  }

  void SimulateResponseInfo(const net::HttpResponseInfo& info) {
    if (request_handler_type_ == URLREQUEST) {
      job_factory_->SetJob(base::MakeUnique<MockURLRequestJob>(
          url_request_.get(), nullptr, info));
      request_->AsURLRequest()->GetURLRequest()->Start();
    } else {
      ResourceResponseHead response;
      response.headers = info.headers;
      request_->AsURLLoaderRequest()->set_response(response);
    }
  }

  void Verify_MainResource_Fallback() {
    EXPECT_FALSE(job_->IsWaiting());
    EXPECT_TRUE(job_->IsDeliveringNetworkResponse());

    // The handler expects to the job to tell it that the request is going to
    // be restarted before it sees the next request.
    handler_->OnPrepareToRestart();

    // When the request is restarted, the existing job is dropped so a
    // real network job gets created. We expect NULL here which will cause
    // the net library to create a real job.
    job_.reset(handler_->MaybeLoadResource(nullptr));
    EXPECT_FALSE(job_.get());

    // Simulate an http error of the real network job.
    SimulateResponseCode(500);

    job_.reset(handler_->MaybeLoadFallbackForResponse(nullptr));
    EXPECT_TRUE(job_.get());
    EXPECT_TRUE(job_->IsDeliveringAppCacheResponse());

    int64_t cache_id = kAppCacheNoCacheId;
    GURL manifest_url;
    handler_->GetExtraResponseInfo(&cache_id, &manifest_url);
    EXPECT_EQ(1, cache_id);
    EXPECT_EQ(GURL("http://blah/manifest/"), manifest_url);
    EXPECT_TRUE(host_->main_resource_was_namespace_entry_);
    EXPECT_EQ(GURL("http://blah/fallbackurl"), host_->namespace_entry_url_);

    EXPECT_EQ(GURL("http://blah/manifest/"),
              host_->preferred_manifest_url());

    TestFinished();
  }

  // MainResource_FallbackOverride --------------------------------------------

  void MainResource_FallbackOverride() {
    PushNextTask(base::BindOnce(
        &AppCacheRequestHandlerTest::Verify_MainResource_FallbackOverride,
        base::Unretained(this)));

    EXPECT_TRUE(CreateRequestAndHandler(GURL("http://blah/fallback-override"),
                                        host_, RESOURCE_TYPE_MAIN_FRAME));
    EXPECT_TRUE(handler_.get());

    mock_storage()->SimulateFindMainResource(
        AppCacheEntry(),
        GURL("http://blah/fallbackurl"),
        AppCacheEntry(AppCacheEntry::EXPLICIT, 1),
        1, 2, GURL("http://blah/manifest/"));

    job_.reset(handler_->MaybeLoadResource(nullptr));
    EXPECT_TRUE(job_.get());
    EXPECT_TRUE(job_->IsWaiting());

    // We have to wait for completion of storage->FindResponseForMainRequest.
    ScheduleNextTask();
  }

  void Verify_MainResource_FallbackOverride() {
    EXPECT_FALSE(job_->IsWaiting());
    EXPECT_TRUE(job_->IsDeliveringNetworkResponse());

    // The handler expects to the job to tell it that the request is going to
    // be restarted before it sees the next request.
    handler_->OnPrepareToRestart();

    // When the request is restarted, the existing job is dropped so a
    // real network job gets created. We expect NULL here which will cause
    // the net library to create a real job.
    job_.reset(handler_->MaybeLoadResource(nullptr));
    EXPECT_FALSE(job_.get());

    // Simulate an http error of the real network job, but with custom
    // headers that override the fallback behavior.
    const char kOverrideHeaders[] =
        "HTTP/1.1 404 BOO HOO\0"
        "x-chromium-appcache-fallback-override: disallow-fallback\0"
        "\0";
    net::HttpResponseInfo info;
    info.headers = new net::HttpResponseHeaders(
        std::string(kOverrideHeaders, arraysize(kOverrideHeaders)));
    SimulateResponseInfo(info);

    job_.reset(handler_->MaybeLoadFallbackForResponse(nullptr));
    EXPECT_FALSE(job_.get());

    // GetExtraResponseInfo should return no information.
    int64_t cache_id = kAppCacheNoCacheId;
    GURL manifest_url;
    handler_->GetExtraResponseInfo(&cache_id, &manifest_url);
    EXPECT_EQ(kAppCacheNoCacheId, cache_id);
    EXPECT_TRUE(manifest_url.is_empty());

    TestFinished();
  }

  // SubResource_Miss_WithNoCacheSelected ----------------------------------

  void SubResource_Miss_WithNoCacheSelected() {
    EXPECT_TRUE(CreateRequestAndHandler(GURL("http://blah/"), host_,
                                        RESOURCE_TYPE_SUB_RESOURCE));
    // We avoid creating handler when possible, sub-resource requests are not
    // subject to retrieval from an appcache when there's no associated cache.
    EXPECT_FALSE(handler_.get());

    TestFinished();
  }

  // SubResource_Miss_WithCacheSelected ----------------------------------

  void SubResource_Miss_WithCacheSelected() {
    // A sub-resource load where the resource is not in an appcache, or
    // in a network or fallback namespace, should result in a failed request.
    host_->AssociateCompleteCache(MakeNewCache());

    EXPECT_TRUE(CreateRequestAndHandler(GURL("http://blah/"), host_,
                                        RESOURCE_TYPE_SUB_RESOURCE));
    EXPECT_TRUE(handler_.get());

    job_.reset(handler_->MaybeLoadResource(nullptr));
    EXPECT_TRUE(job_.get());
    EXPECT_TRUE(job_->IsDeliveringErrorResponse());

    std::unique_ptr<AppCacheJob> fallback_job(
        handler_->MaybeLoadFallbackForRedirect(nullptr,
                                               GURL("http://blah/redirect")));
    EXPECT_FALSE(fallback_job);
    fallback_job.reset(handler_->MaybeLoadFallbackForResponse(nullptr));
    EXPECT_FALSE(fallback_job);

    TestFinished();
  }

  // SubResource_Miss_WithWaitForCacheSelection -----------------------------

  void SubResource_Miss_WithWaitForCacheSelection() {
    // Precondition, the host is waiting on cache selection.
    scoped_refptr<AppCache> cache(MakeNewCache());
    host_->pending_selected_cache_id_ = cache->cache_id();
    host_->set_preferred_manifest_url(cache->owning_group()->manifest_url());

    EXPECT_TRUE(CreateRequestAndHandler(GURL("http://blah/"), host_,
                                        RESOURCE_TYPE_SUB_RESOURCE));
    EXPECT_TRUE(handler_.get());
    job_.reset(handler_->MaybeLoadResource(nullptr));
    EXPECT_TRUE(job_.get());
    EXPECT_TRUE(job_->IsWaiting());

    host_->FinishCacheSelection(cache.get(), NULL);
    EXPECT_FALSE(job_->IsWaiting());
    EXPECT_TRUE(job_->IsDeliveringErrorResponse());

    std::unique_ptr<AppCacheJob> fallback_job(
        handler_->MaybeLoadFallbackForRedirect(nullptr,
                                               GURL("http://blah/redirect")));
    EXPECT_FALSE(fallback_job);
    fallback_job.reset(handler_->MaybeLoadFallbackForResponse(nullptr));
    EXPECT_FALSE(fallback_job);

    TestFinished();
  }

  // SubResource_Hit -----------------------------

  void SubResource_Hit() {
    host_->AssociateCompleteCache(MakeNewCache());

    mock_storage()->SimulateFindSubResource(
        AppCacheEntry(AppCacheEntry::EXPLICIT, 1), AppCacheEntry(), false);

    EXPECT_TRUE(CreateRequestAndHandler(GURL("http://blah/"), host_,
                                        RESOURCE_TYPE_SUB_RESOURCE));
    EXPECT_TRUE(handler_.get());
    job_.reset(handler_->MaybeLoadResource(nullptr));
    EXPECT_TRUE(job_.get());
    EXPECT_TRUE(job_->IsDeliveringAppCacheResponse());

    std::unique_ptr<AppCacheJob> fallback_job(
        handler_->MaybeLoadFallbackForRedirect(nullptr,
                                               GURL("http://blah/redirect")));
    EXPECT_FALSE(fallback_job);
    fallback_job.reset(handler_->MaybeLoadFallbackForResponse(nullptr));
    EXPECT_FALSE(fallback_job);

    TestFinished();
  }

  // SubResource_RedirectFallback -----------------------------

  void SubResource_RedirectFallback() {
    // Redirects to resources in the a different origin are subject to
    // fallback namespaces.
    host_->AssociateCompleteCache(MakeNewCache());

    mock_storage()->SimulateFindSubResource(
        AppCacheEntry(), AppCacheEntry(AppCacheEntry::EXPLICIT, 1), false);

    EXPECT_TRUE(CreateRequestAndHandler(GURL("http://blah/"), host_,
                                        RESOURCE_TYPE_SUB_RESOURCE));
    EXPECT_TRUE(handler_.get());
    job_.reset(handler_->MaybeLoadResource(nullptr));
    // If the request goes to the network, the URLRequest job is destroyed. The
    // URLLoader job is destroyed when the URLLoaderClient disconnects.
    if (request_handler_type_ == URLREQUEST) {
      EXPECT_FALSE(job_.get());
    } else {
      // In the URLLoader world, the same job instance is used to provide the
      // fallback.
      EXPECT_TRUE(job_.get());
      EXPECT_TRUE(job_->IsDeliveringNetworkResponse());
    }

    std::unique_ptr<AppCacheJob> redirect_fallback_job;
    redirect_fallback_job.reset(handler_->MaybeLoadFallbackForRedirect(
        nullptr, GURL("http://not_blah/redirect")));
    EXPECT_TRUE(redirect_fallback_job.get());
    EXPECT_TRUE(redirect_fallback_job->IsDeliveringAppCacheResponse());

    if (request_handler_type_ == URLLOADER) {
      // In the URLLoader world, the same job instance is used to provide the
      // fallback.
      EXPECT_EQ(job_.get(), redirect_fallback_job.get());
      job_.release();
    }

    std::unique_ptr<AppCacheJob> fallback_job(
        handler_->MaybeLoadFallbackForResponse(nullptr));
    EXPECT_FALSE(fallback_job);

    TestFinished();
  }

  // SubResource_NoRedirectFallback -----------------------------

  void SubResource_NoRedirectFallback() {
    // Redirects to resources in the same-origin are not subject to
    // fallback namespaces.
    host_->AssociateCompleteCache(MakeNewCache());

    mock_storage()->SimulateFindSubResource(
        AppCacheEntry(), AppCacheEntry(AppCacheEntry::EXPLICIT, 1), false);

    EXPECT_TRUE(CreateRequestAndHandler(GURL("http://blah/"), host_,
                                        RESOURCE_TYPE_SUB_RESOURCE));
    EXPECT_TRUE(handler_.get());
    job_.reset(handler_->MaybeLoadResource(nullptr));
    if (request_handler_type_ == URLREQUEST) {
      EXPECT_FALSE(job_.get());
    } else {
      // In the URLLoader world, the same job instance is used to provide the
      // fallback.
      EXPECT_TRUE(job_.get());
      EXPECT_TRUE(job_->IsDeliveringNetworkResponse());
    }

    std::unique_ptr<AppCacheJob> fallback_job(
        handler_->MaybeLoadFallbackForRedirect(nullptr,
                                               GURL("http://blah/redirect")));
    EXPECT_FALSE(fallback_job);

    // Fallback responses are provided by a new job instance.
    if (request_handler_type_ == URLLOADER)
      job_.reset(nullptr);

    SimulateResponseCode(200);
    fallback_job.reset(handler_->MaybeLoadFallbackForResponse(nullptr));
    EXPECT_FALSE(fallback_job);

    TestFinished();
  }

  // SubResource_Network -----------------------------

  void SubResource_Network() {
    // A sub-resource load where the resource is in a network namespace,
    // should result in the system using a 'real' job to do the network
    // retrieval.
    host_->AssociateCompleteCache(MakeNewCache());

    mock_storage()->SimulateFindSubResource(
        AppCacheEntry(), AppCacheEntry(), true);

    EXPECT_TRUE(CreateRequestAndHandler(GURL("http://blah/"), host_,
                                        RESOURCE_TYPE_SUB_RESOURCE));
    EXPECT_TRUE(handler_.get());
    job_.reset(handler_->MaybeLoadResource(nullptr));
    // If the request goes to the network, the URLRequest job is destroyed. The
    // URLLoader job is destroyed when the URLLoaderClient disconnects.
    if (request_handler_type_ == URLREQUEST) {
      EXPECT_FALSE(job_.get());
    } else {
      EXPECT_TRUE(job_.get());
      EXPECT_TRUE(job_->IsDeliveringNetworkResponse());
    }

    std::unique_ptr<AppCacheJob> fallback_job(
        handler_->MaybeLoadFallbackForRedirect(nullptr,
                                               GURL("http://blah/redirect")));
    EXPECT_FALSE(fallback_job);
    fallback_job.reset(handler_->MaybeLoadFallbackForResponse(nullptr));
    EXPECT_FALSE(fallback_job);

    TestFinished();
  }

  // DestroyedHost -----------------------------

  void DestroyedHost() {
    host_->AssociateCompleteCache(MakeNewCache());

    mock_storage()->SimulateFindSubResource(
        AppCacheEntry(AppCacheEntry::EXPLICIT, 1), AppCacheEntry(), false);

    EXPECT_TRUE(CreateRequestAndHandler(GURL("http://blah/"), host_,
                                        RESOURCE_TYPE_SUB_RESOURCE));
    EXPECT_TRUE(handler_.get());

    backend_impl_->UnregisterHost(1);
    host_ = NULL;

    EXPECT_FALSE(handler_->MaybeLoadResource(nullptr));
    EXPECT_FALSE(handler_->MaybeLoadFallbackForRedirect(
        nullptr, GURL("http://blah/redirect")));
    EXPECT_FALSE(handler_->MaybeLoadFallbackForResponse(nullptr));

    TestFinished();
  }

  // DestroyedHostWithWaitingJob -----------------------------

  void DestroyedHostWithWaitingJob() {
    // Precondition, the host is waiting on cache selection.
    host_->pending_selected_cache_id_ = 1;

    EXPECT_TRUE(CreateRequestAndHandler(GURL("http://blah/"), host_,
                                        RESOURCE_TYPE_SUB_RESOURCE));
    EXPECT_TRUE(handler_.get());

    job_.reset(handler_->MaybeLoadResource(nullptr));
    EXPECT_TRUE(job_.get());
    EXPECT_TRUE(job_->IsWaiting());

    backend_impl_->UnregisterHost(1);
    host_ = NULL;

    if (request_handler_type_ == URLREQUEST) {
      EXPECT_TRUE(
          static_cast<AppCacheURLRequestJob*>(job_.get())->has_been_killed());
    }
    EXPECT_FALSE(handler_->MaybeLoadResource(nullptr));
    EXPECT_FALSE(handler_->MaybeLoadFallbackForRedirect(
        nullptr, GURL("http://blah/redirect")));
    EXPECT_FALSE(handler_->MaybeLoadFallbackForResponse(nullptr));

    TestFinished();
  }

  // DestroyedService -----------------------------

  void DestroyedService() {
    host_->AssociateCompleteCache(MakeNewCache());

    mock_storage()->SimulateFindSubResource(
        AppCacheEntry(AppCacheEntry::EXPLICIT, 1), AppCacheEntry(), false);

    EXPECT_TRUE(CreateRequestAndHandler(GURL("http://blah/"), host_,
                                        RESOURCE_TYPE_SUB_RESOURCE));
    EXPECT_TRUE(handler_.get());
    job_.reset(handler_->MaybeLoadResource(nullptr));
    EXPECT_TRUE(job_.get());

    backend_impl_.reset();
    mock_frontend_.reset();
    mock_service_.reset();
    mock_policy_.reset();
    host_ = NULL;

    if (request_handler_type_ == URLREQUEST) {
      EXPECT_TRUE(
          static_cast<AppCacheURLRequestJob*>(job_.get())->has_been_killed());
    }
    EXPECT_FALSE(handler_->MaybeLoadResource(nullptr));
    EXPECT_FALSE(handler_->MaybeLoadFallbackForRedirect(
        nullptr, GURL("http://blah/redirect")));
    EXPECT_FALSE(handler_->MaybeLoadFallbackForResponse(nullptr));

    TestFinished();
  }

  void DestroyedServiceWithCrossSiteNav() {
    EXPECT_TRUE(CreateRequestAndHandler(GURL("http://blah/"), host_,
                                        RESOURCE_TYPE_MAIN_FRAME));
    EXPECT_TRUE(handler_.get());
    handler_->PrepareForCrossSiteTransfer(backend_impl_->process_id());
    EXPECT_TRUE(handler_->host_for_cross_site_transfer_.get());

    backend_impl_.reset();
    mock_frontend_.reset();
    mock_service_.reset();
    mock_policy_.reset();
    host_ = NULL;

    EXPECT_FALSE(handler_->host_for_cross_site_transfer_.get());
    EXPECT_FALSE(handler_->MaybeLoadResource(nullptr));
    EXPECT_FALSE(handler_->MaybeLoadFallbackForRedirect(
        nullptr, GURL("http://blah/redirect")));
    EXPECT_FALSE(handler_->MaybeLoadFallbackForResponse(nullptr));

    TestFinished();
  }

  // UnsupportedScheme -----------------------------

  void UnsupportedScheme() {
    // Precondition, the host is waiting on cache selection.
    host_->pending_selected_cache_id_ = 1;

    EXPECT_TRUE(CreateRequestAndHandler(GURL("ftp://blah/"), host_,
                                        RESOURCE_TYPE_SUB_RESOURCE));
    EXPECT_TRUE(handler_.get());  // we could redirect to http (conceivably)

    EXPECT_FALSE(handler_->MaybeLoadResource(nullptr));
    EXPECT_FALSE(handler_->MaybeLoadFallbackForRedirect(
        nullptr, GURL("ftp://blah/redirect")));
    EXPECT_FALSE(handler_->MaybeLoadFallbackForResponse(nullptr));

    TestFinished();
  }

  // CanceledRequest -----------------------------

  void CanceledRequest() {
    EXPECT_TRUE(CreateRequestAndHandler(GURL("http://blah/"), host_,
                                        RESOURCE_TYPE_MAIN_FRAME));
    EXPECT_TRUE(handler_.get());

    job_.reset(handler_->MaybeLoadResource(nullptr));
    EXPECT_TRUE(job_.get());
    EXPECT_TRUE(job_->IsWaiting());
    EXPECT_FALSE(job_->IsStarted());

    base::WeakPtr<AppCacheJob> weak_job = job_->GetWeakPtr();

    // TODO(ananta/michaeln)
    // Rewrite this test for URLLoader.
    if (request_handler_type_ == URLREQUEST) {
      std::unique_ptr<AppCacheURLRequestJob> job(
          static_cast<AppCacheURLRequestJob*>(job_.release()));
      job_factory_->SetJob(std::move(job));

      request_->AsURLRequest()->GetURLRequest()->Start();
      ASSERT_TRUE(weak_job);
      EXPECT_TRUE(weak_job->IsStarted());

      request_->AsURLRequest()->GetURLRequest()->Cancel();
      ASSERT_FALSE(weak_job);
    }

    EXPECT_FALSE(handler_->MaybeLoadFallbackForResponse(nullptr));

    TestFinished();
  }

  // WorkerRequest -----------------------------

  void WorkerRequest() {
    EXPECT_TRUE(AppCacheRequestHandler::IsMainResourceType(
        RESOURCE_TYPE_MAIN_FRAME));
    EXPECT_TRUE(AppCacheRequestHandler::IsMainResourceType(
        RESOURCE_TYPE_SUB_FRAME));
    EXPECT_TRUE(AppCacheRequestHandler::IsMainResourceType(
        RESOURCE_TYPE_SHARED_WORKER));
    EXPECT_FALSE(AppCacheRequestHandler::IsMainResourceType(
        RESOURCE_TYPE_WORKER));


    const int kParentHostId = host_->host_id();
    const int kWorkerHostId = 2;
    const int kAbandonedWorkerHostId = 3;
    const int kNonExsitingHostId = 700;

    backend_impl_->RegisterHost(kWorkerHostId);
    AppCacheHost* worker_host = backend_impl_->GetHost(kWorkerHostId);
    worker_host->SelectCacheForWorker(kParentHostId, kMockProcessId);
    EXPECT_TRUE(CreateRequestAndHandler(GURL("http://blah/"), worker_host,
                                        RESOURCE_TYPE_SHARED_WORKER));
    EXPECT_TRUE(handler_.get());
    // Verify that the handler is associated with the parent host.
    EXPECT_EQ(host_, handler_->host_);

    // Create a new worker host, but associate it with a parent host that
    // does not exists to simulate the host having been torn down.
    backend_impl_->UnregisterHost(kWorkerHostId);
    backend_impl_->RegisterHost(kAbandonedWorkerHostId);
    worker_host = backend_impl_->GetHost(kAbandonedWorkerHostId);
    EXPECT_EQ(NULL, backend_impl_->GetHost(kNonExsitingHostId));
    worker_host->SelectCacheForWorker(kNonExsitingHostId, kMockProcessId);
    EXPECT_TRUE(CreateRequestAndHandler(GURL("http://blah/"), worker_host,
                                        RESOURCE_TYPE_SHARED_WORKER));
    EXPECT_FALSE(handler_.get());

    TestFinished();
  }

  // MainResource_Blocked --------------------------------------------------

  void MainResource_Blocked() {
    PushNextTask(
        base::BindOnce(&AppCacheRequestHandlerTest::Verify_MainResource_Blocked,
                       base::Unretained(this)));

    EXPECT_TRUE(CreateRequestAndHandler(GURL("http://blah/"), host_,
                                        RESOURCE_TYPE_MAIN_FRAME));
    EXPECT_TRUE(handler_.get());

    mock_policy_->can_load_return_value_ = false;
    mock_storage()->SimulateFindMainResource(
        AppCacheEntry(AppCacheEntry::EXPLICIT, 1),
        GURL(), AppCacheEntry(),
        1, 2, GURL("http://blah/manifest/"));

    job_.reset(handler_->MaybeLoadResource(nullptr));
    EXPECT_TRUE(job_.get());
    EXPECT_TRUE(job_->IsWaiting());

    // We have to wait for completion of storage->FindResponseForMainRequest.
    ScheduleNextTask();
  }

  void Verify_MainResource_Blocked() {
    EXPECT_FALSE(job_->IsWaiting());
    EXPECT_FALSE(job_->IsDeliveringAppCacheResponse());

    EXPECT_EQ(0, handler_->found_cache_id_);
    EXPECT_EQ(0, handler_->found_group_id_);
    EXPECT_TRUE(handler_->found_manifest_url_.is_empty());
    EXPECT_TRUE(host_->preferred_manifest_url().is_empty());
    EXPECT_TRUE(host_->main_resource_blocked_);
    EXPECT_EQ(host_->blocked_manifest_url_, "http://blah/manifest/");

    TestFinished();
  }

  // Test case helpers --------------------------------------------------

  AppCache* MakeNewCache() {
    AppCache* cache = new AppCache(
        mock_storage(), mock_storage()->NewCacheId());
    cache->set_complete(true);
    AppCacheGroup* group = new AppCacheGroup(
        mock_storage(), GURL("http://blah/manifest"),
        mock_storage()->NewGroupId());
    group->AddCache(cache);
    return cache;
  }

  MockAppCacheStorage* mock_storage() {
    return reinterpret_cast<MockAppCacheStorage*>(mock_service_->storage());
  }

  bool CreateRequestAndHandler(const GURL& url,
                               AppCacheHost* host,
                               ResourceType resource_type) {
    if (request_handler_type_ == URLREQUEST) {
      url_request_ = empty_context_->CreateRequest(
          url, net::DEFAULT_PRIORITY, &delegate_, TRAFFIC_ANNOTATION_FOR_TESTS);

      std::unique_ptr<AppCacheRequest> request =
          AppCacheURLRequest::Create(url_request_.get());
      request_ = request.get();
      handler_ =
          host->CreateRequestHandler(std::move(request), resource_type, false);
      return true;
    } else if (request_handler_type_ == URLLOADER) {
      ResourceRequest resource_request;
      resource_request.url = url;
      resource_request.method = "GET";
      std::unique_ptr<AppCacheRequest> request =
          AppCacheURLLoaderRequest::Create(resource_request);
      request_ = request.get();
      handler_ =
          host->CreateRequestHandler(std::move(request), resource_type, false);
      return true;
    }
    return false;
  }

  // Data members --------------------------------------------------

  std::unique_ptr<base::WaitableEvent> test_finished_event_;
  std::stack<base::OnceClosure> task_stack_;
  std::unique_ptr<MockAppCacheService> mock_service_;
  std::unique_ptr<AppCacheBackendImpl> backend_impl_;
  std::unique_ptr<MockFrontend> mock_frontend_;
  std::unique_ptr<MockAppCachePolicy> mock_policy_;
  AppCacheHost* host_;
  std::unique_ptr<net::URLRequestContext> empty_context_;
  std::unique_ptr<MockURLRequestJobFactory> job_factory_;
  MockURLRequestDelegate delegate_;
  AppCacheRequest* request_;
  std::unique_ptr<net::URLRequest> url_request_;
  std::unique_ptr<AppCacheRequestHandler> handler_;
  std::unique_ptr<AppCacheJob> job_;

  static std::unique_ptr<base::Thread> io_thread_;
  static std::unique_ptr<base::test::ScopedTaskEnvironment>
      scoped_task_environment_;

  RequestHandlerType request_handler_type_;
  base::test::ScopedFeatureList feature_list_;
};

// static
std::unique_ptr<base::Thread> AppCacheRequestHandlerTest::io_thread_;
std::unique_ptr<base::test::ScopedTaskEnvironment>
    AppCacheRequestHandlerTest::scoped_task_environment_;

TEST_P(AppCacheRequestHandlerTest, MainResource_Miss) {
  RunTestOnIOThread(&AppCacheRequestHandlerTest::MainResource_Miss);
}

TEST_P(AppCacheRequestHandlerTest, MainResource_Hit) {
  RunTestOnIOThread(&AppCacheRequestHandlerTest::MainResource_Hit);
}

TEST_P(AppCacheRequestHandlerTest, MainResource_Fallback) {
  RunTestOnIOThread(&AppCacheRequestHandlerTest::MainResource_Fallback);
}

TEST_P(AppCacheRequestHandlerTest, MainResource_FallbackOverride) {
  RunTestOnIOThread(
      &AppCacheRequestHandlerTest::MainResource_FallbackOverride);
}

TEST_P(AppCacheRequestHandlerTest, SubResource_Miss_WithNoCacheSelected) {
  RunTestOnIOThread(
      &AppCacheRequestHandlerTest::SubResource_Miss_WithNoCacheSelected);
}

TEST_P(AppCacheRequestHandlerTest, SubResource_Miss_WithCacheSelected) {
  RunTestOnIOThread(
      &AppCacheRequestHandlerTest::SubResource_Miss_WithCacheSelected);
}

TEST_P(AppCacheRequestHandlerTest, SubResource_Miss_WithWaitForCacheSelection) {
  RunTestOnIOThread(
      &AppCacheRequestHandlerTest::SubResource_Miss_WithWaitForCacheSelection);
}

TEST_P(AppCacheRequestHandlerTest, SubResource_Hit) {
  RunTestOnIOThread(&AppCacheRequestHandlerTest::SubResource_Hit);
}

TEST_P(AppCacheRequestHandlerTest, SubResource_RedirectFallback) {
  RunTestOnIOThread(&AppCacheRequestHandlerTest::SubResource_RedirectFallback);
}

TEST_P(AppCacheRequestHandlerTest, SubResource_NoRedirectFallback) {
  RunTestOnIOThread(
    &AppCacheRequestHandlerTest::SubResource_NoRedirectFallback);
}

TEST_P(AppCacheRequestHandlerTest, SubResource_Network) {
  RunTestOnIOThread(&AppCacheRequestHandlerTest::SubResource_Network);
}

TEST_P(AppCacheRequestHandlerTest, DestroyedHost) {
  RunTestOnIOThread(&AppCacheRequestHandlerTest::DestroyedHost);
}

TEST_P(AppCacheRequestHandlerTest, DestroyedHostWithWaitingJob) {
  RunTestOnIOThread(&AppCacheRequestHandlerTest::DestroyedHostWithWaitingJob);
}

TEST_P(AppCacheRequestHandlerTest, DestroyedService) {
  RunTestOnIOThread(&AppCacheRequestHandlerTest::DestroyedService);
}

TEST_P(AppCacheRequestHandlerTest, DestroyedServiceWithCrossSiteNav) {
  RunTestOnIOThread(
      &AppCacheRequestHandlerTest::DestroyedServiceWithCrossSiteNav);
}

TEST_P(AppCacheRequestHandlerTest, UnsupportedScheme) {
  RunTestOnIOThread(&AppCacheRequestHandlerTest::UnsupportedScheme);
}

TEST_P(AppCacheRequestHandlerTest, CanceledRequest) {
  RunTestOnIOThread(&AppCacheRequestHandlerTest::CanceledRequest);
}

TEST_P(AppCacheRequestHandlerTest, WorkerRequest) {
  RunTestOnIOThread(&AppCacheRequestHandlerTest::WorkerRequest);
}

TEST_P(AppCacheRequestHandlerTest, MainResource_Blocked) {
  RunTestOnIOThread(&AppCacheRequestHandlerTest::MainResource_Blocked);
}

INSTANTIATE_TEST_CASE_P(,
                        AppCacheRequestHandlerTest,
                        ::testing::Values(URLREQUEST, URLLOADER));

}  // namespace content
