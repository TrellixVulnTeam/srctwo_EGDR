// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_RENDERER_HOST_INPUT_MOCK_INPUT_DISPOSITION_HANDLER_H_
#define CONTENT_BROWSER_RENDERER_HOST_INPUT_MOCK_INPUT_DISPOSITION_HANDLER_H_

#include <stddef.h>

#include <memory>
#include <utility>

#include "content/browser/renderer_host/input/input_disposition_handler.h"

namespace content {

class InputRouter;

class MockInputDispositionHandler : public InputDispositionHandler {
 public:
  MockInputDispositionHandler();
  ~MockInputDispositionHandler() override;

  // InputDispositionHandler
  void OnKeyboardEventAck(const NativeWebKeyboardEventWithLatencyInfo& event,
                          InputEventAckState ack_result) override;
  void OnMouseEventAck(const MouseEventWithLatencyInfo& event,
                       InputEventAckState ack_result) override;
  void OnWheelEventAck(const MouseWheelEventWithLatencyInfo& event,
                       InputEventAckState ack_result) override;
  void OnTouchEventAck(const TouchEventWithLatencyInfo& event,
                       InputEventAckState ack_result) override;
  void OnGestureEventAck(const GestureEventWithLatencyInfo& event,
                         InputEventAckState ack_result) override;
  void OnUnexpectedEventAck(UnexpectedEventAckType type) override;

  size_t GetAndResetAckCount();

  void set_input_router(InputRouter* input_router) {
    input_router_ = input_router;
  }

  void set_followup_touch_event(
      std::unique_ptr<GestureEventWithLatencyInfo> event) {
    gesture_followup_event_ = std::move(event);
  }

  void set_followup_touch_event(
      std::unique_ptr<TouchEventWithLatencyInfo> event) {
    touch_followup_event_ = std::move(event);
  }

  bool unexpected_event_ack_called() const {
    return unexpected_event_ack_called_;
  }
  InputEventAckState ack_state() const { return ack_state_; }

  InputEventAckState acked_wheel_event_state() const {
    return acked_wheel_event_state_;
  }

  blink::WebInputEvent::Type ack_event_type() const { return ack_event_type_; }

  const NativeWebKeyboardEvent& acked_keyboard_event() const {
    return *acked_key_event_;
  }
  const blink::WebMouseWheelEvent& acked_wheel_event() const {
    return acked_wheel_event_;
  }
  const TouchEventWithLatencyInfo& acked_touch_event() const {
    return acked_touch_event_;
  }
  const blink::WebGestureEvent& acked_gesture_event() const {
    return acked_gesture_event_;
  }

 private:
  void RecordAckCalled(blink::WebInputEvent::Type eventType,
                       InputEventAckState ack_result);

  InputRouter* input_router_;

  size_t ack_count_;
  bool unexpected_event_ack_called_;
  blink::WebInputEvent::Type ack_event_type_;
  InputEventAckState ack_state_;
  InputEventAckState acked_wheel_event_state_;
  std::unique_ptr<NativeWebKeyboardEvent> acked_key_event_;
  blink::WebMouseWheelEvent acked_wheel_event_;
  TouchEventWithLatencyInfo acked_touch_event_;
  blink::WebGestureEvent acked_gesture_event_;
  blink::WebMouseEvent acked_mouse_event_;

  std::unique_ptr<GestureEventWithLatencyInfo> gesture_followup_event_;
  std::unique_ptr<TouchEventWithLatencyInfo> touch_followup_event_;
};

}  // namespace content

#endif  // CONTENT_BROWSER_RENDERER_HOST_INPUT_MOCK_INPUT_DISPOSITION_HANDLER_H_
