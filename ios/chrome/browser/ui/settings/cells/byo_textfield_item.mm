// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/settings/cells/byo_textfield_item.h"

#include "base/logging.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

namespace {
// Padding used on the leading and trailing edges of the cell.
const CGFloat kHorizontalPadding = 16;
}  // namespace

@interface BYOTextFieldCell ()

// The text field installed in this cell.
@property(nonatomic, strong) UITextField* textField;

// The constraints set on the textfield.
@property(nonatomic, strong) NSArray<NSLayoutConstraint*>* textFieldConstraints;

// Sets up the text field and displays it inside the cell.
- (void)installTextField:(UITextField*)textField;

@end

@implementation BYOTextFieldItem

@synthesize textField = _textField;

- (instancetype)initWithType:(NSInteger)type {
  self = [super initWithType:type];
  if (self) {
    self.cellClass = [BYOTextFieldCell class];
  }
  return self;
}

- (void)configureCell:(BYOTextFieldCell*)cell {
  [super configureCell:cell];
  [cell installTextField:self.textField];
}

@end

@implementation BYOTextFieldCell

@synthesize textField = _textField;
@synthesize textFieldConstraints = _textFieldConstraints;

- (void)installTextField:(UITextField*)textField {
  // Make sure not text field is still installed.
  [self uninstallTextField];

  // If there is no text field to install, return.
  if (!textField) {
    return;
  }

  // Store the text field.
  self.textField = textField;

  // Add it to the content view.
  textField.translatesAutoresizingMaskIntoConstraints = NO;
  UIView* contentView = self.contentView;
  [contentView addSubview:textField];

  // Store the constraints.
  self.textFieldConstraints = @[
    [textField.topAnchor constraintEqualToAnchor:contentView.topAnchor],
    [textField.bottomAnchor constraintEqualToAnchor:contentView.bottomAnchor],
    [textField.leadingAnchor constraintEqualToAnchor:contentView.leadingAnchor
                                            constant:kHorizontalPadding],
    [textField.trailingAnchor constraintEqualToAnchor:contentView.trailingAnchor
                                             constant:-kHorizontalPadding],
  ];

  // Activate the constraints.
  [NSLayoutConstraint activateConstraints:self.textFieldConstraints];
}

- (void)uninstallTextField {
  // Deactivate the constraints.
  [NSLayoutConstraint deactivateConstraints:self.textFieldConstraints];
  // Release the constraints.
  self.textFieldConstraints = nil;
  // Remove the text field from the content view.
  [self.textField removeFromSuperview];
  // Release the text field.
  self.textField = nil;
}

- (void)prepareForReuse {
  [super prepareForReuse];
  [self uninstallTextField];
}

@end
