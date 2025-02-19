// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "base/macros.h"
#include "base/test/test_mock_time_task_runner.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/timer/timer.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/base/test/material_design_controller_test_api.h"
#include "ui/compositor/scoped_animation_duration_scale_mode.h"
#include "ui/views/animation/ink_drop.h"
#include "ui/views/animation/ink_drop_host.h"
#include "ui/views/animation/ink_drop_impl.h"
#include "ui/views/animation/ink_drop_state.h"
#include "ui/views/animation/ink_drop_stub.h"
#include "ui/views/animation/test/test_ink_drop_host.h"

namespace views {
namespace test {

// Enumeration of all the different InkDrop types.
enum InkDropType { INK_DROP_STUB, INK_DROP_IMPL };

class InkDropTest : public testing::TestWithParam<testing::tuple<InkDropType>> {
 public:
  InkDropTest();
  ~InkDropTest();

 protected:
  // A dummy InkDropHost required to create an InkDrop.
  TestInkDropHost test_ink_drop_host_;

  // The InkDrop returned by the InkDropFactory test target.
  std::unique_ptr<InkDrop> ink_drop_;

 private:
  // Extracts and returns the InkDropType from the test parameters.
  InkDropType GetInkDropType() const;

  std::unique_ptr<ui::ScopedAnimationDurationScaleMode> zero_duration_mode_;

  // Required by base::Timer's.
  std::unique_ptr<base::ThreadTaskRunnerHandle> thread_task_runner_handle_;

  DISALLOW_COPY_AND_ASSIGN(InkDropTest);
};

InkDropTest::InkDropTest() : ink_drop_(nullptr) {
  zero_duration_mode_.reset(new ui::ScopedAnimationDurationScaleMode(
      ui::ScopedAnimationDurationScaleMode::ZERO_DURATION));

  switch (GetInkDropType()) {
    case INK_DROP_STUB:
      ink_drop_.reset(new InkDropStub());
      break;
    case INK_DROP_IMPL:
      ink_drop_.reset(new InkDropImpl(&test_ink_drop_host_, gfx::Size()));
      // The Timer's used by the InkDropImpl class require a
      // base::ThreadTaskRunnerHandle instance.
      scoped_refptr<base::TestMockTimeTaskRunner> task_runner(
          new base::TestMockTimeTaskRunner);
      thread_task_runner_handle_.reset(
          new base::ThreadTaskRunnerHandle(task_runner));
      break;
  }
}

InkDropTest::~InkDropTest() {}

InkDropType InkDropTest::GetInkDropType() const {
  return testing::get<0>(GetParam());
}

// Note: First argument is optional and intentionally left blank.
// (it's a prefix for the generated test cases)
INSTANTIATE_TEST_CASE_P(,
                        InkDropTest,
                        testing::Values(INK_DROP_STUB, INK_DROP_IMPL));

TEST_P(InkDropTest,
       VerifyInkDropLayersRemovedAfterDestructionWhenRippleIsActive) {
  ink_drop_->AnimateToState(InkDropState::ACTION_PENDING);
  ink_drop_.reset();
  EXPECT_EQ(0, test_ink_drop_host_.num_ink_drop_layers());
}

TEST_P(InkDropTest, StateIsHiddenInitially) {
  EXPECT_EQ(InkDropState::HIDDEN, ink_drop_->GetTargetInkDropState());
}

TEST_P(InkDropTest, TypicalQuickAction) {
  ink_drop_->AnimateToState(InkDropState::ACTION_PENDING);
  ink_drop_->AnimateToState(InkDropState::ACTION_TRIGGERED);
  EXPECT_EQ(InkDropState::HIDDEN, ink_drop_->GetTargetInkDropState());
}

TEST_P(InkDropTest, CancelQuickAction) {
  ink_drop_->AnimateToState(InkDropState::ACTION_PENDING);
  ink_drop_->AnimateToState(InkDropState::HIDDEN);
  EXPECT_EQ(InkDropState::HIDDEN, ink_drop_->GetTargetInkDropState());
}

TEST_P(InkDropTest, TypicalSlowAction) {
  ink_drop_->AnimateToState(InkDropState::ACTION_PENDING);
  ink_drop_->AnimateToState(InkDropState::ALTERNATE_ACTION_PENDING);
  ink_drop_->AnimateToState(InkDropState::ALTERNATE_ACTION_TRIGGERED);
  EXPECT_EQ(InkDropState::HIDDEN, ink_drop_->GetTargetInkDropState());
}

TEST_P(InkDropTest, CancelSlowAction) {
  ink_drop_->AnimateToState(InkDropState::ACTION_PENDING);
  ink_drop_->AnimateToState(InkDropState::ALTERNATE_ACTION_PENDING);
  ink_drop_->AnimateToState(InkDropState::HIDDEN);
  EXPECT_EQ(InkDropState::HIDDEN, ink_drop_->GetTargetInkDropState());
}

TEST_P(InkDropTest, TypicalQuickActivated) {
  ink_drop_->AnimateToState(InkDropState::ACTION_PENDING);
  ink_drop_->AnimateToState(InkDropState::ACTIVATED);
  ink_drop_->AnimateToState(InkDropState::DEACTIVATED);
  EXPECT_EQ(InkDropState::HIDDEN, ink_drop_->GetTargetInkDropState());
}

TEST_P(InkDropTest, TypicalSlowActivated) {
  ink_drop_->AnimateToState(InkDropState::ACTION_PENDING);
  ink_drop_->AnimateToState(InkDropState::ALTERNATE_ACTION_PENDING);
  ink_drop_->AnimateToState(InkDropState::ACTIVATED);
  ink_drop_->AnimateToState(InkDropState::DEACTIVATED);
  EXPECT_EQ(InkDropState::HIDDEN, ink_drop_->GetTargetInkDropState());
}

}  // namespace test
}  // namespace views
