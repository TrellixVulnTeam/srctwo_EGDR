// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_UI_AUTHENTICATION_SIGNIN_PROMO_ITEM_H_
#define IOS_CHROME_BROWSER_UI_AUTHENTICATION_SIGNIN_PROMO_ITEM_H_

#import <UIKit/UIKit.h>

#import "ios/chrome/browser/ui/collection_view/cells/collection_view_item.h"
#import "ios/third_party/material_components_ios/src/components/CollectionCells/src/MaterialCollectionCells.h"

@class SigninPromoView;
@class SigninPromoViewConfigurator;

// SigninPromoItem is an item that configures a SigninPromoCell cell.
@interface SigninPromoItem : CollectionViewItem

// Configures the SigninPromoView view from SigninPromoCell.
@property(nonatomic, strong) SigninPromoViewConfigurator* configurator;

@end

// Cell representation for SigninPromoItem. The cell contains only a
// SigninPromoView view.
@interface SigninPromoCell : MDCCollectionViewCell

@property(nonatomic, strong) SigninPromoView* signinPromoView;

@end

#endif  // IOS_CHROME_BROWSER_UI_AUTHENTICATION_SIGNIN_PROMO_ITEM_H_
