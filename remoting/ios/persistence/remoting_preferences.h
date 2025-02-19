// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_IOS_PERSISTENCE_REMOTING_PREFERENCES_H_
#define REMOTING_IOS_PERSISTENCE_REMOTING_PREFERENCES_H_

#import <Foundation/Foundation.h>

@class HostSettings;

// |RemotingPreferences| is the centralized place to ask for information about
// defaults and prefrences.
@interface RemotingPreferences : NSObject

- (HostSettings*)settingsForHost:(NSString*)hostId;
- (void)setSettings:(HostSettings*)settings forHost:(NSString*)hostId;

// Access to the singleton shared instance from this property.
@property(nonatomic, readonly, class) RemotingPreferences* instance;

@property(nonatomic) NSString* activeUserKey;

@end

#endif  // REMOTING_IOS_PERSISTENCE_REMOTING_PREFERENCES_H_
