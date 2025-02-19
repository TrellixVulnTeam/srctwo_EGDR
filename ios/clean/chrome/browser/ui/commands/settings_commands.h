// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CLEAN_CHROME_BROWSER_UI_COMMANDS_SETTINGS_COMMANDS_H_
#define IOS_CLEAN_CHROME_BROWSER_UI_COMMANDS_SETTINGS_COMMANDS_H_

// Commands relating to the Settings UI.
@protocol SettingsCommands
// Display the settings UI.
- (void)showSettings;
// Dismiss the settings UI.
- (void)closeSettings;
@end

#endif  // IOS_CLEAN_CHROME_BROWSER_UI_COMMANDS_SETTINGS_COMMANDS_H_
