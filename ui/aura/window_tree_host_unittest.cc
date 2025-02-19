// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/aura/test/aura_test_base.h"
#include "ui/aura/test/test_screen.h"
#include "ui/aura/window.h"
#include "ui/base/ime/input_method.h"
#include "ui/compositor/layer.h"
#include "ui/events/event_rewriter.h"

namespace {

// Counts number of events observed.
class CounterEventRewriter : public ui::EventRewriter {
 public:
  CounterEventRewriter() : events_seen_(0) {}
  ~CounterEventRewriter() override {}

  int events_seen() const { return events_seen_; }

 private:
  // ui::EventRewriter:
  ui::EventRewriteStatus RewriteEvent(
      const ui::Event& event,
      std::unique_ptr<ui::Event>* new_event) override {
    events_seen_++;
    return ui::EVENT_REWRITE_CONTINUE;
  }

  ui::EventRewriteStatus NextDispatchEvent(
      const ui::Event& last_event,
      std::unique_ptr<ui::Event>* new_event) override {
    return ui::EVENT_REWRITE_CONTINUE;
  }

  int events_seen_;

  DISALLOW_COPY_AND_ASSIGN(CounterEventRewriter);
};

}  // namespace

namespace aura {

using WindowTreeHostTest = test::AuraTestBase;

TEST_F(WindowTreeHostTest, DPIWindowSize) {
  gfx::Rect starting_bounds(0, 0, 800, 600);
  EXPECT_EQ(starting_bounds.size(), host()->compositor()->size());
  EXPECT_EQ(starting_bounds, host()->GetBoundsInPixels());
  EXPECT_EQ(starting_bounds, root_window()->bounds());

  test_screen()->SetDeviceScaleFactor(1.5f);
  EXPECT_EQ(starting_bounds, host()->GetBoundsInPixels());
  // Size should be rounded up after scaling.
  EXPECT_EQ(gfx::Rect(0, 0, 534, 400), root_window()->bounds());

  gfx::Transform transform;
  transform.Translate(0, 1.1f);
  host()->SetRootTransform(transform);
  EXPECT_EQ(gfx::Rect(0, 1, 534, 401), root_window()->bounds());

  gfx::Insets padding(1, 2, 3, 4);
  // Padding is in physical pixels.
  host()->SetOutputSurfacePaddingInPixels(padding);
  gfx::Rect padded_rect = starting_bounds;
  padded_rect.Inset(-padding);
  EXPECT_EQ(padded_rect.size(), host()->compositor()->size());
  EXPECT_EQ(starting_bounds, host()->GetBoundsInPixels());
  EXPECT_EQ(gfx::Rect(1, 1, 534, 401), root_window()->bounds());
  EXPECT_EQ(gfx::Vector2dF(0, 0),
            host()->compositor()->root_layer()->subpixel_position_offset());
}

TEST_F(WindowTreeHostTest, NoRewritesPostIME) {
  CounterEventRewriter event_rewriter;
  host()->AddEventRewriter(&event_rewriter);

  ui::KeyEvent key_event('A', ui::VKEY_A, 0);
  ui::EventDispatchDetails details =
      host()->GetInputMethod()->DispatchKeyEvent(&key_event);
  ASSERT_TRUE(!details.dispatcher_destroyed && !details.target_destroyed);
  EXPECT_EQ(0, event_rewriter.events_seen());

  host()->RemoveEventRewriter(&event_rewriter);
}

TEST_F(WindowTreeHostTest, ColorSpace) {
  EXPECT_EQ(gfx::ColorSpace::CreateSRGB(),
            host()->compositor()->output_color_space());
  test_screen()->SetColorSpace(gfx::ColorSpace::CreateSCRGBLinear());
  EXPECT_EQ(gfx::ColorSpace::CreateSCRGBLinear(),
            host()->compositor()->output_color_space());
}

}  // namespace aura
