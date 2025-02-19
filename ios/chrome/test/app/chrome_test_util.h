// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_TEST_APP_CHROME_TEST_UTIL_H_
#define IOS_CHROME_TEST_APP_CHROME_TEST_UTIL_H_

#import "base/ios/block_types.h"
#import "ios/chrome/browser/ui/commands/browser_commands.h"
#import "ios/web/public/web_state/web_state.h"

namespace ios {
class ChromeBrowserState;
}

@protocol ApplicationCommands;
@class DeviceSharingManager;
@class GenericChromeCommand;
@class MainController;
@class NewTabPageController;

namespace chrome_test_util {

// Returns the main controller.
MainController* GetMainController();

// Returns the DeviceSharingManager object.
DeviceSharingManager* GetDeviceSharingManager();

// Returns the |NewTabPageController| of the current tab if the current tab is
// a new tab and nil otherwise.
NewTabPageController* GetCurrentNewTabPageController();

// Returns the current WebState.
web::WebState* GetCurrentWebState();

// Returns the current, non-incognito ChromeBrowserState.
ios::ChromeBrowserState* GetOriginalBrowserState();

// Returns the current incognito ChromeBrowserState
ios::ChromeBrowserState* GetCurrentIncognitoBrowserState();

// Returns the number of key commands currently registered with the main BVC.
NSUInteger GetRegisteredKeyCommandsCount();

// Returns the dispatcher for the main BVC.
// TODO(crbug.com/738881): Use DispatcherForActiveViewController() instead.
id<BrowserCommands> BrowserCommandDispatcherForMainBVC();

// Returns the dispatcher for the active view controller.
id<ApplicationCommands, BrowserCommands> DispatcherForActiveViewController();

// Runs |command| using the active view controller.
void RunCommandWithActiveViewController(GenericChromeCommand* command);

// Removes all presented infobars.
void RemoveAllInfoBars();

// Dismisses all presented views and modal dialogs.
void ClearPresentedState();

// Sets the value of a boolean local state pref.
// TODO(crbug.com/647022): Clean up other tests that use this helper function.
void SetBooleanLocalStatePref(const char* pref_name, bool value);

// Sets the value of a boolean user pref in the given browser state.
void SetBooleanUserPref(ios::ChromeBrowserState* browser_state,
                        const char* pref_name,
                        bool value);

// Sets the state of using cellular network.
void SetWWANStateTo(bool value);

// Sets the state of first launch.
void SetFirstLaunchStateTo(bool value);

// Checks whether metrics recording is enabled or not.
bool IsMetricsRecordingEnabled();

// Checks whether metrics reporting is enabled or not.
bool IsMetricsReportingEnabled();

// Checks whether breakpad recording is enabled or not.
bool IsBreakpadEnabled();

// Checks whether breakpad reporting is enabled or not.
bool IsBreakpadReportingEnabled();

// Checks whether this is the first launch after upgrade or not.
bool IsFirstLaunchAfterUpgrade();

// Waits for Breakpad to process the queued updates.
void WaitForBreakpadQueue();

// Simulates launching Chrome from another application.
void OpenChromeFromExternalApp(const GURL& url);

}  // namespace chrome_test_util

#endif  // IOS_CHROME_TEST_APP_CHROME_TEST_UTIL_H_
