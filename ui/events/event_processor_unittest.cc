// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <utility>
#include <vector>

#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/events/event.h"
#include "ui/events/event_target_iterator.h"
#include "ui/events/event_targeter.h"
#include "ui/events/event_utils.h"
#include "ui/events/test/events_test_utils.h"
#include "ui/events/test/test_event_handler.h"
#include "ui/events/test/test_event_processor.h"
#include "ui/events/test/test_event_target.h"
#include "ui/events/test/test_event_targeter.h"

typedef std::vector<std::string> HandlerSequenceRecorder;

namespace ui {
namespace test {

class EventProcessorTest : public testing::Test {
 public:
  EventProcessorTest() {}
  ~EventProcessorTest() override {}

 protected:
  // testing::Test:
  void SetUp() override {
    processor_.SetRoot(base::MakeUnique<TestEventTarget>());
    processor_.Reset();
    root()->SetEventTargeter(
        base::MakeUnique<TestEventTargeter>(root(), false));
  }

  TestEventTarget* root() {
    return static_cast<TestEventTarget*>(processor_.GetRoot());
  }

  TestEventProcessor* processor() {
    return &processor_;
  }

  void DispatchEvent(Event* event) {
    processor_.OnEventFromSource(event);
  }

  void SetTarget(TestEventTarget* target) {
    static_cast<TestEventTargeter*>(root()->GetEventTargeter())
        ->set_target(target);
  }

 private:
  TestEventProcessor processor_;

  DISALLOW_COPY_AND_ASSIGN(EventProcessorTest);
};

TEST_F(EventProcessorTest, Basic) {
  std::unique_ptr<TestEventTarget> child(new TestEventTarget());
  child->SetEventTargeter(
      base::MakeUnique<TestEventTargeter>(child.get(), false));
  SetTarget(child.get());
  root()->AddChild(std::move(child));

  MouseEvent mouse(ET_MOUSE_MOVED, gfx::Point(10, 10), gfx::Point(10, 10),
                   EventTimeForNow(), EF_NONE, EF_NONE);
  DispatchEvent(&mouse);
  EXPECT_TRUE(root()->child_at(0)->DidReceiveEvent(ET_MOUSE_MOVED));
  EXPECT_FALSE(root()->DidReceiveEvent(ET_MOUSE_MOVED));

  SetTarget(root());
  root()->RemoveChild(root()->child_at(0));
  DispatchEvent(&mouse);
  EXPECT_TRUE(root()->DidReceiveEvent(ET_MOUSE_MOVED));
}

// ReDispatchEventHandler is used to receive mouse events and forward them
// to a specified EventProcessor. Verifies that the event has the correct
// target and phase both before and after the nested event processing. Also
// verifies that the location of the event remains the same after it has
// been processed by the second EventProcessor.
class ReDispatchEventHandler : public TestEventHandler {
 public:
  ReDispatchEventHandler(EventProcessor* processor, EventTarget* target)
      : processor_(processor), expected_target_(target) {}
  ~ReDispatchEventHandler() override {}

  // TestEventHandler:
  void OnMouseEvent(MouseEvent* event) override {
    TestEventHandler::OnMouseEvent(event);

    EXPECT_EQ(expected_target_, event->target());
    EXPECT_EQ(EP_TARGET, event->phase());

    gfx::Point location(event->location());
    EventDispatchDetails details = processor_->OnEventFromSource(event);
    EXPECT_FALSE(details.dispatcher_destroyed);
    EXPECT_FALSE(details.target_destroyed);

    // The nested event-processing should not have mutated the target,
    // phase, or location of |event|.
    EXPECT_EQ(expected_target_, event->target());
    EXPECT_EQ(EP_TARGET, event->phase());
    EXPECT_EQ(location, event->location());
  }

 private:
  EventProcessor* processor_;
  EventTarget* expected_target_;

  DISALLOW_COPY_AND_ASSIGN(ReDispatchEventHandler);
};

// Verifies that the phase and target information of an event is not mutated
// as a result of sending the event to an event processor while it is still
// being processed by another event processor.
TEST_F(EventProcessorTest, NestedEventProcessing) {
  // Add one child to the default event processor used in this test suite.
  std::unique_ptr<TestEventTarget> child(new TestEventTarget());
  SetTarget(child.get());
  root()->AddChild(std::move(child));

  // Define a second root target and child.
  std::unique_ptr<EventTarget> second_root_scoped(new TestEventTarget());
  TestEventTarget* second_root =
      static_cast<TestEventTarget*>(second_root_scoped.get());
  std::unique_ptr<TestEventTarget> second_child(new TestEventTarget());
  second_root->SetEventTargeter(
      base::WrapUnique(new TestEventTargeter(second_child.get(), false)));
  second_root->AddChild(std::move(second_child));

  // Define a second event processor which owns the second root.
  std::unique_ptr<TestEventProcessor> second_processor(
      new TestEventProcessor());
  second_processor->SetRoot(std::move(second_root_scoped));

  // Indicate that an event which is dispatched to the child target owned by the
  // first event processor should be handled by |target_handler| instead.
  std::unique_ptr<TestEventHandler> target_handler(
      new ReDispatchEventHandler(second_processor.get(), root()->child_at(0)));
  ignore_result(root()->child_at(0)->SetTargetHandler(target_handler.get()));

  // Dispatch a mouse event to the tree of event targets owned by the first
  // event processor, checking in ReDispatchEventHandler that the phase and
  // target information of the event is correct.
  MouseEvent mouse(ET_MOUSE_MOVED, gfx::Point(10, 10), gfx::Point(10, 10),
                   EventTimeForNow(), EF_NONE, EF_NONE);
  DispatchEvent(&mouse);

  // Verify also that |mouse| was seen by the child nodes contained in both
  // event processors and that the event was not handled.
  EXPECT_EQ(1, target_handler->num_mouse_events());
  EXPECT_TRUE(second_root->child_at(0)->DidReceiveEvent(ET_MOUSE_MOVED));
  EXPECT_FALSE(mouse.handled());
  second_root->child_at(0)->ResetReceivedEvents();
  root()->child_at(0)->ResetReceivedEvents();

  target_handler->Reset();

  // Indicate that the child of the second root should handle events, and
  // dispatch another mouse event to verify that it is marked as handled.
  second_root->child_at(0)->set_mark_events_as_handled(true);
  MouseEvent mouse2(ET_MOUSE_MOVED, gfx::Point(10, 10), gfx::Point(10, 10),
                    EventTimeForNow(), EF_NONE, EF_NONE);
  DispatchEvent(&mouse2);
  EXPECT_EQ(1, target_handler->num_mouse_events());
  EXPECT_TRUE(second_root->child_at(0)->DidReceiveEvent(ET_MOUSE_MOVED));
  EXPECT_TRUE(mouse2.handled());
}

// Verifies that OnEventProcessingFinished() is called when an event
// has been handled.
TEST_F(EventProcessorTest, OnEventProcessingFinished) {
  std::unique_ptr<TestEventTarget> child(new TestEventTarget());
  child->set_mark_events_as_handled(true);
  SetTarget(child.get());
  root()->AddChild(std::move(child));

  // Dispatch a mouse event. We expect the event to be seen by the target,
  // handled, and we expect OnEventProcessingFinished() to be invoked once.
  MouseEvent mouse(ET_MOUSE_MOVED, gfx::Point(10, 10), gfx::Point(10, 10),
                   EventTimeForNow(), EF_NONE, EF_NONE);
  DispatchEvent(&mouse);
  EXPECT_TRUE(root()->child_at(0)->DidReceiveEvent(ET_MOUSE_MOVED));
  EXPECT_FALSE(root()->DidReceiveEvent(ET_MOUSE_MOVED));
  EXPECT_TRUE(mouse.handled());
  EXPECT_EQ(1, processor()->num_times_processing_finished());
}

// Verifies that OnEventProcessingStarted() has been called when starting to
// process an event, and that processing does not take place if
// OnEventProcessingStarted() marks the event as handled. Also verifies that
// OnEventProcessingFinished() is also called in either case.
TEST_F(EventProcessorTest, OnEventProcessingStarted) {
  std::unique_ptr<TestEventTarget> child(new TestEventTarget());
  SetTarget(child.get());
  root()->AddChild(std::move(child));

  // Dispatch a mouse event. We expect the event to be seen by the target,
  // OnEventProcessingStarted() should be called once, and
  // OnEventProcessingFinished() should be called once. The event should
  // remain unhandled.
  MouseEvent mouse(ET_MOUSE_MOVED, gfx::Point(10, 10), gfx::Point(10, 10),
                   EventTimeForNow(), EF_NONE, EF_NONE);
  DispatchEvent(&mouse);
  EXPECT_TRUE(root()->child_at(0)->DidReceiveEvent(ET_MOUSE_MOVED));
  EXPECT_FALSE(root()->DidReceiveEvent(ET_MOUSE_MOVED));
  EXPECT_FALSE(mouse.handled());
  EXPECT_EQ(1, processor()->num_times_processing_started());
  EXPECT_EQ(1, processor()->num_times_processing_finished());
  processor()->Reset();
  root()->ResetReceivedEvents();
  root()->child_at(0)->ResetReceivedEvents();

  // Dispatch another mouse event, but with OnEventProcessingStarted() marking
  // the event as handled to prevent processing. We expect the event to not be
  // seen by the target this time, but OnEventProcessingStarted() and
  // OnEventProcessingFinished() should both still be called once.
  processor()->set_should_processing_occur(false);
  MouseEvent mouse2(ET_MOUSE_MOVED, gfx::Point(10, 10), gfx::Point(10, 10),
                    EventTimeForNow(), EF_NONE, EF_NONE);
  DispatchEvent(&mouse2);
  EXPECT_FALSE(root()->child_at(0)->DidReceiveEvent(ET_MOUSE_MOVED));
  EXPECT_FALSE(root()->DidReceiveEvent(ET_MOUSE_MOVED));
  EXPECT_TRUE(mouse2.handled());
  EXPECT_EQ(1, processor()->num_times_processing_started());
  EXPECT_EQ(1, processor()->num_times_processing_finished());
}

// Tests that unhandled events are correctly dispatched to the next-best
// target as decided by the TestEventTargeter.
TEST_F(EventProcessorTest, DispatchToNextBestTarget) {
  std::unique_ptr<TestEventTarget> child(new TestEventTarget());
  std::unique_ptr<TestEventTarget> grandchild(new TestEventTarget());

  // Install a TestEventTargeter which permits bubbling.
  root()->SetEventTargeter(
      base::WrapUnique(new TestEventTargeter(grandchild.get(), true)));
  child->AddChild(std::move(grandchild));
  root()->AddChild(std::move(child));

  ASSERT_EQ(1u, root()->child_count());
  ASSERT_EQ(1u, root()->child_at(0)->child_count());
  ASSERT_EQ(0u, root()->child_at(0)->child_at(0)->child_count());

  TestEventTarget* child_r = root()->child_at(0);
  TestEventTarget* grandchild_r = child_r->child_at(0);

  // When the root has a TestEventTargeter installed which permits bubbling,
  // events targeted at the grandchild target should be dispatched to all three
  // targets.
  KeyEvent key_event(ET_KEY_PRESSED, VKEY_ESCAPE, EF_NONE);
  DispatchEvent(&key_event);
  EXPECT_TRUE(root()->DidReceiveEvent(ET_KEY_PRESSED));
  EXPECT_TRUE(child_r->DidReceiveEvent(ET_KEY_PRESSED));
  EXPECT_TRUE(grandchild_r->DidReceiveEvent(ET_KEY_PRESSED));
  root()->ResetReceivedEvents();
  child_r->ResetReceivedEvents();
  grandchild_r->ResetReceivedEvents();

  // Add a pre-target handler on the child of the root that will mark the event
  // as handled. No targets in the hierarchy should receive the event.
  TestEventHandler handler;
  child_r->AddPreTargetHandler(&handler);
  key_event = KeyEvent(ET_KEY_PRESSED, VKEY_ESCAPE, EF_NONE);
  DispatchEvent(&key_event);
  EXPECT_FALSE(root()->DidReceiveEvent(ET_KEY_PRESSED));
  EXPECT_FALSE(child_r->DidReceiveEvent(ET_KEY_PRESSED));
  EXPECT_FALSE(grandchild_r->DidReceiveEvent(ET_KEY_PRESSED));
  EXPECT_EQ(1, handler.num_key_events());
  handler.Reset();

  // Add a post-target handler on the child of the root that will mark the event
  // as handled. Only the grandchild (the initial target) should receive the
  // event.
  child_r->RemovePreTargetHandler(&handler);
  child_r->AddPostTargetHandler(&handler);
  key_event = KeyEvent(ET_KEY_PRESSED, VKEY_ESCAPE, EF_NONE);
  DispatchEvent(&key_event);
  EXPECT_FALSE(root()->DidReceiveEvent(ET_KEY_PRESSED));
  EXPECT_FALSE(child_r->DidReceiveEvent(ET_KEY_PRESSED));
  EXPECT_TRUE(grandchild_r->DidReceiveEvent(ET_KEY_PRESSED));
  EXPECT_EQ(1, handler.num_key_events());
  handler.Reset();
  grandchild_r->ResetReceivedEvents();
  child_r->RemovePostTargetHandler(&handler);

  // Mark the event as handled when it reaches the EP_TARGET phase of
  // dispatch at the child of the root. The child and grandchild
  // targets should both receive the event, but the root should not.
  child_r->set_mark_events_as_handled(true);
  key_event = KeyEvent(ET_KEY_PRESSED, VKEY_ESCAPE, EF_NONE);
  DispatchEvent(&key_event);
  EXPECT_FALSE(root()->DidReceiveEvent(ET_KEY_PRESSED));
  EXPECT_TRUE(child_r->DidReceiveEvent(ET_KEY_PRESSED));
  EXPECT_TRUE(grandchild_r->DidReceiveEvent(ET_KEY_PRESSED));
  root()->ResetReceivedEvents();
  child_r->ResetReceivedEvents();
  grandchild_r->ResetReceivedEvents();
  child_r->set_mark_events_as_handled(false);
}

// Tests that unhandled events are seen by the correct sequence of
// targets, pre-target handlers, and post-target handlers when
// a TestEventTargeter is installed on the root target which permits bubbling.
TEST_F(EventProcessorTest, HandlerSequence) {
  std::unique_ptr<TestEventTarget> child(new TestEventTarget());
  std::unique_ptr<TestEventTarget> grandchild(new TestEventTarget());

  // Install a TestEventTargeter which permits bubbling.
  root()->SetEventTargeter(
      base::WrapUnique(new TestEventTargeter(grandchild.get(), true)));
  child->AddChild(std::move(grandchild));
  root()->AddChild(std::move(child));

  ASSERT_EQ(1u, root()->child_count());
  ASSERT_EQ(1u, root()->child_at(0)->child_count());
  ASSERT_EQ(0u, root()->child_at(0)->child_at(0)->child_count());

  TestEventTarget* child_r = root()->child_at(0);
  TestEventTarget* grandchild_r = child_r->child_at(0);

  HandlerSequenceRecorder recorder;
  root()->set_target_name("R");
  root()->set_recorder(&recorder);
  child_r->set_target_name("C");
  child_r->set_recorder(&recorder);
  grandchild_r->set_target_name("G");
  grandchild_r->set_recorder(&recorder);

  TestEventHandler pre_root;
  pre_root.set_handler_name("PreR");
  pre_root.set_recorder(&recorder);
  root()->AddPreTargetHandler(&pre_root);

  TestEventHandler pre_child;
  pre_child.set_handler_name("PreC");
  pre_child.set_recorder(&recorder);
  child_r->AddPreTargetHandler(&pre_child);

  TestEventHandler pre_grandchild;
  pre_grandchild.set_handler_name("PreG");
  pre_grandchild.set_recorder(&recorder);
  grandchild_r->AddPreTargetHandler(&pre_grandchild);

  TestEventHandler post_root;
  post_root.set_handler_name("PostR");
  post_root.set_recorder(&recorder);
  root()->AddPostTargetHandler(&post_root);

  TestEventHandler post_child;
  post_child.set_handler_name("PostC");
  post_child.set_recorder(&recorder);
  child_r->AddPostTargetHandler(&post_child);

  TestEventHandler post_grandchild;
  post_grandchild.set_handler_name("PostG");
  post_grandchild.set_recorder(&recorder);
  grandchild_r->AddPostTargetHandler(&post_grandchild);

  MouseEvent mouse(ET_MOUSE_MOVED, gfx::Point(10, 10), gfx::Point(10, 10),
                   EventTimeForNow(), EF_NONE, EF_NONE);
  DispatchEvent(&mouse);

  std::string expected[] = { "PreR", "PreC", "PreG", "G", "PostG", "PostC",
      "PostR", "PreR", "PreC", "C", "PostC", "PostR", "PreR", "R", "PostR" };
  EXPECT_EQ(std::vector<std::string>(
      expected, expected + arraysize(expected)), recorder);
}

}  // namespace test
}  // namespace ui
