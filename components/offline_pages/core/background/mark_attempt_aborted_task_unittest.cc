// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/offline_pages/core/background/mark_attempt_aborted_task.h"

#include <memory>

#include "base/bind.h"
#include "base/test/test_simple_task_runner.h"
#include "base/threading/thread_task_runner_handle.h"
#include "components/offline_pages/core/background/change_requests_state_task.h"
#include "components/offline_pages/core/background/mark_attempt_started_task.h"
#include "components/offline_pages/core/background/request_queue_in_memory_store.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace offline_pages {
namespace {
const int64_t kRequestId1 = 42;
const int64_t kRequestId2 = 44;
const GURL kUrl1("http://example.com");
const ClientId kClientId1("download", "1234");
}  // namespace

class MarkAttemptAbortedTaskTest : public testing::Test {
 public:
  MarkAttemptAbortedTaskTest();
  ~MarkAttemptAbortedTaskTest() override;

  void PumpLoop();

  void InitializeStore(RequestQueueStore* store);
  void AddItemToStore(RequestQueueStore* store);
  void ChangeRequestsStateCallback(
      std::unique_ptr<UpdateRequestsResult> result);

  void ClearResults();

  UpdateRequestsResult* last_result() const { return result_.get(); }

 private:
  void InitializeStoreDone(bool success);
  void AddRequestDone(ItemActionStatus status);

  std::unique_ptr<UpdateRequestsResult> result_;
  scoped_refptr<base::TestSimpleTaskRunner> task_runner_;
  base::ThreadTaskRunnerHandle task_runner_handle_;
};

MarkAttemptAbortedTaskTest::MarkAttemptAbortedTaskTest()
    : task_runner_(new base::TestSimpleTaskRunner),
      task_runner_handle_(task_runner_) {}

MarkAttemptAbortedTaskTest::~MarkAttemptAbortedTaskTest() {}

void MarkAttemptAbortedTaskTest::PumpLoop() {
  task_runner_->RunUntilIdle();
}

void MarkAttemptAbortedTaskTest::InitializeStore(RequestQueueStore* store) {
  store->Initialize(base::Bind(&MarkAttemptAbortedTaskTest::InitializeStoreDone,
                               base::Unretained(this)));
  PumpLoop();
}

void MarkAttemptAbortedTaskTest::AddItemToStore(RequestQueueStore* store) {
  base::Time creation_time = base::Time::Now();
  SavePageRequest request_1(kRequestId1, kUrl1, kClientId1, creation_time,
                            true);
  store->AddRequest(request_1,
                    base::Bind(&MarkAttemptAbortedTaskTest::AddRequestDone,
                               base::Unretained(this)));
  PumpLoop();
}

void MarkAttemptAbortedTaskTest::ChangeRequestsStateCallback(
    std::unique_ptr<UpdateRequestsResult> result) {
  result_ = std::move(result);
}

void MarkAttemptAbortedTaskTest::ClearResults() {
  result_.reset(nullptr);
}

void MarkAttemptAbortedTaskTest::InitializeStoreDone(bool success) {
  ASSERT_TRUE(success);
}

void MarkAttemptAbortedTaskTest::AddRequestDone(ItemActionStatus status) {
  ASSERT_EQ(ItemActionStatus::SUCCESS, status);
}

TEST_F(MarkAttemptAbortedTaskTest, MarkAttemptAbortedWhenStoreEmpty) {
  RequestQueueInMemoryStore store;
  InitializeStore(&store);

  MarkAttemptAbortedTask task(
      &store, kRequestId1,
      base::Bind(&MarkAttemptAbortedTaskTest::ChangeRequestsStateCallback,
                 base::Unretained(this)));
  task.Run();
  PumpLoop();
  ASSERT_TRUE(last_result());
  EXPECT_EQ(1UL, last_result()->item_statuses.size());
  EXPECT_EQ(kRequestId1, last_result()->item_statuses.at(0).first);
  EXPECT_EQ(ItemActionStatus::NOT_FOUND,
            last_result()->item_statuses.at(0).second);
  EXPECT_EQ(0UL, last_result()->updated_items.size());
}

TEST_F(MarkAttemptAbortedTaskTest, MarkAttemptAbortedWhenExists) {
  RequestQueueInMemoryStore store;
  InitializeStore(&store);
  AddItemToStore(&store);

  // First mark attempt started.
  MarkAttemptStartedTask start_request_task(
      &store, kRequestId1,
      base::Bind(&MarkAttemptAbortedTaskTest::ChangeRequestsStateCallback,
                 base::Unretained(this)));
  start_request_task.Run();
  PumpLoop();
  ClearResults();

  MarkAttemptAbortedTask task(
      &store, kRequestId1,
      base::Bind(&MarkAttemptAbortedTaskTest::ChangeRequestsStateCallback,
                 base::Unretained(this)));

  task.Run();
  PumpLoop();
  ASSERT_TRUE(last_result());
  EXPECT_EQ(1UL, last_result()->item_statuses.size());
  EXPECT_EQ(kRequestId1, last_result()->item_statuses.at(0).first);
  EXPECT_EQ(ItemActionStatus::SUCCESS,
            last_result()->item_statuses.at(0).second);
  EXPECT_EQ(1UL, last_result()->updated_items.size());
  EXPECT_EQ(SavePageRequest::RequestState::AVAILABLE,
            last_result()->updated_items.at(0).request_state());
}

TEST_F(MarkAttemptAbortedTaskTest, MarkAttemptAbortedWhenItemMissing) {
  RequestQueueInMemoryStore store;
  InitializeStore(&store);
  AddItemToStore(&store);

  MarkAttemptAbortedTask task(
      &store, kRequestId2,
      base::Bind(&MarkAttemptAbortedTaskTest::ChangeRequestsStateCallback,
                 base::Unretained(this)));
  task.Run();
  PumpLoop();
  ASSERT_TRUE(last_result());
  EXPECT_EQ(1UL, last_result()->item_statuses.size());
  EXPECT_EQ(kRequestId2, last_result()->item_statuses.at(0).first);
  EXPECT_EQ(ItemActionStatus::NOT_FOUND,
            last_result()->item_statuses.at(0).second);
  EXPECT_EQ(0UL, last_result()->updated_items.size());
}

TEST_F(MarkAttemptAbortedTaskTest, MarkAttemptAbortedWhenPaused) {
  RequestQueueInMemoryStore store;
  InitializeStore(&store);
  AddItemToStore(&store);

  // First mark attempt started.
  MarkAttemptStartedTask start_request_task(
      &store, kRequestId1,
      base::Bind(&MarkAttemptAbortedTaskTest::ChangeRequestsStateCallback,
                 base::Unretained(this)));
  start_request_task.Run();
  PumpLoop();
  ClearResults();

  // Mark the attempt as PAUSED, so we test the PAUSED to PAUSED transition.
    std::vector<int64_t> requests;
  requests.push_back(kRequestId1);
  ChangeRequestsStateTask pauseTask(
      &store, requests, SavePageRequest::RequestState::PAUSED,
      base::Bind(&MarkAttemptAbortedTaskTest::ChangeRequestsStateCallback,
                 base::Unretained(this)));
  pauseTask.Run();
  PumpLoop();

  // Abort the task, the state should not change from PAUSED.
  MarkAttemptAbortedTask abortTask(
      &store, kRequestId1,
      base::Bind(&MarkAttemptAbortedTaskTest::ChangeRequestsStateCallback,
                 base::Unretained(this)));

  abortTask.Run();
  PumpLoop();
  ASSERT_TRUE(last_result());
  EXPECT_EQ(1UL, last_result()->item_statuses.size());
  EXPECT_EQ(kRequestId1, last_result()->item_statuses.at(0).first);
  EXPECT_EQ(ItemActionStatus::SUCCESS,
            last_result()->item_statuses.at(0).second);
  EXPECT_EQ(1UL, last_result()->updated_items.size());
  EXPECT_EQ(SavePageRequest::RequestState::PAUSED,
            last_result()->updated_items.at(0).request_state());
}

}  // namespace offline_pages
