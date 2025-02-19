// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/task_manager/sampling/shared_sampler.h"

#include <windows.h>

#include <memory>

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "base/sequenced_task_runner.h"
#include "base/task_scheduler/post_task.h"
#include "base/threading/sequenced_worker_pool.h"
#include "base/time/time.h"
#include "chrome/browser/task_manager/sampling/shared_sampler_win_defines.h"
#include "chrome/browser/task_manager/task_manager_observer.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/test/test_browser_thread_bundle.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace task_manager {

// This test class drives SharedSampler in a way similar to the real
// implementation in TaskManagerImpl and TaskGroup.
class SharedSamplerTest : public testing::Test {
 public:
  SharedSamplerTest()
      : blocking_pool_runner_(GetBlockingPoolRunner()),
        shared_sampler_(new SharedSampler(blocking_pool_runner_)) {
    shared_sampler_->RegisterCallbacks(
        base::GetCurrentProcId(),
        base::Bind(&SharedSamplerTest::OnIdleWakeupsRefreshDone,
                   base::Unretained(this)),
        base::Bind(&SharedSamplerTest::OnPhysicalMemoryUsageRefreshDone,
                   base::Unretained(this)),
        base::Bind(&SharedSamplerTest::OnStartTimeRefreshDone,
                   base::Unretained(this)),
        base::Bind(&SharedSamplerTest::OnCpuTimeRefreshDone,
                   base::Unretained(this)));
  }

  ~SharedSamplerTest() override {}

 protected:
  int64_t physical_bytes() const { return physical_bytes_; }

  base::Time start_time() const { return start_time_; }

  base::TimeDelta cpu_time() const { return cpu_time_; }

  int idle_wakeups_per_second() const { return idle_wakeups_per_second_; }

  int64_t finished_refresh_type() const { return finished_refresh_type_; }

  void StartRefresh(int64_t refresh_flags) {
    finished_refresh_type_ = 0;
    expected_refresh_type_ = refresh_flags;
    shared_sampler_->Refresh(base::GetCurrentProcId(), refresh_flags);
  }

  void WaitUntilRefreshDone() {
    base::RunLoop run_loop;
    quit_closure_ = run_loop.QuitWhenIdleClosure();
    run_loop.Run();
  }

 private:
  static scoped_refptr<base::SequencedTaskRunner> GetBlockingPoolRunner() {
    return base::CreateSequencedTaskRunnerWithTraits({base::MayBlock()});
  }

  void OnRefreshTypeFinished(int64_t finished_refresh_type) {
    finished_refresh_type_ |= finished_refresh_type;

    if (finished_refresh_type_ == expected_refresh_type_)
      quit_closure_.Run();
  }

  void OnPhysicalMemoryUsageRefreshDone(int64_t physical_bytes) {
    physical_bytes_ = physical_bytes;
    OnRefreshTypeFinished(REFRESH_TYPE_PHYSICAL_MEMORY);
  }

  void OnIdleWakeupsRefreshDone(int idle_wakeups_per_second) {
    idle_wakeups_per_second_ = idle_wakeups_per_second;
    OnRefreshTypeFinished(REFRESH_TYPE_IDLE_WAKEUPS);
  }

  void OnStartTimeRefreshDone(base::Time start_time) {
    start_time_ = start_time;
    OnRefreshTypeFinished(REFRESH_TYPE_START_TIME);
  }

  void OnCpuTimeRefreshDone(base::TimeDelta cpu_time) {
    cpu_time_ = cpu_time;
    OnRefreshTypeFinished(REFRESH_TYPE_CPU_TIME);
  }

  int64_t expected_refresh_type_ = 0;
  int64_t finished_refresh_type_ = 0;
  base::Closure quit_closure_;

  int64_t physical_bytes_ = 0;
  int idle_wakeups_per_second_ = -1;
  base::Time start_time_;
  base::TimeDelta cpu_time_;

  content::TestBrowserThreadBundle thread_bundle_;
  scoped_refptr<base::SequencedTaskRunner> blocking_pool_runner_;
  scoped_refptr<SharedSampler> shared_sampler_;

  DISALLOW_COPY_AND_ASSIGN(SharedSamplerTest);
};

// Tests that Idle Wakeups per second value can be obtained from SharedSampler.
TEST_F(SharedSamplerTest, IdleWakeups) {
  StartRefresh(REFRESH_TYPE_IDLE_WAKEUPS);
  WaitUntilRefreshDone();
  EXPECT_EQ(REFRESH_TYPE_IDLE_WAKEUPS, finished_refresh_type());

  // Since idle_wakeups_per_second is an incremental value that reflects delta
  // between two refreshes, the first refresh always returns zero value. This
  // is consistent with implementation on other platforms.
  EXPECT_EQ(0, idle_wakeups_per_second());

  // Do a short sleep - that should trigger a context switch.
  ::Sleep(1);

  // Do another refresh.
  StartRefresh(REFRESH_TYPE_IDLE_WAKEUPS);
  WaitUntilRefreshDone();

  // Should get a greater than zero rate now.
  EXPECT_GT(idle_wakeups_per_second(), 0);
}

// Verifies that Memory (Private WS) value can be obtained from Shared Sampler.
TEST_F(SharedSamplerTest, PhysicalMemory) {
  StartRefresh(REFRESH_TYPE_PHYSICAL_MEMORY);
  WaitUntilRefreshDone();
  EXPECT_EQ(REFRESH_TYPE_PHYSICAL_MEMORY, finished_refresh_type());

  int64_t initial_value = physical_bytes();

  // Allocate a large continuos block of memory.
  const int allocated_size = 4 * 1024 * 1024;
  std::vector<uint8_t> memory_block(allocated_size);

  StartRefresh(REFRESH_TYPE_PHYSICAL_MEMORY);
  WaitUntilRefreshDone();

  // Verify that physical bytes has increased accordingly.
  EXPECT_GE(physical_bytes(), initial_value + allocated_size);
}

// Tests that process start time can be obtained from SharedSampler.
TEST_F(SharedSamplerTest, StartTime) {
  StartRefresh(REFRESH_TYPE_START_TIME);
  WaitUntilRefreshDone();
  EXPECT_EQ(REFRESH_TYPE_START_TIME, finished_refresh_type());

  // Should get a greater than zero now.
  base::Time start_time_prev = start_time();
  EXPECT_GT(start_time_prev, base::Time());

  // Do a refresh.
  StartRefresh(REFRESH_TYPE_START_TIME);
  WaitUntilRefreshDone();

  // Start time should not change.
  EXPECT_EQ(start_time(), start_time_prev);
}

// Tests that CPU time can be obtained from SharedSampler.
TEST_F(SharedSamplerTest, CpuTime) {
  StartRefresh(REFRESH_TYPE_CPU_TIME);
  WaitUntilRefreshDone();
  EXPECT_EQ(REFRESH_TYPE_CPU_TIME, finished_refresh_type());

  base::TimeDelta cpu_time_prev = cpu_time();

  // Do a refresh.
  StartRefresh(REFRESH_TYPE_CPU_TIME);
  WaitUntilRefreshDone();

  // CPU time should not decrease.
  EXPECT_GE(cpu_time(), cpu_time_prev);
}

// Verifies that multiple refresh types can be refreshed at the same time.
TEST_F(SharedSamplerTest, MultipleRefreshTypes) {
  StartRefresh(REFRESH_TYPE_IDLE_WAKEUPS | REFRESH_TYPE_PHYSICAL_MEMORY |
               REFRESH_TYPE_START_TIME | REFRESH_TYPE_CPU_TIME);
  WaitUntilRefreshDone();
  EXPECT_EQ(REFRESH_TYPE_IDLE_WAKEUPS | REFRESH_TYPE_PHYSICAL_MEMORY |
                REFRESH_TYPE_START_TIME | REFRESH_TYPE_CPU_TIME,
            finished_refresh_type());
}

static int ReturnZeroThreadProcessInformation(unsigned char* buffer,
                                              int buffer_size) {
  // Calculate the number of bytes required for the structure, and ImageName.
  base::FilePath current_exe;
  CHECK(PathService::Get(base::FILE_EXE, &current_exe));
  base::string16 image_name = current_exe.BaseName().value();

  const int kImageNameBytes = image_name.length() * sizeof(base::char16);
  const int kRequiredBytes = sizeof(SYSTEM_PROCESS_INFORMATION) +
                             kImageNameBytes + sizeof(base::char16);
  if (kRequiredBytes > buffer_size)
    return kRequiredBytes;

  // Create a zero'd structure, so that fields such as thread count will be zero
  // by default.
  // Set process handle and image name, so the SharedSampler will match us.
  memset(buffer, 0, kRequiredBytes);
  SYSTEM_PROCESS_INFORMATION* process_info =
      reinterpret_cast<SYSTEM_PROCESS_INFORMATION*>(buffer);
  process_info->ProcessId = reinterpret_cast<HANDLE>(base::GetCurrentProcId());
  process_info->ImageName.Length = process_info->ImageName.MaximumLength =
      kImageNameBytes;
  process_info->ImageName.Buffer = reinterpret_cast<LPWSTR>(process_info + 1);
  process_info->NumberOfThreads = 0u;
  process_info->WorkingSetPrivateSize = 1024ull;
  buffer += sizeof(SYSTEM_PROCESS_INFORMATION);

  // Copy the image name into place. The earlier memset() provides null
  // termination.
  memcpy(buffer, image_name.data(), kImageNameBytes);

  return kRequiredBytes;
}

// Verifies that the SharedSampler copes with zero-thread processes.
TEST_F(SharedSamplerTest, ZeroThreadProcess) {
  SharedSampler::SetQuerySystemInformationForTest(
      ReturnZeroThreadProcessInformation);

  StartRefresh(REFRESH_TYPE_IDLE_WAKEUPS | REFRESH_TYPE_PHYSICAL_MEMORY |
               REFRESH_TYPE_START_TIME | REFRESH_TYPE_CPU_TIME);
  WaitUntilRefreshDone();
  EXPECT_EQ(REFRESH_TYPE_IDLE_WAKEUPS | REFRESH_TYPE_PHYSICAL_MEMORY |
                REFRESH_TYPE_START_TIME | REFRESH_TYPE_CPU_TIME,
            finished_refresh_type());

  EXPECT_EQ(1024ll, physical_bytes());

  SharedSampler::SetQuerySystemInformationForTest(nullptr);
}

}  // namespace task_manager
