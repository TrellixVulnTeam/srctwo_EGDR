// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/accelerator_utils.h"

#import <Cocoa/Cocoa.h>

#include "chrome/browser/global_keyboard_shortcuts_mac.h"
#include "chrome/browser/ui/cocoa/accelerators_cocoa.h"
#include "ui/base/accelerators/accelerator.h"
#import "ui/base/accelerators/platform_accelerator_cocoa.h"
#import "ui/events/keycodes/keyboard_code_conversion_mac.h"

namespace chrome {

bool IsChromeAccelerator(const ui::Accelerator& accelerator, Profile* profile) {
  // The |accelerator| passed in contains a Windows key code but no platform
  // accelerator info. The Accelerator list is the opposite: It has accelerators
  // that have key_code() == VKEY_UNKNOWN but they contain a platform
  // accelerator. We find common ground by converting the passed in Windows key
  // code to a character and use that when comparing against the Accelerator
  // list.
  unichar shifted_character;
  ui::MacKeyCodeForWindowsKeyCode(accelerator.key_code(), 0, &shifted_character,
                                  nullptr);
  NSString* characters =
      [[[NSString alloc] initWithCharacters:&shifted_character
                                     length:1] autorelease];

  NSUInteger modifiers =
      (accelerator.IsCtrlDown() ? NSControlKeyMask : 0) |
      (accelerator.IsCmdDown() ? NSCommandKeyMask : 0) |
      (accelerator.IsAltDown() ? NSAlternateKeyMask : 0) |
      (accelerator.IsShiftDown() ? NSShiftKeyMask : 0);

  NSEvent* event = [NSEvent keyEventWithType:NSKeyDown
                                    location:NSZeroPoint
                               modifierFlags:modifiers
                                   timestamp:0
                                windowNumber:0
                                     context:nil
                                  characters:characters
                 charactersIgnoringModifiers:characters
                                   isARepeat:NO
                                     keyCode:accelerator.key_code()];

  return CommandForKeyEvent(event) != -1;
}

ui::Accelerator GetPrimaryChromeAcceleratorForCommandId(int command_id) {
  const ui::Accelerator* accelerator =
      AcceleratorsCocoa::GetInstance()->GetAcceleratorForCommand(command_id);

  return accelerator ? *accelerator : ui::Accelerator();
}

}  // namespace chrome
