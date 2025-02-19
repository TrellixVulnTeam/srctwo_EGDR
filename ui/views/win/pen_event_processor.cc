// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pen_event_processor.h"

#include "base/memory/ptr_util.h"
#include "base/time/time.h"
#include "ui/events/event_utils.h"

namespace views {
namespace {

int GetFlagsFromPointerMessage(UINT message, const POINTER_INFO& pointer_info) {
  int flags = ui::EF_NONE;
  if (pointer_info.pointerFlags & POINTER_FLAG_FIRSTBUTTON)
    flags |= ui::EF_LEFT_MOUSE_BUTTON;

  if (pointer_info.pointerFlags & POINTER_FLAG_SECONDBUTTON)
    flags |= ui::EF_RIGHT_MOUSE_BUTTON;

  if (message == WM_POINTERUP) {
    if (pointer_info.ButtonChangeType == POINTER_CHANGE_SECONDBUTTON_UP)
      flags |= ui::EF_RIGHT_MOUSE_BUTTON;
    else
      flags |= ui::EF_LEFT_MOUSE_BUTTON;
  }
  return flags;
}

}  // namespace

PenEventProcessor::PenEventProcessor(ui::SequentialIDGenerator* id_generator,
                                     bool direct_manipulation_enabled)
    : id_generator_(id_generator),
      direct_manipulation_enabled_(direct_manipulation_enabled) {}

PenEventProcessor::~PenEventProcessor() {}

std::unique_ptr<ui::Event> PenEventProcessor::GenerateEvent(
    UINT message,
    UINT32 pointer_id,
    const POINTER_PEN_INFO& pointer_pen_info,
    const gfx::Point& point) {
  unsigned int mapped_pointer_id = id_generator_->GetGeneratedID(pointer_id);

  // We are now creating a fake mouse event with pointer type of pen from
  // the WM_POINTER message and then setting up an associated pointer
  // details in the MouseEvent which contains the pen's information.
  ui::EventPointerType input_type = ui::EventPointerType::POINTER_TYPE_PEN;
  // TODO(lanwei): penFlags of PEN_FLAG_INVERTED may also indicate we are using
  // an eraser, but it is under debate. Please see
  // https://github.com/w3c/pointerevents/issues/134/.
  if (pointer_pen_info.penFlags & PEN_FLAG_ERASER)
    input_type = ui::EventPointerType::POINTER_TYPE_ERASER;

  // convert pressure into a float [0, 1]. The range of the pressure is
  // [0, 1024] as specified on MSDN.
  float pressure = static_cast<float>(pointer_pen_info.pressure) / 1024;
  float rotation = pointer_pen_info.rotation;
  int tilt_x = pointer_pen_info.tiltX;
  int tilt_y = pointer_pen_info.tiltY;
  ui::PointerDetails pointer_details(
      input_type, mapped_pointer_id, /* radius_x */ 0.0f, /* radius_y */ 0.0f,
      pressure, tilt_x, tilt_y, /* tangential_pressure */ 0.0f, rotation);

  // If the flag is disabled, we send mouse events for all pen inputs.
  if (!direct_manipulation_enabled_) {
    return GenerateMouseEvent(message, pointer_id, pointer_pen_info.pointerInfo,
                              point, pointer_details);
  }
  bool is_pointer_event =
      message == WM_POINTERENTER || message == WM_POINTERLEAVE;

  // Send MouseEvents when the pen is hovering or any buttons (other than the
  // tip) are depressed when the stylus makes contact with the digitizer. Ensure
  // we read |send_touch_for_pen_| before we process the event as we want to
  // ensure a TouchRelease is sent appropriately at the end when the stylus is
  // no longer in contact with the digitizer.
  bool send_touch = send_touch_for_pen_;
  if (pointer_pen_info.pointerInfo.pointerFlags & POINTER_FLAG_INCONTACT) {
    if (!pen_in_contact_) {
      send_touch = send_touch_for_pen_ =
          (pointer_pen_info.pointerInfo.pointerFlags &
           (POINTER_FLAG_SECONDBUTTON | POINTER_FLAG_THIRDBUTTON |
            POINTER_FLAG_FOURTHBUTTON | POINTER_FLAG_FIFTHBUTTON)) == 0;
    }
    pen_in_contact_ = true;
  } else {
    pen_in_contact_ = false;
    send_touch_for_pen_ = false;
  }

  if (is_pointer_event || !send_touch) {
    return GenerateMouseEvent(message, pointer_id, pointer_pen_info.pointerInfo,
                              point, pointer_details);
  }

  return GenerateTouchEvent(message, pointer_id, pointer_pen_info.pointerInfo,
                            point, pointer_details);
}

std::unique_ptr<ui::Event> PenEventProcessor::GenerateMouseEvent(
    UINT message,
    UINT32 pointer_id,
    const POINTER_INFO& pointer_info,
    const gfx::Point& point,
    const ui::PointerDetails& pointer_details) {
  ui::EventType event_type = ui::ET_MOUSE_MOVED;
  int flag = GetFlagsFromPointerMessage(message, pointer_info);
  int changed_flag = ui::EF_NONE;
  int click_count = 0;
  switch (message) {
    case WM_POINTERDOWN:
      event_type = ui::ET_MOUSE_PRESSED;
      if (pointer_info.ButtonChangeType == POINTER_CHANGE_FIRSTBUTTON_DOWN)
        changed_flag = ui::EF_LEFT_MOUSE_BUTTON;
      else
        changed_flag = ui::EF_RIGHT_MOUSE_BUTTON;
      click_count = 1;
      sent_mouse_down_ = true;
      break;
    case WM_POINTERUP:
      event_type = ui::ET_MOUSE_RELEASED;
      if (pointer_info.ButtonChangeType == POINTER_CHANGE_FIRSTBUTTON_UP)
        changed_flag = ui::EF_LEFT_MOUSE_BUTTON;
      else
        changed_flag = ui::EF_RIGHT_MOUSE_BUTTON;
      id_generator_->MaybeReleaseNumber(pointer_id);
      click_count = 1;
      if (!sent_mouse_down_)
        return nullptr;
      sent_mouse_down_ = false;
      break;
    case WM_POINTERUPDATE:
      event_type = ui::ET_MOUSE_DRAGGED;
      if (flag == ui::EF_NONE)
        event_type = ui::ET_MOUSE_MOVED;
      break;
    case WM_POINTERENTER:
      event_type = ui::ET_MOUSE_ENTERED;
      break;
    case WM_POINTERLEAVE:
      event_type = ui::ET_MOUSE_EXITED;
      id_generator_->MaybeReleaseNumber(pointer_id);
      break;
    default:
      NOTREACHED();
  }
  std::unique_ptr<ui::Event> event = base::MakeUnique<ui::MouseEvent>(
      event_type, point, point, ui::EventTimeForNow(), flag, changed_flag,
      pointer_details);
  event->AsMouseEvent()->SetClickCount(click_count);
  return event;
}

std::unique_ptr<ui::Event> PenEventProcessor::GenerateTouchEvent(
    UINT message,
    UINT32 pointer_id,
    const POINTER_INFO& pointer_info,
    const gfx::Point& point,
    const ui::PointerDetails& pointer_details) {
  int flags = GetFlagsFromPointerMessage(message, pointer_info);

  ui::EventType event_type = ui::ET_TOUCH_MOVED;
  switch (message) {
    case WM_POINTERDOWN:
      event_type = ui::ET_TOUCH_PRESSED;
      sent_touch_start_ = true;
      break;
    case WM_POINTERUP:
      event_type = ui::ET_TOUCH_RELEASED;
      id_generator_->MaybeReleaseNumber(pointer_id);
      if (!sent_touch_start_)
        return nullptr;
      sent_touch_start_ = false;
      break;
    case WM_POINTERUPDATE:
      event_type = ui::ET_TOUCH_MOVED;
      break;
    default:
      NOTREACHED();
  }

  const base::TimeTicks event_time = ui::EventTimeForNow();

  int rotation_angle = static_cast<int>(pointer_details.twist) % 180;
  if (rotation_angle < 0)
    rotation_angle += 180;
  std::unique_ptr<ui::Event> event = base::MakeUnique<ui::TouchEvent>(
      event_type, point, event_time, pointer_details, flags, rotation_angle);
  event->latency()->AddLatencyNumberWithTimestamp(
      ui::INPUT_EVENT_LATENCY_ORIGINAL_COMPONENT, 0, 0, event_time, 1);
  return event;
}

}  // namespace views
