// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_UI_SETTINGS_CELLS_BYO_TEXTFIELD_ITEM_H_
#define IOS_CHROME_BROWSER_UI_SETTINGS_CELLS_BYO_TEXTFIELD_ITEM_H_

#import <UIKit/UIKit.h>

#import "ios/chrome/browser/ui/collection_view/cells/collection_view_item.h"
#import "ios/third_party/material_components_ios/src/components/CollectionCells/src/MaterialCollectionCells.h"

// Bring-your-own-text-field: Item that hosts a text field provided by the user
// of this class.
// Useful for adding a text field to a collection view, where the view may
// be scrolled out of view and back in.  By using this object, even if the
// collection view cell containing the text field is cleared and reused, the
// user-entered content remains unchanged by using the same text field.
// Not recommended for large models, as the text field is not reused.
@interface BYOTextFieldItem : CollectionViewItem

// The text field that will be installed in the cell during -configureCell:.
@property(nonatomic, strong) UITextField* textField;

@end

// Cell class associated to BYOTextFieldItem.
@interface BYOTextFieldCell : MDCCollectionViewCell
@end

#endif  // IOS_CHROME_BROWSER_UI_SETTINGS_CELLS_BYO_TEXTFIELD_ITEM_H_
