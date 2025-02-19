// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/offline_pages/core/background/reconcile_task.h"

#include <memory>
#include <set>

#include "base/bind.h"
#include "base/test/test_simple_task_runner.h"
#include "base/threading/thread_task_runner_handle.h"
#include "components/offline_pages/core/background/request_coordinator.h"
#include "components/offline_pages/core/background/request_queue_in_memory_store.h"
#include "components/offline_pages/core/background/request_queue_store.h"
#include "components/offline_pages/core/background/save_page_request.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace offline_pages {

namespace {
// Data for request 1.
const int64_t kRequestId1 = 17;
const GURL kUrl1("https://google.com");
const ClientId kClientId1("bookmark", "1234");
// Data for request 2.
const int64_t kRequestId2 = 42;
const GURL kUrl2("http://nytimes.com");
const ClientId kClientId2("bookmark", "5678");
const bool kUserRequested = true;

// Default request
const SavePageRequest kEmptyRequest(0UL,
                                    GURL(""),
                                    ClientId("", ""),
                                    base::Time(),
                                    true);
}  // namespace

class ReconcileTaskTest : public testing::Test {
 public:
  ReconcileTaskTest();

  ~ReconcileTaskTest() override;

  void SetUp() override;

  void PumpLoop();

  void AddRequestDone(ItemActionStatus status);

  void GetRequestsCallback(
      bool success,
      std::vector<std::unique_ptr<SavePageRequest>> requests);

  void ReconcileCallback(std::unique_ptr<UpdateRequestsResult> result);

  void QueueRequests(const SavePageRequest& request1,
                     const SavePageRequest& request2);

  // Reset the factory and the task using the current policy.
  void MakeTask();

  ReconcileTask* task() { return task_.get(); }
  RequestQueueStore* store() { return store_.get(); }
  std::vector<std::unique_ptr<SavePageRequest>>& found_requests() {
    return found_requests_;
  }

 protected:
  void InitializeStoreDone(bool success);

  std::unique_ptr<RequestQueueStore> store_;
  std::unique_ptr<ReconcileTask> task_;
  std::vector<std::unique_ptr<SavePageRequest>> found_requests_;
  bool reconcile_called_;

 private:
  scoped_refptr<base::TestSimpleTaskRunner> task_runner_;
  base::ThreadTaskRunnerHandle task_runner_handle_;
};

ReconcileTaskTest::ReconcileTaskTest()
    : reconcile_called_(false),
      task_runner_(new base::TestSimpleTaskRunner),
      task_runner_handle_(task_runner_) {}

ReconcileTaskTest::~ReconcileTaskTest() {}

void ReconcileTaskTest::SetUp() {
  DeviceConditions conditions;
  store_.reset(new RequestQueueInMemoryStore());
  MakeTask();

  store_->Initialize(base::Bind(&ReconcileTaskTest::InitializeStoreDone,
                                base::Unretained(this)));
  PumpLoop();
}

void ReconcileTaskTest::PumpLoop() {
  task_runner_->RunUntilIdle();
}

void ReconcileTaskTest::AddRequestDone(ItemActionStatus status) {
  ASSERT_EQ(ItemActionStatus::SUCCESS, status);
}

void ReconcileTaskTest::GetRequestsCallback(
    bool success,
    std::vector<std::unique_ptr<SavePageRequest>> requests) {
  found_requests_ = std::move(requests);
}

void ReconcileTaskTest::ReconcileCallback(
    std::unique_ptr<UpdateRequestsResult> result) {
  reconcile_called_ = true;
  // Make sure the item in the callback is now AVAILABLE.
  EXPECT_EQ(SavePageRequest::RequestState::AVAILABLE,
            result->updated_items.at(0).request_state());
}

// Test helper to queue the two given requests.
void ReconcileTaskTest::QueueRequests(const SavePageRequest& request1,
                                      const SavePageRequest& request2) {
  DeviceConditions conditions;
  std::set<int64_t> disabled_requests;
  // Add test requests on the Queue.
  store_->AddRequest(request1, base::Bind(&ReconcileTaskTest::AddRequestDone,
                                          base::Unretained(this)));
  store_->AddRequest(request2, base::Bind(&ReconcileTaskTest::AddRequestDone,
                                          base::Unretained(this)));

  // Pump the loop to give the async queue the opportunity to do the adds.
  PumpLoop();
}

void ReconcileTaskTest::MakeTask() {
  task_.reset(new ReconcileTask(
      store_.get(), base::Bind(&ReconcileTaskTest::ReconcileCallback,
                               base::Unretained(this))));
}

void ReconcileTaskTest::InitializeStoreDone(bool success) {
  ASSERT_TRUE(success);
}

TEST_F(ReconcileTaskTest, Reconcile) {
  base::Time creation_time = base::Time::Now();
  // Request2 will be expired, request1 will be current.
  SavePageRequest request1(kRequestId1, kUrl1, kClientId1, creation_time,
                           kUserRequested);
  request1.set_request_state(SavePageRequest::RequestState::PAUSED);
  SavePageRequest request2(kRequestId2, kUrl2, kClientId2, creation_time,
                           kUserRequested);
  request2.set_request_state(SavePageRequest::RequestState::OFFLINING);
  QueueRequests(request1, request2);

  // Initiate cleanup.
  task()->Run();
  PumpLoop();

  // See what is left in the queue, should be just the other request.
  store()->GetRequests(base::Bind(&ReconcileTaskTest::GetRequestsCallback,
                                  base::Unretained(this)));
  PumpLoop();
  EXPECT_EQ(2UL, found_requests().size());

  // in case requests come back in a different order, check which is where.
  int request1_index = 0;
  int request2_index = 1;
  if (found_requests().at(0)->request_id() != kRequestId1) {
    request1_index = 1;
    request2_index = 0;
  }
  EXPECT_EQ(SavePageRequest::RequestState::PAUSED,
            found_requests().at(request1_index)->request_state());
  EXPECT_EQ(SavePageRequest::RequestState::AVAILABLE,
            found_requests().at(request2_index)->request_state());
  EXPECT_TRUE(reconcile_called_);
}

TEST_F(ReconcileTaskTest, NothingToReconcile) {
  base::Time creation_time = base::Time::Now();
  // Request2 will be expired, request1 will be current.
  SavePageRequest request1(kRequestId1, kUrl1, kClientId1, creation_time,
                           kUserRequested);
  request1.set_request_state(SavePageRequest::RequestState::PAUSED);
  SavePageRequest request2(kRequestId2, kUrl2, kClientId2, creation_time,
                           kUserRequested);
  request2.set_request_state(SavePageRequest::RequestState::AVAILABLE);
  QueueRequests(request1, request2);

  // Initiate cleanup.
  task()->Run();
  PumpLoop();

  // See what is left in the queue, should be just the other request.
  store()->GetRequests(base::Bind(&ReconcileTaskTest::GetRequestsCallback,
                                  base::Unretained(this)));
  PumpLoop();
  EXPECT_EQ(2UL, found_requests().size());

  // in case requests come back in a different order, check which is where.
  int request1_index = 0;
  int request2_index = 1;
  if (found_requests().at(0)->request_id() != kRequestId1) {
    request1_index = 1;
    request2_index = 0;
  }
  // Requests should still be in their starting states.
  EXPECT_EQ(SavePageRequest::RequestState::PAUSED,
            found_requests().at(request1_index)->request_state());
  EXPECT_EQ(SavePageRequest::RequestState::AVAILABLE,
            found_requests().at(request2_index)->request_state());
  // In this case, we do not expect the reconcile callback to be called.
  EXPECT_FALSE(reconcile_called_);
}

}  // namespace offline_pages
