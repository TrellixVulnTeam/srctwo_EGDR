// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_WEB_VIEW_SHELL_SHELL_VIEW_CONTROLLER_H_
#define IOS_WEB_VIEW_SHELL_SHELL_VIEW_CONTROLLER_H_

#import <ChromeWebView/ChromeWebView.h>
#import <UIKit/UIKit.h>

// Accessibility label added to the back button.
extern NSString* const kWebViewShellBackButtonAccessibilityLabel;
// Accessibility label added to the forward button.
extern NSString* const kWebViewShellForwardButtonAccessibilityLabel;
// Accessibility label added to the URL address text field.
extern NSString* const kWebViewShellAddressFieldAccessibilityLabel;
// Accessibility identifier added to the text field of JavaScript prompts.
extern NSString* const
    kWebViewShellJavaScriptDialogTextFieldAccessibiltyIdentifier;

// Implements the main UI for web_view_shell, including the toolbar and web
// view.
@interface ShellViewController : UIViewController

// CWV view which renders the web page.
@property(nonatomic, strong) CWVWebView* webView;

@end

#endif  // IOS_WEB_VIEW_SHELL_SHELL_VIEW_CONTROLLER_H_
