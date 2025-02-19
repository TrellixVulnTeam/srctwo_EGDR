// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_IOS_APP_HOST_COLLECTION_VIEW_CELL_H_
#define REMOTING_IOS_APP_HOST_COLLECTION_VIEW_CELL_H_

#import <UIKit/UIKit.h>

#import "ios/third_party/material_components_ios/src/components/Collections/src/MaterialCollections.h"

@class HostInfo;

@interface HostCollectionViewCell : MDCCollectionViewCell

// Update a cell with host info. Typically called for cell reuse.
- (void)populateContentWithHostInfo:(HostInfo*)hostInfo;

@property(readonly, nonatomic) HostInfo* hostInfo;

@end

#endif  // REMOTING_IOS_APP_HOST_COLLECTION_VIEW_CELL_H_
