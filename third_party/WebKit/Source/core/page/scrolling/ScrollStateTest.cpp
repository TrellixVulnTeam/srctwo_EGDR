// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/page/scrolling/ScrollState.h"

#include <memory>
#include "core/dom/Document.h"
#include "core/dom/Element.h"
#include "platform/wtf/PtrUtil.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

namespace {

ScrollState* CreateScrollState(double delta_x,
                               double delta_y,
                               bool beginning,
                               bool ending) {
  std::unique_ptr<ScrollStateData> scroll_state_data =
      WTF::MakeUnique<ScrollStateData>();
  scroll_state_data->delta_x = delta_x;
  scroll_state_data->delta_y = delta_y;
  scroll_state_data->is_beginning = beginning;
  scroll_state_data->is_ending = ending;
  return ScrollState::Create(std::move(scroll_state_data));
}

class ScrollStateTest : public ::testing::Test {};

TEST_F(ScrollStateTest, ConsumeDeltaNative) {
  const float kDeltaX = 12.3;
  const float kDeltaY = 3.9;

  const float kDeltaXToConsume = 1.2;
  const float kDeltaYToConsume = 2.3;

  ScrollState* scroll_state = CreateScrollState(kDeltaX, kDeltaY, false, false);
  EXPECT_FLOAT_EQ(kDeltaX, scroll_state->deltaX());
  EXPECT_FLOAT_EQ(kDeltaY, scroll_state->deltaY());
  EXPECT_FALSE(scroll_state->DeltaConsumedForScrollSequence());
  EXPECT_FALSE(scroll_state->FullyConsumed());

  scroll_state->ConsumeDeltaNative(0, 0);
  EXPECT_FLOAT_EQ(kDeltaX, scroll_state->deltaX());
  EXPECT_FLOAT_EQ(kDeltaY, scroll_state->deltaY());
  EXPECT_FALSE(scroll_state->DeltaConsumedForScrollSequence());
  EXPECT_FALSE(scroll_state->FullyConsumed());

  scroll_state->ConsumeDeltaNative(kDeltaXToConsume, 0);
  EXPECT_FLOAT_EQ(kDeltaX - kDeltaXToConsume, scroll_state->deltaX());
  EXPECT_FLOAT_EQ(kDeltaY, scroll_state->deltaY());
  EXPECT_TRUE(scroll_state->DeltaConsumedForScrollSequence());
  EXPECT_FALSE(scroll_state->FullyConsumed());

  scroll_state->ConsumeDeltaNative(0, kDeltaYToConsume);
  EXPECT_FLOAT_EQ(kDeltaX - kDeltaXToConsume, scroll_state->deltaX());
  EXPECT_FLOAT_EQ(kDeltaY - kDeltaYToConsume, scroll_state->deltaY());
  EXPECT_TRUE(scroll_state->DeltaConsumedForScrollSequence());
  EXPECT_FALSE(scroll_state->FullyConsumed());

  scroll_state->ConsumeDeltaNative(scroll_state->deltaX(),
                                   scroll_state->deltaY());
  EXPECT_TRUE(scroll_state->DeltaConsumedForScrollSequence());
  EXPECT_TRUE(scroll_state->FullyConsumed());
}

TEST_F(ScrollStateTest, CurrentNativeScrollingElement) {
  ScrollState* scroll_state = CreateScrollState(0, 0, false, false);
  Element* element =
      Element::Create(QualifiedName::Null(), Document::CreateForTest());
  scroll_state->SetCurrentNativeScrollingElement(element);

  EXPECT_EQ(element, scroll_state->CurrentNativeScrollingElement());
}

TEST_F(ScrollStateTest, FullyConsumed) {
  ScrollState* scroll_state_begin = CreateScrollState(0, 0, true, false);
  ScrollState* scroll_state = CreateScrollState(0, 0, false, false);
  ScrollState* scroll_state_end = CreateScrollState(0, 0, false, true);
  EXPECT_FALSE(scroll_state_begin->FullyConsumed());
  EXPECT_TRUE(scroll_state->FullyConsumed());
  EXPECT_FALSE(scroll_state_end->FullyConsumed());
}

}  // namespace

}  // namespace blink
