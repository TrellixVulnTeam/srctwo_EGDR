// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_BACKGROUND_FETCH_BACKGROUND_FETCH_TEST_BASE_H_
#define CONTENT_BROWSER_BACKGROUND_FETCH_BACKGROUND_FETCH_TEST_BASE_H_

#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "content/browser/background_fetch/background_fetch_embedded_worker_test_helper.h"
#include "content/common/service_worker/service_worker_types.h"
#include "content/public/test/test_browser_context.h"
#include "content/public/test/test_browser_thread_bundle.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/origin.h"

namespace net {
class HttpResponseHeaders;
}

namespace content {

class BackgroundFetchRegistrationId;
class MockDownloadManager;
class ServiceWorkerRegistration;

// Base class containing common functionality needed in unit tests written for
// the Background Fetch feature.
class BackgroundFetchTestBase : public ::testing::Test {
 public:
  BackgroundFetchTestBase();
  ~BackgroundFetchTestBase() override;

  // ::testing::Test overrides.
  void SetUp() override;
  void TearDown() override;

  // Structure encapsulating the data for a injected response. Should only be
  // created by the builder, which also defines the ownership semantics.
  struct TestResponse {
    TestResponse();
    ~TestResponse();

    scoped_refptr<net::HttpResponseHeaders> headers;
    std::string data;
  };

  // Builder for creating a TestResponse object with the given data. The faked
  // download manager will respond to the corresponding request based on this.
  class TestResponseBuilder {
   public:
    explicit TestResponseBuilder(int response_code);
    ~TestResponseBuilder();

    TestResponseBuilder& AddResponseHeader(const std::string& name,
                                           const std::string& value);
    TestResponseBuilder& SetResponseData(std::string data);

    // Finalizes the builder and invalidates the underlying response.
    std::unique_ptr<TestResponse> Build();

   private:
    std::unique_ptr<TestResponse> response_;

    DISALLOW_COPY_AND_ASSIGN(TestResponseBuilder);
  };

  // Creates a Background Fetch registration backed by a Service Worker
  // registration for the testing origin. The resulting registration will be
  // stored in |*registration_id|. Returns whether creation was successful,
  // which must be asserted by tests. The ServiceWorkerRegistration that
  // backs the |*registration_id| will be kept alive for the test's lifetime.
  bool CreateRegistrationId(const std::string& tag,
                            BackgroundFetchRegistrationId* registration_id)
      WARN_UNUSED_RESULT;

  // Creates a ServiceWorkerFetchRequest instance for the given details and
  // provides a faked |response| with the faked download manager.
  ServiceWorkerFetchRequest CreateRequestWithProvidedResponse(
      const std::string& method,
      const std::string& url,
      std::unique_ptr<TestResponse> response);

  // Returns the embedded worker test helper instance, which can be used to
  // influence the behaviour of the Service Worker events.
  BackgroundFetchEmbeddedWorkerTestHelper* embedded_worker_test_helper() {
    return &embedded_worker_test_helper_;
  }

  // Returns the browser context that should be used for the tests.
  BrowserContext* browser_context() { return &browser_context_; }

  // Returns the download manager used for the tests.
  MockDownloadManager* download_manager();

  // Returns the origin that should be used for Background Fetch tests.
  const url::Origin& origin() const { return origin_; }

 protected:
  TestBrowserThreadBundle thread_bundle_;  // Must be first member.

 private:
  class RespondingDownloadManager;

  TestBrowserContext browser_context_;

  RespondingDownloadManager* download_manager_;  // owned by |browser_context_|

  BackgroundFetchEmbeddedWorkerTestHelper embedded_worker_test_helper_;

  url::Origin origin_;

  // Vector of ServiceWorkerRegistration instances that have to be kept alive
  // for the lifetime of this test.
  std::vector<scoped_refptr<ServiceWorkerRegistration>>
      service_worker_registrations_;

  bool set_up_called_ = false;
  bool tear_down_called_ = false;

  DISALLOW_COPY_AND_ASSIGN(BackgroundFetchTestBase);
};

}  // namespace content

#endif  // CONTENT_BROWSER_BACKGROUND_FETCH_BACKGROUND_FETCH_TEST_BASE_H_
