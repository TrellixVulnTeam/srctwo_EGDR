// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/touch_selection/touch_selection_controller.h"

#include <vector>

#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/events/test/motion_event_test_utils.h"
#include "ui/touch_selection/touch_selection_controller_test_api.h"

using testing::ElementsAre;
using testing::IsEmpty;
using ui::test::MockMotionEvent;

namespace ui {
namespace {

const int kDefaultTapTimeoutMs = 200;
const float kDefaulTapSlop = 10.f;
const gfx::PointF kIgnoredPoint(0, 0);

class MockTouchHandleDrawable : public TouchHandleDrawable {
 public:
  explicit MockTouchHandleDrawable(bool* contains_point)
      : intersects_rect_(contains_point) {}
  ~MockTouchHandleDrawable() override {}
  void SetEnabled(bool enabled) override {}
  void SetOrientation(ui::TouchHandleOrientation orientation,
                      bool mirror_vertical,
                      bool mirror_horizontal) override {}
  void SetOrigin(const gfx::PointF& origin) override {}
  void SetAlpha(float alpha) override {}
  gfx::RectF GetVisibleBounds() const override {
    return *intersects_rect_ ? gfx::RectF(-1000, -1000, 2000, 2000)
                             : gfx::RectF(-1000, -1000, 0, 0);
  }
  float GetDrawableHorizontalPaddingRatio() const override { return 0; }

 private:
  bool* intersects_rect_;

  DISALLOW_COPY_AND_ASSIGN(MockTouchHandleDrawable);
};

}  // namespace

class TouchSelectionControllerTest : public testing::Test,
                                     public TouchSelectionControllerClient {
 public:
  TouchSelectionControllerTest()
      : caret_moved_(false),
        selection_moved_(false),
        selection_points_swapped_(false),
        needs_animate_(false),
        animation_enabled_(true),
        dragging_enabled_(false) {}

  ~TouchSelectionControllerTest() override {}

  // testing::Test implementation.

  void SetUp() override {
    controller_.reset(new TouchSelectionController(this, DefaultConfig()));
  }

  void TearDown() override { controller_.reset(); }

  // TouchSelectionControllerClient implementation.

  bool SupportsAnimation() const override { return animation_enabled_; }

  void SetNeedsAnimate() override { needs_animate_ = true; }

  void MoveCaret(const gfx::PointF& position) override {
    caret_moved_ = true;
    caret_position_ = position;
  }

  void SelectBetweenCoordinates(const gfx::PointF& base,
                                const gfx::PointF& extent) override {
    if (base == selection_end_ && extent == selection_start_)
      selection_points_swapped_ = true;

    selection_start_ = base;
    selection_end_ = extent;
  }

  void MoveRangeSelectionExtent(const gfx::PointF& extent) override {
    selection_moved_ = true;
    selection_end_ = extent;
  }

  void OnSelectionEvent(SelectionEventType event) override {
    events_.push_back(event);
    last_event_start_ = controller_->GetStartPosition();
    last_event_end_ = controller_->GetEndPosition();
    last_event_bounds_rect_ = controller_->GetRectBetweenBounds();
  }

  std::unique_ptr<TouchHandleDrawable> CreateDrawable() override {
    return base::MakeUnique<MockTouchHandleDrawable>(&dragging_enabled_);
  }

  void EnableLongPressDragSelection() {
    TouchSelectionController::Config config = DefaultConfig();
    config.enable_longpress_drag_selection = true;
    controller_.reset(new TouchSelectionController(this, config));
  }

  void SetAnimationEnabled(bool enabled) { animation_enabled_ = enabled; }
  void SetDraggingEnabled(bool enabled) { dragging_enabled_ = enabled; }

  void ClearSelection() {
    controller_->OnSelectionBoundsChanged(gfx::SelectionBound(),
                                          gfx::SelectionBound());
  }

  void ClearInsertion() { ClearSelection(); }

  void ChangeInsertion(const gfx::RectF& rect, bool visible) {
    gfx::SelectionBound bound;
    bound.set_type(gfx::SelectionBound::CENTER);
    bound.SetEdge(rect.origin(), rect.bottom_left());
    bound.set_visible(visible);
    controller_->OnSelectionBoundsChanged(bound, bound);
  }

  void ChangeSelection(const gfx::RectF& start_rect,
                       bool start_visible,
                       const gfx::RectF& end_rect,
                       bool end_visible) {
    gfx::SelectionBound start_bound, end_bound;
    start_bound.set_type(gfx::SelectionBound::LEFT);
    end_bound.set_type(gfx::SelectionBound::RIGHT);
    start_bound.SetEdge(start_rect.origin(), start_rect.bottom_left());
    end_bound.SetEdge(end_rect.origin(), end_rect.bottom_left());
    start_bound.set_visible(start_visible);
    end_bound.set_visible(end_visible);
    controller_->OnSelectionBoundsChanged(start_bound, end_bound);
  }

  void OnLongPressEvent() {
    controller().HandleLongPressEvent(base::TimeTicks(),
                                          kIgnoredPoint);
  }

  void OnTapEvent() {
    controller().HandleTapEvent(kIgnoredPoint, 1);
  }

  void OnDoubleTapEvent() {
    controller().HandleTapEvent(kIgnoredPoint, 2);
  }

  void Animate() {
    base::TimeTicks now = base::TimeTicks::Now();
    while (needs_animate_) {
      needs_animate_ = controller_->Animate(now);
      now += base::TimeDelta::FromMilliseconds(16);
    }
  }

  bool GetAndResetNeedsAnimate() {
    bool needs_animate = needs_animate_;
    Animate();
    return needs_animate;
  }

  bool GetAndResetCaretMoved() {
    bool moved = caret_moved_;
    caret_moved_ = false;
    return moved;
  }

  bool GetAndResetSelectionMoved() {
    bool moved = selection_moved_;
    selection_moved_ = false;
    return moved;
  }

  bool GetAndResetSelectionPointsSwapped() {
    bool swapped = selection_points_swapped_;
    selection_points_swapped_ = false;
    return swapped;
  }

  const gfx::PointF& GetLastCaretPosition() const { return caret_position_; }
  const gfx::PointF& GetLastSelectionStart() const { return selection_start_; }
  const gfx::PointF& GetLastSelectionEnd() const { return selection_end_; }
  const gfx::PointF& GetLastEventStart() const { return last_event_start_; }
  const gfx::PointF& GetLastEventEnd() const { return last_event_end_; }
  const gfx::RectF& GetLastEventBoundsRect() const {
    return last_event_bounds_rect_;
  }

  std::vector<SelectionEventType> GetAndResetEvents() {
    std::vector<SelectionEventType> events;
    events.swap(events_);
    return events;
  }

  TouchSelectionController& controller() { return *controller_; }

 private:
  TouchSelectionController::Config DefaultConfig() {
    // |enable_longpress_drag_selection| is set to false by default, and should
    // be overriden for explicit testing.
    TouchSelectionController::Config config;
    config.max_tap_duration =
        base::TimeDelta::FromMilliseconds(kDefaultTapTimeoutMs);
    config.tap_slop = kDefaulTapSlop;
    config.enable_longpress_drag_selection = false;
    return config;
  }

  gfx::PointF last_event_start_;
  gfx::PointF last_event_end_;
  gfx::PointF caret_position_;
  gfx::PointF selection_start_;
  gfx::PointF selection_end_;
  gfx::RectF last_event_bounds_rect_;
  std::vector<SelectionEventType> events_;
  bool caret_moved_;
  bool selection_moved_;
  bool selection_points_swapped_;
  bool needs_animate_;
  bool animation_enabled_;
  bool dragging_enabled_;
  std::unique_ptr<TouchSelectionController> controller_;

  DISALLOW_COPY_AND_ASSIGN(TouchSelectionControllerTest);
};

TEST_F(TouchSelectionControllerTest, InsertionBasic) {
  gfx::RectF insertion_rect(5, 5, 0, 10);
  bool visible = true;

  OnTapEvent();
  ChangeInsertion(insertion_rect, visible);
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(INSERTION_HANDLE_SHOWN));
  EXPECT_EQ(insertion_rect.bottom_left(), GetLastEventStart());

  insertion_rect.Offset(1, 0);
  ChangeInsertion(insertion_rect, visible);
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(INSERTION_HANDLE_MOVED));
  EXPECT_EQ(insertion_rect.bottom_left(), GetLastEventStart());

  insertion_rect.Offset(0, 1);
  ChangeInsertion(insertion_rect, visible);
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(INSERTION_HANDLE_MOVED));
  EXPECT_EQ(insertion_rect.bottom_left(), GetLastEventStart());

  OnTapEvent();
  insertion_rect.Offset(1, 0);
  ChangeInsertion(insertion_rect, visible);
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(INSERTION_HANDLE_SHOWN));
  EXPECT_EQ(insertion_rect.bottom_left(), GetLastEventStart());

  ClearInsertion();
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(INSERTION_HANDLE_CLEARED));
}

TEST_F(TouchSelectionControllerTest, InsertionToSelectionTransition) {
  OnLongPressEvent();

  gfx::RectF start_rect(5, 5, 0, 10);
  gfx::RectF end_rect(50, 5, 0, 10);
  bool visible = true;

  ChangeInsertion(start_rect, visible);
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(INSERTION_HANDLE_SHOWN));
  EXPECT_EQ(start_rect.bottom_left(), GetLastEventStart());

  ChangeSelection(start_rect, visible, end_rect, visible);
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(INSERTION_HANDLE_CLEARED, SELECTION_HANDLES_SHOWN));
  EXPECT_EQ(start_rect.bottom_left(), GetLastEventStart());

  ChangeInsertion(end_rect, visible);
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(SELECTION_HANDLES_CLEARED, INSERTION_HANDLE_SHOWN));
  EXPECT_EQ(end_rect.bottom_left(), GetLastEventStart());

  ClearInsertion();
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(INSERTION_HANDLE_CLEARED));

  OnTapEvent();
  ChangeInsertion(end_rect, visible);
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(INSERTION_HANDLE_SHOWN));
  EXPECT_EQ(end_rect.bottom_left(), GetLastEventStart());
}

TEST_F(TouchSelectionControllerTest, InsertionDragged) {
  base::TimeTicks event_time = base::TimeTicks::Now();
  OnTapEvent();

  // The touch sequence should not be handled if insertion is not active.
  MockMotionEvent event(MockMotionEvent::ACTION_DOWN, event_time, 0, 0);
  EXPECT_FALSE(controller().WillHandleTouchEvent(event));

  float line_height = 10.f;
  gfx::RectF start_rect(10, 0, 0, line_height);
  bool visible = true;
  ChangeInsertion(start_rect, visible);
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(INSERTION_HANDLE_SHOWN));
  EXPECT_EQ(start_rect.bottom_left(), GetLastEventStart());

  // The touch sequence should be handled only if the drawable reports a hit.
  EXPECT_FALSE(controller().WillHandleTouchEvent(event));
  SetDraggingEnabled(true);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_FALSE(GetAndResetCaretMoved());
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(INSERTION_HANDLE_DRAG_STARTED));

  // The MoveCaret() result should reflect the movement.
  // The reported position is offset from the center of |start_rect|.
  gfx::PointF start_offset = start_rect.CenterPoint();
  event = MockMotionEvent(MockMotionEvent::ACTION_MOVE, event_time, 0, 5);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_TRUE(GetAndResetCaretMoved());
  EXPECT_EQ(start_offset + gfx::Vector2dF(0, 5), GetLastCaretPosition());

  event = MockMotionEvent(MockMotionEvent::ACTION_MOVE, event_time, 5, 5);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_TRUE(GetAndResetCaretMoved());
  EXPECT_EQ(start_offset + gfx::Vector2dF(5, 5), GetLastCaretPosition());

  event = MockMotionEvent(MockMotionEvent::ACTION_MOVE, event_time, 10, 10);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_TRUE(GetAndResetCaretMoved());
  EXPECT_EQ(start_offset + gfx::Vector2dF(10, 10), GetLastCaretPosition());

  event = MockMotionEvent(MockMotionEvent::ACTION_UP, event_time, 10, 5);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_FALSE(GetAndResetCaretMoved());
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(INSERTION_HANDLE_DRAG_STOPPED));

  // Following ACTION_DOWN should not be consumed if it does not start handle
  // dragging.
  SetDraggingEnabled(false);
  event = MockMotionEvent(MotionEvent::ACTION_DOWN, event_time, 0, 0);
  EXPECT_FALSE(controller().WillHandleTouchEvent(event));
}

TEST_F(TouchSelectionControllerTest, InsertionDeactivatedWhileDragging) {
  base::TimeTicks event_time = base::TimeTicks::Now();
  OnTapEvent();

  float line_height = 10.f;
  gfx::RectF start_rect(10, 0, 0, line_height);
  bool visible = true;
  ChangeInsertion(start_rect, visible);
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(INSERTION_HANDLE_SHOWN));
  EXPECT_EQ(start_rect.bottom_left(), GetLastEventStart());

  // Enable dragging so that the following ACTION_DOWN starts handle dragging.
  SetDraggingEnabled(true);

  // Touch down to start dragging.
  MockMotionEvent event(MockMotionEvent::ACTION_DOWN, event_time, 0, 0);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_FALSE(GetAndResetCaretMoved());
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(INSERTION_HANDLE_DRAG_STARTED));

  // Move the handle.
  gfx::PointF start_offset = start_rect.CenterPoint();
  event = MockMotionEvent(MockMotionEvent::ACTION_MOVE, event_time, 0, 5);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_TRUE(GetAndResetCaretMoved());
  EXPECT_EQ(start_offset + gfx::Vector2dF(0, 5), GetLastCaretPosition());

  // Deactivate touch selection to end dragging.
  controller().HideAndDisallowShowingAutomatically();
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(INSERTION_HANDLE_DRAG_STOPPED,
                                               INSERTION_HANDLE_CLEARED));

  // Move the finger. There is no handle to move, so the cursor is not moved;
  // but, the event is still consumed because the touch down that started the
  // touch sequence was consumed.
  event = MockMotionEvent(MockMotionEvent::ACTION_MOVE, event_time, 5, 5);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_FALSE(GetAndResetCaretMoved());
  EXPECT_EQ(start_offset + gfx::Vector2dF(0, 5), GetLastCaretPosition());

  // Lift the finger to end the touch sequence.
  event = MockMotionEvent(MockMotionEvent::ACTION_UP, event_time, 5, 5);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_FALSE(GetAndResetCaretMoved());
  EXPECT_THAT(GetAndResetEvents(), IsEmpty());

  // Following ACTION_DOWN should not be consumed if it does not start handle
  // dragging.
  SetDraggingEnabled(false);
  event = MockMotionEvent(MotionEvent::ACTION_DOWN, event_time, 0, 0);
  EXPECT_FALSE(controller().WillHandleTouchEvent(event));
}

TEST_F(TouchSelectionControllerTest, InsertionTapped) {
  base::TimeTicks event_time = base::TimeTicks::Now();
  OnTapEvent();
  SetDraggingEnabled(true);

  gfx::RectF start_rect(10, 0, 0, 10);
  bool visible = true;
  ChangeInsertion(start_rect, visible);
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(INSERTION_HANDLE_SHOWN));

  MockMotionEvent event(MockMotionEvent::ACTION_DOWN, event_time, 0, 0);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(INSERTION_HANDLE_DRAG_STARTED));

  event = MockMotionEvent(MockMotionEvent::ACTION_UP, event_time, 0, 0);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(INSERTION_HANDLE_TAPPED,
                                               INSERTION_HANDLE_DRAG_STOPPED));

  // Reset the insertion.
  ClearInsertion();
  OnTapEvent();
  ChangeInsertion(start_rect, visible);
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(INSERTION_HANDLE_CLEARED, INSERTION_HANDLE_SHOWN));

  // No tap should be signalled if the time between DOWN and UP was too long.
  event = MockMotionEvent(MockMotionEvent::ACTION_DOWN, event_time, 0, 0);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  event = MockMotionEvent(MockMotionEvent::ACTION_UP,
                          event_time + base::TimeDelta::FromSeconds(1),
                          0,
                          0);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(INSERTION_HANDLE_DRAG_STARTED,
                                               INSERTION_HANDLE_DRAG_STOPPED));

  // Reset the insertion.
  ClearInsertion();
  OnTapEvent();
  ChangeInsertion(start_rect, visible);
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(INSERTION_HANDLE_CLEARED, INSERTION_HANDLE_SHOWN));

  // No tap should be signalled if the drag was too long.
  event = MockMotionEvent(MockMotionEvent::ACTION_DOWN, event_time, 0, 0);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  event = MockMotionEvent(MockMotionEvent::ACTION_MOVE, event_time, 100, 0);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  event = MockMotionEvent(MockMotionEvent::ACTION_UP, event_time, 100, 0);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(INSERTION_HANDLE_DRAG_STARTED,
                                               INSERTION_HANDLE_DRAG_STOPPED));

  // Reset the insertion.
  ClearInsertion();
  OnTapEvent();
  ChangeInsertion(start_rect, visible);
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(INSERTION_HANDLE_CLEARED, INSERTION_HANDLE_SHOWN));

  // No tap should be signalled if the touch sequence is cancelled.
  event = MockMotionEvent(MockMotionEvent::ACTION_DOWN, event_time, 0, 0);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  event = MockMotionEvent(MockMotionEvent::ACTION_CANCEL, event_time, 0, 0);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(INSERTION_HANDLE_DRAG_STARTED,
                                               INSERTION_HANDLE_DRAG_STOPPED));
}

TEST_F(TouchSelectionControllerTest, SelectionBasic) {
  gfx::RectF start_rect(5, 5, 0, 10);
  gfx::RectF end_rect(50, 5, 0, 10);
  bool visible = true;

  OnLongPressEvent();
  ChangeSelection(start_rect, visible, end_rect, visible);
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(SELECTION_HANDLES_SHOWN));
  EXPECT_EQ(start_rect.bottom_left(), GetLastEventStart());

  start_rect.Offset(1, 0);
  ChangeSelection(start_rect, visible, end_rect, visible);
  // Selection movement does not currently trigger a separate event.
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLES_MOVED));
  EXPECT_EQ(start_rect.bottom_left(), GetLastEventStart());
  EXPECT_EQ(end_rect.bottom_left(), GetLastEventEnd());

  ClearSelection();
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(SELECTION_HANDLES_CLEARED));
}

TEST_F(TouchSelectionControllerTest, SelectionAllowedByDoubleTap) {
  gfx::RectF start_rect(5, 5, 0, 10);
  gfx::RectF end_rect(50, 5, 0, 10);
  bool visible = true;

  OnDoubleTapEvent();
  ChangeSelection(start_rect, visible, end_rect, visible);
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(SELECTION_HANDLES_SHOWN));
  EXPECT_EQ(start_rect.bottom_left(), GetLastEventStart());
}

TEST_F(TouchSelectionControllerTest, SelectionAllowedByDoubleTapOnEditable) {
  gfx::RectF start_rect(5, 5, 0, 10);
  gfx::RectF end_rect(50, 5, 0, 10);
  bool visible = true;

  // If the user double tap selects text in an editable region, the first tap
  // will register insertion and the second tap selection.
  OnTapEvent();
  ChangeInsertion(start_rect, visible);
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(INSERTION_HANDLE_SHOWN));

  OnDoubleTapEvent();
  ChangeSelection(start_rect, visible, end_rect, visible);
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(INSERTION_HANDLE_CLEARED, SELECTION_HANDLES_SHOWN));
}

TEST_F(TouchSelectionControllerTest, SelectionAllowsEmptyUpdateAfterLongPress) {
  gfx::RectF start_rect(5, 5, 0, 10);
  gfx::RectF end_rect(50, 5, 0, 10);
  bool visible = true;

  OnLongPressEvent();
  EXPECT_THAT(GetAndResetEvents(), IsEmpty());

  // There may be several empty updates after a longpress due to the
  // asynchronous response. These empty updates should not prevent the selection
  // handles from (eventually) activating.
  ClearSelection();
  EXPECT_THAT(GetAndResetEvents(), IsEmpty());

  ClearSelection();
  EXPECT_THAT(GetAndResetEvents(), IsEmpty());

  ChangeSelection(start_rect, visible, end_rect, visible);
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(SELECTION_HANDLES_SHOWN));
}

TEST_F(TouchSelectionControllerTest, SelectionRepeatedLongPress) {
  gfx::RectF start_rect(5, 5, 0, 10);
  gfx::RectF end_rect(50, 5, 0, 10);
  bool visible = true;

  OnLongPressEvent();
  ChangeSelection(start_rect, visible, end_rect, visible);
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(SELECTION_HANDLES_SHOWN));
  EXPECT_EQ(start_rect.bottom_left(), GetLastEventStart());
  EXPECT_EQ(end_rect.bottom_left(), GetLastEventEnd());

  // A long press triggering a new selection should re-send the
  // SELECTION_HANDLES_SHOWN
  // event notification.
  start_rect.Offset(10, 10);
  OnLongPressEvent();
  ChangeSelection(start_rect, visible, end_rect, visible);
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLES_SHOWN));
  EXPECT_EQ(start_rect.bottom_left(), GetLastEventStart());
  EXPECT_EQ(end_rect.bottom_left(), GetLastEventEnd());
}

TEST_F(TouchSelectionControllerTest, SelectionDragged) {
  base::TimeTicks event_time = base::TimeTicks::Now();
  OnLongPressEvent();

  // The touch sequence should not be handled if selection is not active.
  MockMotionEvent event(MockMotionEvent::ACTION_DOWN, event_time, 0, 0);
  EXPECT_FALSE(controller().WillHandleTouchEvent(event));

  float line_height = 10.f;
  gfx::RectF start_rect(0, 0, 0, line_height);
  gfx::RectF end_rect(50, 0, 0, line_height);
  bool visible = true;
  ChangeSelection(start_rect, visible, end_rect, visible);
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(SELECTION_HANDLES_SHOWN));
  EXPECT_EQ(start_rect.bottom_left(), GetLastEventStart());

  // The touch sequence should be handled only if the drawable reports a hit.
  EXPECT_FALSE(controller().WillHandleTouchEvent(event));
  SetDraggingEnabled(true);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_FALSE(GetAndResetSelectionMoved());

  // The SelectBetweenCoordinates() result should reflect the movement. Note
  // that the start coordinate will always reflect the "fixed" handle's
  // position, in this case the position from |end_rect|.
  // Note that the reported position is offset from the center of the
  // input rects (i.e., the middle of the corresponding text line).
  gfx::PointF fixed_offset = end_rect.CenterPoint();
  gfx::PointF start_offset = start_rect.CenterPoint();
  event = MockMotionEvent(MockMotionEvent::ACTION_MOVE, event_time, 0, 5);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLE_DRAG_STARTED));
  EXPECT_TRUE(GetAndResetSelectionMoved());
  EXPECT_EQ(fixed_offset, GetLastSelectionStart());
  EXPECT_EQ(start_offset + gfx::Vector2dF(0, 5), GetLastSelectionEnd());

  event = MockMotionEvent(MockMotionEvent::ACTION_MOVE, event_time, 5, 5);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_TRUE(GetAndResetSelectionMoved());
  EXPECT_EQ(fixed_offset, GetLastSelectionStart());
  EXPECT_EQ(start_offset + gfx::Vector2dF(5, 5), GetLastSelectionEnd());

  event = MockMotionEvent(MockMotionEvent::ACTION_MOVE, event_time, 10, 5);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_TRUE(GetAndResetSelectionMoved());
  EXPECT_EQ(fixed_offset, GetLastSelectionStart());
  EXPECT_EQ(start_offset + gfx::Vector2dF(10, 5), GetLastSelectionEnd());

  event = MockMotionEvent(MockMotionEvent::ACTION_UP, event_time, 10, 5);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLE_DRAG_STOPPED));
  EXPECT_FALSE(GetAndResetSelectionMoved());

  // Following ACTION_DOWN should not be consumed if it does not start handle
  // dragging.
  SetDraggingEnabled(false);
  event = MockMotionEvent(MotionEvent::ACTION_DOWN, event_time, 0, 0);
  EXPECT_FALSE(controller().WillHandleTouchEvent(event));
}

TEST_F(TouchSelectionControllerTest, SelectionDraggedWithOverlap) {
  base::TimeTicks event_time = base::TimeTicks::Now();
  OnLongPressEvent();

  float line_height = 10.f;
  gfx::RectF start_rect(0, 0, 0, line_height);
  gfx::RectF end_rect(50, 0, 0, line_height);
  bool visible = true;
  ChangeSelection(start_rect, visible, end_rect, visible);
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(SELECTION_HANDLES_SHOWN));
  EXPECT_EQ(start_rect.bottom_left(), GetLastEventStart());

  // The ACTION_DOWN should lock to the closest handle.
  gfx::PointF end_offset = end_rect.CenterPoint();
  gfx::PointF fixed_offset = start_rect.CenterPoint();
  float touch_down_x = (end_offset.x() + fixed_offset.x()) / 2 + 1.f;
  MockMotionEvent event(
      MockMotionEvent::ACTION_DOWN, event_time, touch_down_x, 0);
  SetDraggingEnabled(true);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLE_DRAG_STARTED));
  EXPECT_FALSE(GetAndResetSelectionMoved());

  // Even though the ACTION_MOVE is over the start handle, it should continue
  // targetting the end handle that consumed the ACTION_DOWN.
  event = MockMotionEvent(MockMotionEvent::ACTION_MOVE, event_time, 0, 0);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_TRUE(GetAndResetSelectionMoved());
  EXPECT_EQ(fixed_offset, GetLastSelectionStart());
  EXPECT_EQ(end_offset - gfx::Vector2dF(touch_down_x, 0),
            GetLastSelectionEnd());

  event = MockMotionEvent(MockMotionEvent::ACTION_UP, event_time, 0, 0);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLE_DRAG_STOPPED));
  EXPECT_FALSE(GetAndResetSelectionMoved());
}

TEST_F(TouchSelectionControllerTest, SelectionDraggedToSwitchBaseAndExtent) {
  base::TimeTicks event_time = base::TimeTicks::Now();
  OnLongPressEvent();

  float line_height = 10.f;
  gfx::RectF start_rect(50, line_height, 0, line_height);
  gfx::RectF end_rect(100, line_height, 0, line_height);
  bool visible = true;
  ChangeSelection(start_rect, visible, end_rect, visible);
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(SELECTION_HANDLES_SHOWN));
  EXPECT_EQ(start_rect.bottom_left(), GetLastEventStart());

  SetDraggingEnabled(true);

  // Move the extent, not triggering a swap of points.
  MockMotionEvent event(MockMotionEvent::ACTION_DOWN, event_time,
                        end_rect.x(), end_rect.bottom());
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_FALSE(GetAndResetSelectionMoved());
  EXPECT_FALSE(GetAndResetSelectionPointsSwapped());

  gfx::PointF base_offset = start_rect.CenterPoint();
  gfx::PointF extent_offset = end_rect.CenterPoint();
  event = MockMotionEvent(MockMotionEvent::ACTION_MOVE, event_time,
                          end_rect.x(), end_rect.bottom() + 5);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLE_DRAG_STARTED));
  EXPECT_TRUE(GetAndResetSelectionMoved());
  EXPECT_FALSE(GetAndResetSelectionPointsSwapped());
  EXPECT_EQ(base_offset, GetLastSelectionStart());
  EXPECT_EQ(extent_offset + gfx::Vector2dF(0, 5), GetLastSelectionEnd());

  event = MockMotionEvent(MockMotionEvent::ACTION_UP, event_time, 10, 5);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLE_DRAG_STOPPED));
  EXPECT_FALSE(GetAndResetSelectionMoved());

  end_rect += gfx::Vector2dF(0, 5);
  ChangeSelection(start_rect, visible, end_rect, visible);
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLES_MOVED));

  // Move the base, triggering a swap of points.
  event = MockMotionEvent(MockMotionEvent::ACTION_DOWN, event_time,
                          start_rect.x(), start_rect.bottom());
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_FALSE(GetAndResetSelectionMoved());
  EXPECT_TRUE(GetAndResetSelectionPointsSwapped());

  base_offset = end_rect.CenterPoint();
  extent_offset = start_rect.CenterPoint();
  event = MockMotionEvent(MockMotionEvent::ACTION_MOVE, event_time,
                          start_rect.x(), start_rect.bottom() + 5);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLE_DRAG_STARTED));
  EXPECT_TRUE(GetAndResetSelectionMoved());
  EXPECT_FALSE(GetAndResetSelectionPointsSwapped());
  EXPECT_EQ(base_offset, GetLastSelectionStart());
  EXPECT_EQ(extent_offset + gfx::Vector2dF(0, 5), GetLastSelectionEnd());

  event = MockMotionEvent(MockMotionEvent::ACTION_UP, event_time, 10, 5);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLE_DRAG_STOPPED));
  EXPECT_FALSE(GetAndResetSelectionMoved());

  start_rect += gfx::Vector2dF(0, 5);
  ChangeSelection(start_rect, visible, end_rect, visible);
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLES_MOVED));

  // Move the same point again, not triggering a swap of points.
  event = MockMotionEvent(MockMotionEvent::ACTION_DOWN, event_time,
                          start_rect.x(), start_rect.bottom());
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_FALSE(GetAndResetSelectionMoved());
  EXPECT_FALSE(GetAndResetSelectionPointsSwapped());

  base_offset = end_rect.CenterPoint();
  extent_offset = start_rect.CenterPoint();
  event = MockMotionEvent(MockMotionEvent::ACTION_MOVE, event_time,
                          start_rect.x(), start_rect.bottom() + 5);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLE_DRAG_STARTED));
  EXPECT_TRUE(GetAndResetSelectionMoved());
  EXPECT_FALSE(GetAndResetSelectionPointsSwapped());
  EXPECT_EQ(base_offset, GetLastSelectionStart());
  EXPECT_EQ(extent_offset + gfx::Vector2dF(0, 5), GetLastSelectionEnd());

  event = MockMotionEvent(MockMotionEvent::ACTION_UP, event_time, 10, 5);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLE_DRAG_STOPPED));
  EXPECT_FALSE(GetAndResetSelectionMoved());

  start_rect += gfx::Vector2dF(0, 5);
  ChangeSelection(start_rect, visible, end_rect, visible);
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLES_MOVED));

  // Move the base, triggering a swap of points.
  event = MockMotionEvent(MockMotionEvent::ACTION_DOWN, event_time,
                          end_rect.x(), end_rect.bottom());
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_FALSE(GetAndResetSelectionMoved());
  EXPECT_TRUE(GetAndResetSelectionPointsSwapped());

  base_offset = start_rect.CenterPoint();
  extent_offset = end_rect.CenterPoint();
  event = MockMotionEvent(MockMotionEvent::ACTION_MOVE, event_time,
                          end_rect.x(), end_rect.bottom() + 5);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLE_DRAG_STARTED));
  EXPECT_TRUE(GetAndResetSelectionMoved());
  EXPECT_FALSE(GetAndResetSelectionPointsSwapped());
  EXPECT_EQ(base_offset, GetLastSelectionStart());
  EXPECT_EQ(extent_offset + gfx::Vector2dF(0, 5), GetLastSelectionEnd());

  event = MockMotionEvent(MockMotionEvent::ACTION_UP, event_time, 10, 5);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLE_DRAG_STOPPED));
  EXPECT_FALSE(GetAndResetSelectionMoved());
}

TEST_F(TouchSelectionControllerTest, SelectionDragExtremeLineSize) {
  base::TimeTicks event_time = base::TimeTicks::Now();
  OnLongPressEvent();

  float small_line_height = 1.f;
  float large_line_height = 50.f;
  gfx::RectF small_line_rect(0, 0, 0, small_line_height);
  gfx::RectF large_line_rect(50, 50, 0, large_line_height);
  bool visible = true;
  ChangeSelection(small_line_rect, visible, large_line_rect, visible);
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(SELECTION_HANDLES_SHOWN));
  EXPECT_EQ(small_line_rect.bottom_left(), GetLastEventStart());

  // Start dragging the handle on the small line.
  MockMotionEvent event(MockMotionEvent::ACTION_DOWN, event_time,
                        small_line_rect.x(), small_line_rect.y());
  SetDraggingEnabled(true);
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLE_DRAG_STARTED));
  // The drag coordinate for large lines should be capped to a reasonable
  // offset, allowing seamless transition to neighboring lines with different
  // sizes. The drag coordinate for small lines should have an offset
  // commensurate with the small line size.
  EXPECT_EQ(large_line_rect.bottom_left() - gfx::Vector2dF(0, 8.f),
            GetLastSelectionStart());
  EXPECT_EQ(small_line_rect.CenterPoint(), GetLastSelectionEnd());

  small_line_rect += gfx::Vector2dF(25.f, 0);
  event = MockMotionEvent(MockMotionEvent::ACTION_MOVE, event_time,
                          small_line_rect.x(), small_line_rect.y());
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_TRUE(GetAndResetSelectionMoved());
  EXPECT_EQ(small_line_rect.CenterPoint(), GetLastSelectionEnd());
}

TEST_F(TouchSelectionControllerTest, Animation) {
  OnTapEvent();

  gfx::RectF insertion_rect(5, 5, 0, 10);

  bool visible = true;
  ChangeInsertion(insertion_rect, visible);
  EXPECT_FALSE(GetAndResetNeedsAnimate());

  visible = false;
  ChangeInsertion(insertion_rect, visible);
  EXPECT_TRUE(GetAndResetNeedsAnimate());

  visible = true;
  ChangeInsertion(insertion_rect, visible);
  EXPECT_TRUE(GetAndResetNeedsAnimate());

  // If the handles are explicity hidden, no animation should be triggered.
  controller().HideAndDisallowShowingAutomatically();
  EXPECT_FALSE(GetAndResetNeedsAnimate());

  // If the client doesn't support animation, no animation should be triggered.
  SetAnimationEnabled(false);
  OnTapEvent();
  visible = true;
  ChangeInsertion(insertion_rect, visible);
  EXPECT_FALSE(GetAndResetNeedsAnimate());
}

TEST_F(TouchSelectionControllerTest, TemporarilyHidden) {
  TouchSelectionControllerTestApi test_controller(&controller());

  OnTapEvent();

  gfx::RectF insertion_rect(5, 5, 0, 10);

  bool visible = true;
  ChangeInsertion(insertion_rect, visible);
  EXPECT_FALSE(GetAndResetNeedsAnimate());
  EXPECT_TRUE(test_controller.GetStartVisible());
  EXPECT_TRUE(test_controller.GetEndVisible());

  controller().SetTemporarilyHidden(true);
  EXPECT_TRUE(GetAndResetNeedsAnimate());
  EXPECT_FALSE(test_controller.GetStartVisible());
  EXPECT_FALSE(test_controller.GetEndVisible());

  EXPECT_EQ(0.f, test_controller.GetStartAlpha());
  EXPECT_EQ(0.f, test_controller.GetEndAlpha());

  visible = false;
  ChangeInsertion(insertion_rect, visible);
  EXPECT_FALSE(GetAndResetNeedsAnimate());
  EXPECT_FALSE(test_controller.GetStartVisible());
  EXPECT_EQ(0.f, test_controller.GetStartAlpha());

  visible = true;
  ChangeInsertion(insertion_rect, visible);
  EXPECT_FALSE(GetAndResetNeedsAnimate());
  EXPECT_FALSE(test_controller.GetStartVisible());
  EXPECT_EQ(0.f, test_controller.GetStartAlpha());

  controller().SetTemporarilyHidden(false);
  EXPECT_TRUE(GetAndResetNeedsAnimate());
  EXPECT_TRUE(test_controller.GetStartVisible());
}

TEST_F(TouchSelectionControllerTest, SelectionClearOnTap) {
  gfx::RectF start_rect(5, 5, 0, 10);
  gfx::RectF end_rect(50, 5, 0, 10);
  bool visible = true;

  OnLongPressEvent();
  ChangeSelection(start_rect, visible, end_rect, visible);
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(SELECTION_HANDLES_SHOWN));

  // Selection should not be cleared if the selection bounds have not changed.
  OnTapEvent();
  EXPECT_THAT(GetAndResetEvents(), IsEmpty());
  EXPECT_EQ(start_rect.bottom_left(), GetLastEventStart());

  OnTapEvent();
  ClearSelection();
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(SELECTION_HANDLES_CLEARED));
}

TEST_F(TouchSelectionControllerTest, LongPressDrag) {
  EnableLongPressDragSelection();
  TouchSelectionControllerTestApi test_controller(&controller());

  gfx::RectF start_rect(-50, 0, 0, 10);
  gfx::RectF end_rect(50, 0, 0, 10);
  bool visible = true;

  // Start a touch sequence.
  MockMotionEvent event;
  EXPECT_FALSE(controller().WillHandleTouchEvent(event.PressPoint(0, 0)));

  // Activate a longpress-triggered selection.
  OnLongPressEvent();
  ChangeSelection(start_rect, visible, end_rect, visible);
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(SELECTION_HANDLES_SHOWN));
  EXPECT_EQ(start_rect.bottom_left(), GetLastEventStart());

  // The handles should remain invisible while the touch release and longpress
  // drag gesture are pending.
  EXPECT_FALSE(test_controller.GetStartVisible());
  EXPECT_FALSE(test_controller.GetEndVisible());
  EXPECT_EQ(0.f, test_controller.GetStartAlpha());
  EXPECT_EQ(0.f, test_controller.GetEndAlpha());

  // The selection coordinates should reflect the drag movement.
  gfx::PointF fixed_offset = start_rect.CenterPoint();
  gfx::PointF end_offset = end_rect.CenterPoint();
  EXPECT_TRUE(controller().WillHandleTouchEvent(event.MovePoint(0, 0, 0)));
  EXPECT_THAT(GetAndResetEvents(), IsEmpty());

  EXPECT_TRUE(
      controller().WillHandleTouchEvent(event.MovePoint(0, 0, kDefaulTapSlop)));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLE_DRAG_STARTED));
  EXPECT_EQ(fixed_offset, GetLastSelectionStart());
  EXPECT_EQ(end_offset, GetLastSelectionEnd());

  // Movement after the start of drag will be relative to the moved endpoint.
  EXPECT_TRUE(controller().WillHandleTouchEvent(
      event.MovePoint(0, 0, 2 * kDefaulTapSlop)));
  EXPECT_TRUE(GetAndResetSelectionMoved());
  EXPECT_EQ(end_offset + gfx::Vector2dF(0, kDefaulTapSlop),
            GetLastSelectionEnd());

  EXPECT_TRUE(controller().WillHandleTouchEvent(
      event.MovePoint(0, kDefaulTapSlop, 2 * kDefaulTapSlop)));
  EXPECT_TRUE(GetAndResetSelectionMoved());
  EXPECT_EQ(end_offset + gfx::Vector2dF(kDefaulTapSlop, kDefaulTapSlop),
            GetLastSelectionEnd());

  EXPECT_TRUE(controller().WillHandleTouchEvent(
      event.MovePoint(0, 2 * kDefaulTapSlop, 2 * kDefaulTapSlop)));
  EXPECT_TRUE(GetAndResetSelectionMoved());
  EXPECT_EQ(end_offset + gfx::Vector2dF(2 * kDefaulTapSlop, kDefaulTapSlop),
            GetLastSelectionEnd());

  // The handles should still be hidden.
  EXPECT_FALSE(test_controller.GetStartVisible());
  EXPECT_FALSE(test_controller.GetEndVisible());
  EXPECT_EQ(0.f, test_controller.GetStartAlpha());
  EXPECT_EQ(0.f, test_controller.GetEndAlpha());

  // Releasing the touch sequence should end the drag and show the handles.
  EXPECT_FALSE(controller().WillHandleTouchEvent(event.ReleasePoint()));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLE_DRAG_STOPPED));
  EXPECT_TRUE(test_controller.GetStartVisible());
  EXPECT_TRUE(test_controller.GetEndVisible());
}

TEST_F(TouchSelectionControllerTest, LongPressNoDrag) {
  EnableLongPressDragSelection();
  TouchSelectionControllerTestApi test_controller(&controller());

  gfx::RectF start_rect(-50, 0, 0, 10);
  gfx::RectF end_rect(50, 0, 0, 10);
  bool visible = true;

  // Start a touch sequence.
  MockMotionEvent event;
  EXPECT_FALSE(controller().WillHandleTouchEvent(event.PressPoint(0, 0)));

  // Activate a longpress-triggered selection.
  OnLongPressEvent();
  ChangeSelection(start_rect, visible, end_rect, visible);
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(SELECTION_HANDLES_SHOWN));
  EXPECT_EQ(start_rect.bottom_left(), GetLastEventStart());

  // The handles should remain invisible while the touch release and longpress
  // drag gesture are pending.
  EXPECT_FALSE(test_controller.GetStartVisible());
  EXPECT_FALSE(test_controller.GetEndVisible());

  EXPECT_EQ(0.f, test_controller.GetStartAlpha());
  EXPECT_EQ(0.f, test_controller.GetEndAlpha());

  // If no drag movement occurs, the handles should reappear after the touch
  // is released.
  EXPECT_FALSE(controller().WillHandleTouchEvent(event.ReleasePoint()));
  EXPECT_THAT(GetAndResetEvents(), IsEmpty());
  EXPECT_TRUE(test_controller.GetStartVisible());
  EXPECT_TRUE(test_controller.GetEndVisible());
}

TEST_F(TouchSelectionControllerTest, NoLongPressDragIfDisabled) {
  // The TouchSelectionController disables longpress drag selection by default.
  TouchSelectionControllerTestApi test_controller(&controller());

  gfx::RectF start_rect(-50, 0, 0, 10);
  gfx::RectF end_rect(50, 0, 0, 10);
  bool visible = true;

  // Start a touch sequence.
  MockMotionEvent event;
  EXPECT_FALSE(controller().WillHandleTouchEvent(event.PressPoint(0, 0)));

  // Activate a longpress-triggered selection.
  OnLongPressEvent();
  ChangeSelection(start_rect, visible, end_rect, visible);
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(SELECTION_HANDLES_SHOWN));
  EXPECT_EQ(start_rect.bottom_left(), GetLastEventStart());
  EXPECT_TRUE(test_controller.GetStartVisible());
  EXPECT_TRUE(test_controller.GetEndVisible());

  EXPECT_EQ(1.f, test_controller.GetStartAlpha());
  EXPECT_EQ(1.f, test_controller.GetEndAlpha());

  // Subsequent motion of the same touch sequence after longpress shouldn't
  // trigger drag selection.
  EXPECT_FALSE(controller().WillHandleTouchEvent(event.MovePoint(0, 0, 0)));
  EXPECT_THAT(GetAndResetEvents(), IsEmpty());

  EXPECT_FALSE(controller().WillHandleTouchEvent(
      event.MovePoint(0, 0, kDefaulTapSlop * 10)));
  EXPECT_THAT(GetAndResetEvents(), IsEmpty());

  // Releasing the touch sequence should have no effect.
  EXPECT_FALSE(controller().WillHandleTouchEvent(event.ReleasePoint()));
  EXPECT_THAT(GetAndResetEvents(), IsEmpty());
  EXPECT_TRUE(test_controller.GetStartVisible());
  EXPECT_TRUE(test_controller.GetEndVisible());

  EXPECT_EQ(1.f, test_controller.GetStartAlpha());
  EXPECT_EQ(1.f, test_controller.GetEndAlpha());
}

TEST_F(TouchSelectionControllerTest, RectBetweenBounds) {
  gfx::RectF start_rect(5, 5, 0, 10);
  gfx::RectF end_rect(50, 5, 0, 10);
  bool visible = true;

  EXPECT_EQ(gfx::RectF(), controller().GetRectBetweenBounds());

  OnLongPressEvent();
  ChangeSelection(start_rect, visible, end_rect, visible);
  ASSERT_THAT(GetAndResetEvents(),
              ElementsAre(SELECTION_HANDLES_SHOWN));
  EXPECT_EQ(gfx::RectF(5, 5, 45, 10), controller().GetRectBetweenBounds());

  // The result of |GetRectBetweenBounds| should be available within the
  // |OnSelectionEvent| callback, as stored by |GetLastEventBoundsRect()|.
  EXPECT_EQ(GetLastEventBoundsRect(), controller().GetRectBetweenBounds());

  start_rect.Offset(1, 0);
  ChangeSelection(start_rect, visible, end_rect, visible);
  ASSERT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLES_MOVED));
  EXPECT_EQ(gfx::RectF(6, 5, 44, 10), controller().GetRectBetweenBounds());
  EXPECT_EQ(GetLastEventBoundsRect(), controller().GetRectBetweenBounds());

  // If only one bound is visible, the selection bounding rect should reflect
  // only the visible bound.
  ChangeSelection(start_rect, visible, end_rect, false);
  ASSERT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLES_MOVED));
  EXPECT_EQ(start_rect, controller().GetRectBetweenBounds());
  EXPECT_EQ(GetLastEventBoundsRect(), controller().GetRectBetweenBounds());

  ChangeSelection(start_rect, false, end_rect, visible);
  ASSERT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLES_MOVED));
  EXPECT_EQ(end_rect, controller().GetRectBetweenBounds());
  EXPECT_EQ(GetLastEventBoundsRect(), controller().GetRectBetweenBounds());

  // If both bounds are visible, the full bounding rect should be returned.
  ChangeSelection(start_rect, false, end_rect, false);
  ASSERT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLES_MOVED));
  EXPECT_EQ(gfx::RectF(6, 5, 44, 10), controller().GetRectBetweenBounds());
  EXPECT_EQ(GetLastEventBoundsRect(), controller().GetRectBetweenBounds());

  ClearSelection();
  ASSERT_THAT(GetAndResetEvents(),
              ElementsAre(SELECTION_HANDLES_CLEARED));
  EXPECT_EQ(gfx::RectF(), controller().GetRectBetweenBounds());
}

TEST_F(TouchSelectionControllerTest, SelectionNoOrientationChangeWhenSwapped) {
  TouchSelectionControllerTestApi test_controller(&controller());
  base::TimeTicks event_time = base::TimeTicks::Now();
  OnLongPressEvent();

  float line_height = 10.f;
  gfx::RectF start_rect(50, line_height, 0, line_height);
  gfx::RectF end_rect(100, line_height, 0, line_height);
  bool visible = true;
  ChangeSelection(start_rect, visible, end_rect, visible);
  EXPECT_THAT(GetAndResetEvents(),
              ElementsAre(SELECTION_HANDLES_SHOWN));
  EXPECT_EQ(start_rect.bottom_left(), GetLastEventStart());
  EXPECT_EQ(test_controller.GetStartHandleOrientation(),
            TouchHandleOrientation::LEFT);
  EXPECT_EQ(test_controller.GetEndHandleOrientation(),
            TouchHandleOrientation::RIGHT);

  SetDraggingEnabled(true);

  // Simulate moving the base, not triggering a swap of points.
  MockMotionEvent event(MockMotionEvent::ACTION_DOWN, event_time,
                        start_rect.x(), start_rect.bottom());
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLE_DRAG_STARTED));

  gfx::RectF offset_rect = end_rect;
  offset_rect.Offset(gfx::Vector2dF(-10, 0));
  ChangeSelection(offset_rect, visible, end_rect, visible);
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLES_MOVED));
  EXPECT_EQ(test_controller.GetStartHandleOrientation(),
            TouchHandleOrientation::LEFT);
  EXPECT_EQ(test_controller.GetEndHandleOrientation(),
            TouchHandleOrientation::RIGHT);

  event_time += base::TimeDelta::FromMilliseconds(2 * kDefaultTapTimeoutMs);
  event = MockMotionEvent(MockMotionEvent::ACTION_UP, event_time,
                          offset_rect.x(), offset_rect.bottom());
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLE_DRAG_STOPPED));
  EXPECT_EQ(test_controller.GetStartHandleOrientation(),
            TouchHandleOrientation::LEFT);
  EXPECT_EQ(test_controller.GetEndHandleOrientation(),
            TouchHandleOrientation::RIGHT);

  // Simulate moving the base, triggering a swap of points.
  event = MockMotionEvent(MockMotionEvent::ACTION_DOWN, event_time,
                          offset_rect.x(), offset_rect.bottom());
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLE_DRAG_STARTED));

  offset_rect.Offset(gfx::Vector2dF(20, 0));
  ChangeSelection(end_rect, visible, offset_rect, visible);
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLES_MOVED));
  EXPECT_EQ(test_controller.GetStartHandleOrientation(),
            TouchHandleOrientation::LEFT);
  EXPECT_EQ(test_controller.GetEndHandleOrientation(),
            TouchHandleOrientation::LEFT);

  event_time += base::TimeDelta::FromMilliseconds(2 * kDefaultTapTimeoutMs);
  event = MockMotionEvent(MockMotionEvent::ACTION_UP, event_time,
                          offset_rect.x(), offset_rect.bottom());
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLE_DRAG_STOPPED));
  EXPECT_EQ(test_controller.GetStartHandleOrientation(),
            TouchHandleOrientation::LEFT);
  EXPECT_EQ(test_controller.GetEndHandleOrientation(),
            TouchHandleOrientation::RIGHT);

  // Simulate moving the anchor, not triggering a swap of points.
  event = MockMotionEvent(MockMotionEvent::ACTION_DOWN, event_time,
                          offset_rect.x(), offset_rect.bottom());
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLE_DRAG_STARTED));

  offset_rect.Offset(gfx::Vector2dF(-5, 0));
  ChangeSelection(end_rect, visible, offset_rect, visible);
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLES_MOVED));
  EXPECT_EQ(test_controller.GetStartHandleOrientation(),
            TouchHandleOrientation::LEFT);
  EXPECT_EQ(test_controller.GetEndHandleOrientation(),
            TouchHandleOrientation::RIGHT);

  event_time += base::TimeDelta::FromMilliseconds(2 * kDefaultTapTimeoutMs);
  event = MockMotionEvent(MockMotionEvent::ACTION_UP, event_time,
                          offset_rect.x(), offset_rect.bottom());
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLE_DRAG_STOPPED));
  EXPECT_EQ(test_controller.GetStartHandleOrientation(),
            TouchHandleOrientation::LEFT);
  EXPECT_EQ(test_controller.GetEndHandleOrientation(),
            TouchHandleOrientation::RIGHT);

  // Simulate moving the anchor, triggering a swap of points.
  event = MockMotionEvent(MockMotionEvent::ACTION_DOWN, event_time,
                          offset_rect.x(), offset_rect.bottom());
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLE_DRAG_STARTED));

  offset_rect.Offset(gfx::Vector2dF(-15, 0));
  ChangeSelection(offset_rect, visible, end_rect, visible);
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLES_MOVED));
  EXPECT_EQ(test_controller.GetStartHandleOrientation(),
            TouchHandleOrientation::RIGHT);
  EXPECT_EQ(test_controller.GetEndHandleOrientation(),
            TouchHandleOrientation::RIGHT);

  event_time += base::TimeDelta::FromMilliseconds(2 * kDefaultTapTimeoutMs);
  event = MockMotionEvent(MockMotionEvent::ACTION_UP, event_time,
                          offset_rect.x(), offset_rect.bottom());
  EXPECT_TRUE(controller().WillHandleTouchEvent(event));
  EXPECT_THAT(GetAndResetEvents(), ElementsAre(SELECTION_HANDLE_DRAG_STOPPED));
  EXPECT_EQ(test_controller.GetStartHandleOrientation(),
            TouchHandleOrientation::LEFT);
  EXPECT_EQ(test_controller.GetEndHandleOrientation(),
            TouchHandleOrientation::RIGHT);
}

}  // namespace ui
