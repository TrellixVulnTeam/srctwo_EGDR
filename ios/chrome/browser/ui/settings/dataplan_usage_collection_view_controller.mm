// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/settings/dataplan_usage_collection_view_controller.h"

#include "base/logging.h"
#import "base/mac/foundation_util.h"
#include "components/prefs/pref_member.h"
#include "components/prefs/pref_service.h"
#import "ios/chrome/browser/ui/collection_view/cells/collection_view_text_item.h"
#import "ios/chrome/browser/ui/collection_view/collection_view_model.h"
#include "ios/chrome/grit/ios_strings.h"
#import "ios/third_party/material_components_ios/src/components/CollectionCells/src/MaterialCollectionCells.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/l10n/l10n_util_mac.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

namespace {

typedef NS_ENUM(NSInteger, SectionIdentifier) {
  SectionIdentifierOptions = kSectionIdentifierEnumZero,
};

typedef NS_ENUM(NSInteger, ItemType) {
  ItemTypeOptionsAlways = kItemTypeEnumZero,
  ItemTypeOptionsOnlyOnWiFi,
  ItemTypeOptionsNever,
};

}  // namespace.

@interface DataplanUsageCollectionViewController () {
  BooleanPrefMember basePreference_;
  BooleanPrefMember wifiPreference_;
}

// Updates the checked state of the cells to match the preferences.
- (void)updateCheckedState;

// Updates the PrefService with the given values.
- (void)updateBasePref:(BOOL)basePref wifiPref:(BOOL)wifiPref;
@end

@implementation DataplanUsageCollectionViewController

#pragma mark - Initialization

- (instancetype)initWithPrefs:(PrefService*)prefs
                     basePref:(const char*)basePreference
                     wifiPref:(const char*)wifiPreference
                        title:(NSString*)title {
  UICollectionViewLayout* layout = [[MDCCollectionViewFlowLayout alloc] init];
  self =
      [super initWithLayout:layout style:CollectionViewControllerStyleAppBar];
  if (self) {
    self.title = title;
    basePreference_.Init(basePreference, prefs);
    wifiPreference_.Init(wifiPreference, prefs);
    [self loadModel];
  }
  return self;
}

- (void)loadModel {
  [super loadModel];
  CollectionViewModel* model = self.collectionViewModel;

  [model addSectionWithIdentifier:SectionIdentifierOptions];

  CollectionViewTextItem* always =
      [[CollectionViewTextItem alloc] initWithType:ItemTypeOptionsAlways];
  [always setText:l10n_util::GetNSString(IDS_IOS_OPTIONS_DATA_USAGE_ALWAYS)];
  [always setAccessibilityTraits:UIAccessibilityTraitButton];
  [model addItem:always toSectionWithIdentifier:SectionIdentifierOptions];

  CollectionViewTextItem* wifi =
      [[CollectionViewTextItem alloc] initWithType:ItemTypeOptionsOnlyOnWiFi];
  [wifi setText:l10n_util::GetNSString(IDS_IOS_OPTIONS_DATA_USAGE_ONLY_WIFI)];
  [wifi setAccessibilityTraits:UIAccessibilityTraitButton];
  [model addItem:wifi toSectionWithIdentifier:SectionIdentifierOptions];

  CollectionViewTextItem* never =
      [[CollectionViewTextItem alloc] initWithType:ItemTypeOptionsNever];
  [never setText:l10n_util::GetNSString(IDS_IOS_OPTIONS_DATA_USAGE_NEVER)];
  [never setAccessibilityTraits:UIAccessibilityTraitButton];
  [model addItem:never toSectionWithIdentifier:SectionIdentifierOptions];

  [self updateCheckedState];
}

- (void)updateCheckedState {
  BOOL basePrefOn = basePreference_.GetValue();
  BOOL wifiPrefOn = wifiPreference_.GetValue();
  CollectionViewModel* model = self.collectionViewModel;

  std::unordered_map<NSInteger, bool> optionsMap = {
      {ItemTypeOptionsAlways, basePrefOn && !wifiPrefOn},
      {ItemTypeOptionsOnlyOnWiFi, basePrefOn && wifiPrefOn},
      {ItemTypeOptionsNever, !basePrefOn},
  };

  NSMutableArray* modifiedItems = [NSMutableArray array];
  for (CollectionViewTextItem* item in
       [model itemsInSectionWithIdentifier:SectionIdentifierOptions]) {
    auto value = optionsMap.find(item.type);
    DCHECK(value != optionsMap.end());

    MDCCollectionViewCellAccessoryType desiredType =
        value->second ? MDCCollectionViewCellAccessoryCheckmark
                      : MDCCollectionViewCellAccessoryNone;
    if (item.accessoryType != desiredType) {
      item.accessoryType = desiredType;
      [modifiedItems addObject:item];
    }
  }

  [self reconfigureCellsForItems:modifiedItems];
}

- (void)updateBasePref:(BOOL)basePref wifiPref:(BOOL)wifiPref {
  basePreference_.SetValue(basePref);
  wifiPreference_.SetValue(wifiPref);
  [self updateCheckedState];
}

#pragma mark - Internal methods

+ (NSString*)currentLabelForPreference:(PrefService*)prefs
                              basePref:(const char*)basePreference
                              wifiPref:(const char*)wifiPreference {
  if (!prefs)
    return nil;
  if (prefs->GetBoolean(basePreference)) {
    if (prefs->GetBoolean(wifiPreference))
      return l10n_util::GetNSString(IDS_IOS_OPTIONS_DATA_USAGE_ONLY_WIFI);
    else
      return l10n_util::GetNSString(IDS_IOS_OPTIONS_DATA_USAGE_ALWAYS);
  }
  return l10n_util::GetNSString(IDS_IOS_OPTIONS_DATA_USAGE_NEVER);
}

#pragma mark - UICollectionViewDelegate

- (void)collectionView:(UICollectionView*)collectionView
    didSelectItemAtIndexPath:(NSIndexPath*)indexPath {
  [super collectionView:collectionView didSelectItemAtIndexPath:indexPath];

  NSInteger type = [self.collectionViewModel itemTypeForIndexPath:indexPath];
  switch (type) {
    case ItemTypeOptionsAlways:
      [self updateBasePref:YES wifiPref:NO];
      break;
    case ItemTypeOptionsOnlyOnWiFi:
      [self updateBasePref:YES wifiPref:YES];
      break;
    case ItemTypeOptionsNever:
      [self updateBasePref:NO wifiPref:NO];
      break;
  }
}

@end
