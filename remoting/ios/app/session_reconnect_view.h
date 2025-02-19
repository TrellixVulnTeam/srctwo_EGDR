// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_IOS_SESSON_RECONNECT_VIEW_H_
#define REMOTING_IOS_SESSON_RECONNECT_VIEW_H_

#import <UIKit/UIKit.h>

@protocol SessionReconnectViewDelegate<NSObject>

// Notifies the delegate that the user tapped the reconnect button.
@optional
- (void)didTapReconnect;

@end

// This view is the container for a session connection error. It will display a
// reconnect button.
@interface SessionReconnectView : UIView

// This delegate will handle interactions on the view.
@property(weak, nonatomic) id<SessionReconnectViewDelegate> delegate;

// This is the optional error text to be displayed above the reconnect button.
@property(nonatomic, copy) NSString* errorText;

@end

#endif  // REMOTING_IOS_SESSON_RECONNECT_VIEW_H_
