// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_APP_MAIN_CONTROLLER_H_
#define IOS_CHROME_APP_MAIN_CONTROLLER_H_

#import <UIKit/UIKit.h>

#import "ios/chrome/app/application_delegate/app_navigation.h"
#import "ios/chrome/app/application_delegate/browser_launcher.h"
#import "ios/chrome/app/application_delegate/startup_information.h"
#import "ios/chrome/app/application_delegate/tab_opening.h"
#import "ios/chrome/app/application_delegate/tab_switching.h"
#import "ios/chrome/browser/ui/commands/application_commands.h"
#import "ios/chrome/browser/ui/main/browser_view_information.h"

@class AppState;
@class MetricsMediator;

// The main controller of the application, owned by the MainWindow nib. Also
// serves as the delegate for the app. Owns all the various top-level
// UI controllers.
//
// By design, it has no public API of its own. Anything interacting with
// MainController should be doing so through a specific protocol.
@interface MainController : NSObject<ApplicationCommands,
                                     AppNavigation,
                                     BrowserLauncher,
                                     StartupInformation,
                                     TabOpening,
                                     TabSwitching>

// A BrowserViewInformation object to perform BrowserViewController operations.
@property(weak, nonatomic, readonly) id<BrowserViewInformation>
    browserViewInformation;

// The application window.
@property(nonatomic, strong) UIWindow* window;

// Contains information about the application state, for example whether the
// safe mode is activated.
@property(nonatomic, weak) AppState* appState;

// This metrics mediator is used to check and update the metrics accordingly to
// to the user preferences.
@property(nonatomic, weak) MetricsMediator* metricsMediator;

// UIResponder addition to execute a Chrome command.  Overridden in UIWindow to
// forward the call to the application delegate. The application delegate
// forwards the call to this class.
- (void)chromeExecuteCommand:(id)sender;

// Returns whether the tab switcher is active.
- (BOOL)isTabSwitcherActive;

@end

#endif  // IOS_CHROME_APP_MAIN_CONTROLLER_H_
