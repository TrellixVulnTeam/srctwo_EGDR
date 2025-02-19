// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/gamepad/game_controller_data_fetcher_mac.h"

#include <string.h>

#include "base/strings/string16.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "device/gamepad/gamepad_standard_mappings.h"

#import <GameController/GameController.h>

namespace device {

namespace {

void CopyNSStringAsUTF16LittleEndian(NSString* src,
                                     UChar* dest,
                                     size_t dest_len) {
  NSData* as16 = [src dataUsingEncoding:NSUTF16LittleEndianStringEncoding];
  memset(dest, 0, dest_len);
  [as16 getBytes:dest length:dest_len - sizeof(UChar)];
}

}  // namespace

GameControllerDataFetcherMac::GameControllerDataFetcherMac() {}

GameControllerDataFetcherMac::~GameControllerDataFetcherMac() {}

GamepadSource GameControllerDataFetcherMac::source() {
  return Factory::static_source();
}

void GameControllerDataFetcherMac::GetGamepadData(bool) {
  NSArray* controllers = [GCController controllers];

  for (GCController* controller in controllers) {
    auto extended_gamepad = [controller extendedGamepad];

    // We only support the extendedGamepad profile, the basic gamepad profile
    // appears to only be for iOS devices.
    if (!extended_gamepad)
      continue;

    int player_index = [controller playerIndex];
    PadState* state = GetPadState(player_index);
    if (!state)
      continue;

    Gamepad& pad = state->data;

    // This first time we encounter a gamepad, set its name, mapping, and
    // axes/button counts. This information is static, so it only needs to be
    // done once.
    if (state->active_state == GAMEPAD_NEWLY_ACTIVE) {
      NSString* vendorName = [controller vendorName];
      NSString* ident =
          [NSString stringWithFormat:@"%@ (STANDARD GAMEPAD)",
                                     vendorName ? vendorName : @"Unknown"];
      CopyNSStringAsUTF16LittleEndian(ident, pad.id, sizeof(pad.id));

      CopyNSStringAsUTF16LittleEndian(@"standard", pad.mapping,
                                      sizeof(pad.mapping));

      pad.axes_length = AXIS_INDEX_COUNT;
      pad.buttons_length = BUTTON_INDEX_COUNT - 1;
      pad.connected = true;
    }

    pad.timestamp = base::TimeTicks::Now().ToInternalValue();

    pad.axes[AXIS_INDEX_LEFT_STICK_X] =
        [[[extended_gamepad leftThumbstick] xAxis] value];
    pad.axes[AXIS_INDEX_LEFT_STICK_Y] =
        -[[[extended_gamepad leftThumbstick] yAxis] value];
    pad.axes[AXIS_INDEX_RIGHT_STICK_X] =
        [[[extended_gamepad rightThumbstick] xAxis] value];
    pad.axes[AXIS_INDEX_RIGHT_STICK_Y] =
        -[[[extended_gamepad rightThumbstick] yAxis] value];

#define BUTTON(i, b)                      \
  pad.buttons[i].pressed = [b isPressed]; \
  pad.buttons[i].value = [b value];

    BUTTON(BUTTON_INDEX_PRIMARY, [extended_gamepad buttonA]);
    BUTTON(BUTTON_INDEX_SECONDARY, [extended_gamepad buttonB]);
    BUTTON(BUTTON_INDEX_TERTIARY, [extended_gamepad buttonX]);
    BUTTON(BUTTON_INDEX_QUATERNARY, [extended_gamepad buttonY]);
    BUTTON(BUTTON_INDEX_LEFT_SHOULDER, [extended_gamepad leftShoulder]);
    BUTTON(BUTTON_INDEX_RIGHT_SHOULDER, [extended_gamepad rightShoulder]);
    BUTTON(BUTTON_INDEX_LEFT_TRIGGER, [extended_gamepad leftTrigger]);
    BUTTON(BUTTON_INDEX_RIGHT_TRIGGER, [extended_gamepad rightTrigger]);

    // No start, select, or thumbstick buttons

    BUTTON(BUTTON_INDEX_DPAD_UP, [[extended_gamepad dpad] up]);
    BUTTON(BUTTON_INDEX_DPAD_DOWN, [[extended_gamepad dpad] down]);
    BUTTON(BUTTON_INDEX_DPAD_LEFT, [[extended_gamepad dpad] left]);
    BUTTON(BUTTON_INDEX_DPAD_RIGHT, [[extended_gamepad dpad] right]);

#undef BUTTON
  }
}

}  // namespace device
