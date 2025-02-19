// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/chrome/browser/ui/collection_view/collection_view_model.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/mac/foundation_util.h"
#include "base/strings/string_piece.h"
#import "ios/chrome/browser/ui/collection_view/cells/collection_view_item.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/gtest_mac.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

@interface CollectionViewModel (Testing)
// Adds an item with the given type to the section with the given identifier.
// It is possible to add multiple items with the same type to the same section.
// Sharing types across sections is undefined behavior.
- (void)addItemWithType:(NSInteger)itemType
    toSectionWithIdentifier:(NSInteger)sectionIdentifier;
@end

@implementation CollectionViewModel (Testing)

- (void)addItemWithType:(NSInteger)itemType
    toSectionWithIdentifier:(NSInteger)sectionIdentifier {
  CollectionViewItem* item = [[CollectionViewItem alloc] initWithType:itemType];
  [self addItem:item toSectionWithIdentifier:sectionIdentifier];
}

@end

@interface TestCollectionViewItemSubclass : CollectionViewItem
@end
@implementation TestCollectionViewItemSubclass
@end

namespace {

typedef NS_ENUM(NSInteger, SectionIdentifier) {
  SectionIdentifierCheese = kSectionIdentifierEnumZero,
  SectionIdentifierWeasley,
};

typedef NS_ENUM(NSInteger, ItemType) {
  ItemTypeCheeseHeader = kItemTypeEnumZero,
  ItemTypeCheeseCheddar,
  ItemTypeCheeseGouda,
  ItemTypeCheesePepperJack,
  ItemTypeWeasleyRon,
  ItemTypeWeasleyGinny,
  ItemTypeWeasleyArthur,
  ItemTypeWeasleyFooter,
};

void LogSink(const char* file,
             int line,
             const base::StringPiece message,
             const base::StringPiece stack_trace) {
  // No-op.
}

// Test generic model boxing (check done at compilation time).
TEST(CollectionViewModelTest, GenericModelBoxing) {
  CollectionViewModel<TestCollectionViewItemSubclass*>* specificModel =
      [[CollectionViewModel alloc] init];

  // |generalModel| is a superclass of |specificModel|. So specificModel can be
  // boxed into generalModel, but not the other way around.
  // specificModel = generalModel would not compile.
  CollectionViewModel<CollectionViewItem*>* generalModel = specificModel;
  generalModel = nil;
}

TEST(CollectionViewModelTest, EmptyModel) {
  CollectionViewModel* model = [[CollectionViewModel alloc] init];

  // Check there are no items.
  EXPECT_EQ(NO, [model hasItemAtIndexPath:[NSIndexPath indexPathForItem:0
                                                              inSection:0]]);

  // Check the collection view data sourcing methods.
  EXPECT_EQ(0, [model numberOfSections]);
}

TEST(CollectionViewModelTest, SingleSection) {
  CollectionViewModel* model = [[CollectionViewModel alloc] init];

  [model addSectionWithIdentifier:SectionIdentifierCheese];
  [model addItemWithType:ItemTypeCheeseCheddar
      toSectionWithIdentifier:SectionIdentifierCheese];
  [model addItemWithType:ItemTypeCheeseGouda
      toSectionWithIdentifier:SectionIdentifierCheese];
  [model addItemWithType:ItemTypeCheesePepperJack
      toSectionWithIdentifier:SectionIdentifierCheese];

  // Check there are some items but not more.
  EXPECT_EQ(NO, [model hasItemAtIndexPath:nil]);
  EXPECT_EQ(YES, [model hasItemAtIndexPath:[NSIndexPath indexPathForItem:0
                                                               inSection:0]]);
  EXPECT_EQ(YES, [model hasItemAtIndexPath:[NSIndexPath indexPathForItem:2
                                                               inSection:0]]);
  EXPECT_EQ(NO, [model hasItemAtIndexPath:[NSIndexPath indexPathForItem:3
                                                              inSection:0]]);
  EXPECT_EQ(NO, [model hasItemAtIndexPath:[NSIndexPath indexPathForItem:0
                                                              inSection:1]]);

  // Check the collection view data sourcing methods.
  EXPECT_EQ(1, [model numberOfSections]);
  EXPECT_EQ(3, [model numberOfItemsInSection:0]);

  // Check the section identifier <-> section correspondance methods.
  EXPECT_EQ(SectionIdentifierCheese, [model sectionIdentifierForSection:0]);
  EXPECT_EQ(0, [model sectionForSectionIdentifier:SectionIdentifierCheese]);

  // Check the item type <-> item correspondance methods.
  EXPECT_EQ(ItemTypeCheeseCheddar,
            [model itemTypeForIndexPath:[NSIndexPath indexPathForItem:0
                                                            inSection:0]]);
  EXPECT_EQ(ItemTypeCheeseGouda,
            [model itemTypeForIndexPath:[NSIndexPath indexPathForItem:1
                                                            inSection:0]]);
  EXPECT_EQ(ItemTypeCheesePepperJack,
            [model itemTypeForIndexPath:[NSIndexPath indexPathForItem:2
                                                            inSection:0]]);
}

TEST(CollectionViewModelTest, SingleSectionWithMissingItems) {
  CollectionViewModel* model = [[CollectionViewModel alloc] init];

  [model addSectionWithIdentifier:SectionIdentifierCheese];
  [model addItemWithType:ItemTypeCheeseCheddar
      toSectionWithIdentifier:SectionIdentifierCheese];
  // "Gouda" is intentionally omitted.
  [model addItemWithType:ItemTypeCheesePepperJack
      toSectionWithIdentifier:SectionIdentifierCheese];

  // Check the item type <-> item correspondance methods.
  EXPECT_EQ(ItemTypeCheeseCheddar,
            [model itemTypeForIndexPath:[NSIndexPath indexPathForItem:0
                                                            inSection:0]]);
  EXPECT_EQ(ItemTypeCheesePepperJack,
            [model itemTypeForIndexPath:[NSIndexPath indexPathForItem:1
                                                            inSection:0]]);
}

TEST(CollectionViewModelTest, MultipleSections) {
  CollectionViewModel* model = [[CollectionViewModel alloc] init];

  [model addSectionWithIdentifier:SectionIdentifierCheese];
  // "Cheddar" and "Gouda" are intentionally omitted.
  [model addItemWithType:ItemTypeCheesePepperJack
      toSectionWithIdentifier:SectionIdentifierCheese];

  [model addSectionWithIdentifier:SectionIdentifierWeasley];
  // "Ron" is intentionally omitted.
  [model addItemWithType:ItemTypeWeasleyGinny
      toSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyArthur
      toSectionWithIdentifier:SectionIdentifierWeasley];

  // Check the collection view data sourcing methods.
  EXPECT_EQ(2, [model numberOfSections]);
  EXPECT_EQ(2, [model numberOfItemsInSection:1]);

  // Check the section identifier <-> section correspondance methods.
  EXPECT_EQ(SectionIdentifierCheese, [model sectionIdentifierForSection:0]);
  EXPECT_EQ(0, [model sectionForSectionIdentifier:SectionIdentifierCheese]);
  EXPECT_EQ(SectionIdentifierWeasley, [model sectionIdentifierForSection:1]);
  EXPECT_EQ(1, [model sectionForSectionIdentifier:SectionIdentifierWeasley]);

  // Check the item type <-> item correspondance methods.
  EXPECT_EQ(ItemTypeCheesePepperJack,
            [model itemTypeForIndexPath:[NSIndexPath indexPathForItem:0
                                                            inSection:0]]);
  EXPECT_EQ(ItemTypeWeasleyGinny,
            [model itemTypeForIndexPath:[NSIndexPath indexPathForItem:0
                                                            inSection:1]]);
  EXPECT_EQ(ItemTypeWeasleyArthur,
            [model itemTypeForIndexPath:[NSIndexPath indexPathForItem:1
                                                            inSection:1]]);
}

TEST(CollectionViewModelTest, GetIndexPathFromModelCoordinates) {
  CollectionViewModel* model = [[CollectionViewModel alloc] init];

  [model addSectionWithIdentifier:SectionIdentifierCheese];
  [model addItemWithType:ItemTypeCheesePepperJack
      toSectionWithIdentifier:SectionIdentifierCheese];
  [model addSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyGinny
      toSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyArthur
      toSectionWithIdentifier:SectionIdentifierWeasley];

  // Check the index path retrieval method for a single item.
  NSIndexPath* indexPath =
      [model indexPathForItemType:ItemTypeWeasleyGinny
                sectionIdentifier:SectionIdentifierWeasley];
  EXPECT_EQ(1, indexPath.section);
  EXPECT_EQ(0, indexPath.item);

  // Check the index path retrieval method for the first item.
  indexPath = [model indexPathForItemType:ItemTypeWeasleyGinny
                        sectionIdentifier:SectionIdentifierWeasley
                                  atIndex:0];
  EXPECT_EQ(1, indexPath.section);
  EXPECT_EQ(0, indexPath.item);
}

TEST(CollectionViewItemTest, RepeatedItems) {
  CollectionViewModel* model = [[CollectionViewModel alloc] init];

  [model addSectionWithIdentifier:SectionIdentifierCheese];
  [model addItemWithType:ItemTypeCheesePepperJack
      toSectionWithIdentifier:SectionIdentifierCheese];
  [model addSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyGinny
      toSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyArthur
      toSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyArthur
      toSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyArthur
      toSectionWithIdentifier:SectionIdentifierWeasley];

  NSIndexPath* indexPath =
      [model indexPathForItemType:ItemTypeWeasleyArthur
                sectionIdentifier:SectionIdentifierWeasley];

  // Check the index path retrieval method for a single item on a repeated item.
  EXPECT_EQ(1, indexPath.section);
  EXPECT_EQ(1, indexPath.item);

  // Check the index path retrieval method for a repeated item.
  indexPath = [model indexPathForItemType:ItemTypeWeasleyArthur
                        sectionIdentifier:SectionIdentifierWeasley
                                  atIndex:1];

  EXPECT_EQ(1, indexPath.section);
  EXPECT_EQ(2, indexPath.item);
}

TEST(CollectionViewModelTest, RepeatedItemIndex) {
  CollectionViewModel* model = [[CollectionViewModel alloc] init];

  [model addSectionWithIdentifier:SectionIdentifierCheese];
  [model addItemWithType:ItemTypeCheesePepperJack
      toSectionWithIdentifier:SectionIdentifierCheese];
  [model addSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyGinny
      toSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyArthur
      toSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyArthur
      toSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyArthur
      toSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyGinny
      toSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyArthur
      toSectionWithIdentifier:SectionIdentifierWeasley];

  // Check the index path <-> index in item type correspondance method.
  EXPECT_EQ(
      0U, [model indexInItemTypeForIndexPath:[NSIndexPath indexPathForItem:0
                                                                 inSection:0]]);
  EXPECT_EQ(
      0U, [model indexInItemTypeForIndexPath:[NSIndexPath indexPathForItem:1
                                                                 inSection:1]]);
  EXPECT_EQ(
      2U, [model indexInItemTypeForIndexPath:[NSIndexPath indexPathForItem:3
                                                                 inSection:1]]);
  EXPECT_EQ(
      3U, [model indexInItemTypeForIndexPath:[NSIndexPath indexPathForItem:5
                                                                 inSection:1]]);
}

TEST(CollectionViewModelTest, RetrieveAddedItem) {
  CollectionViewModel* model = [[CollectionViewModel alloc] init];

  [model addSectionWithIdentifier:SectionIdentifierCheese];
  CollectionViewItem* someItem =
      [[CollectionViewItem alloc] initWithType:ItemTypeCheeseGouda];
  [model addItem:someItem toSectionWithIdentifier:SectionIdentifierCheese];

  // Check that the item is the same in the model.
  EXPECT_EQ(someItem, [model itemAtIndexPath:[NSIndexPath indexPathForItem:0
                                                                 inSection:0]]);
}

TEST(CollectionViewModelTest, RetrieveItemsInSection) {
  CollectionViewModel* model = [[CollectionViewModel alloc] init];
  [model addSectionWithIdentifier:SectionIdentifierCheese];
  CollectionViewItem* cheddar =
      [[CollectionViewItem alloc] initWithType:ItemTypeCheeseCheddar];
  [model addItem:cheddar toSectionWithIdentifier:SectionIdentifierCheese];
  CollectionViewItem* pepperJack =
      [[CollectionViewItem alloc] initWithType:ItemTypeCheesePepperJack];
  [model addItem:pepperJack toSectionWithIdentifier:SectionIdentifierCheese];
  CollectionViewItem* gouda =
      [[CollectionViewItem alloc] initWithType:ItemTypeCheeseGouda];
  [model addItem:gouda toSectionWithIdentifier:SectionIdentifierCheese];

  NSArray* cheeseItems =
      [model itemsInSectionWithIdentifier:SectionIdentifierCheese];
  EXPECT_EQ(3U, [cheeseItems count]);
  EXPECT_NSEQ(cheddar, cheeseItems[0]);
  EXPECT_NSEQ(pepperJack, cheeseItems[1]);
  EXPECT_NSEQ(gouda, cheeseItems[2]);
}

TEST(CollectionViewModelTest, InvalidIndexPath) {
  CollectionViewModel* model = [[CollectionViewModel alloc] init];
  [model addSectionWithIdentifier:SectionIdentifierCheese];

  logging::ScopedLogAssertHandler scoped_assert_handler(base::Bind(LogSink));
  bool out_of_bounds_exception_thrown = false;
  @try {
    [model indexInItemTypeForIndexPath:[NSIndexPath indexPathForItem:0
                                                           inSection:0]];
  } @catch (NSException* exception) {
    if ([[exception name] isEqualToString:NSRangeException]) {
      out_of_bounds_exception_thrown = true;
    }
  }
  EXPECT_TRUE(out_of_bounds_exception_thrown);
}

TEST(CollectionViewModelTest, RemoveItems) {
  CollectionViewModel* model = [[CollectionViewModel alloc] init];

  [model addSectionWithIdentifier:SectionIdentifierCheese];
  [model addItemWithType:ItemTypeCheesePepperJack
      toSectionWithIdentifier:SectionIdentifierCheese];
  [model addItemWithType:ItemTypeCheeseGouda
      toSectionWithIdentifier:SectionIdentifierCheese];

  [model addSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyGinny
      toSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyArthur
      toSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyArthur
      toSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyArthur
      toSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyRon
      toSectionWithIdentifier:SectionIdentifierWeasley];

  [model removeItemWithType:ItemTypeCheesePepperJack
      fromSectionWithIdentifier:SectionIdentifierCheese];
  [model removeItemWithType:ItemTypeWeasleyGinny
      fromSectionWithIdentifier:SectionIdentifierWeasley];
  [model removeItemWithType:ItemTypeWeasleyArthur
      fromSectionWithIdentifier:SectionIdentifierWeasley
                        atIndex:2];

  // Check the collection view data sourcing methods.
  EXPECT_EQ(2, [model numberOfSections]);

  // Check the index path retrieval method for a single item.
  NSIndexPath* indexPath = [model indexPathForItemType:ItemTypeCheeseGouda
                                     sectionIdentifier:SectionIdentifierCheese];
  EXPECT_EQ(0, indexPath.section);
  EXPECT_EQ(0, indexPath.item);

  // Check the index path retrieval method for a repeated item.
  indexPath = [model indexPathForItemType:ItemTypeWeasleyArthur
                        sectionIdentifier:SectionIdentifierWeasley
                                  atIndex:1];
  EXPECT_EQ(1, indexPath.section);
  EXPECT_EQ(1, indexPath.item);

  // Check the index path retrieval method for a single item.
  indexPath = [model indexPathForItemType:ItemTypeWeasleyRon
                        sectionIdentifier:SectionIdentifierWeasley];
  EXPECT_EQ(1, indexPath.section);
  EXPECT_EQ(2, indexPath.item);
}

TEST(CollectionViewModelTest, RemoveSections) {
  CollectionViewModel* model = [[CollectionViewModel alloc] init];

  // Empty section.
  [model addSectionWithIdentifier:SectionIdentifierWeasley];

  // Section with items.
  [model addSectionWithIdentifier:SectionIdentifierCheese];
  [model addItemWithType:ItemTypeCheesePepperJack
      toSectionWithIdentifier:SectionIdentifierCheese];
  [model addItemWithType:ItemTypeCheeseGouda
      toSectionWithIdentifier:SectionIdentifierCheese];

  // Check the collection view data sourcing methods.
  EXPECT_EQ(2, [model numberOfSections]);
  EXPECT_EQ(0, [model numberOfItemsInSection:0]);
  EXPECT_EQ(2, [model numberOfItemsInSection:1]);

  // Remove an empty section.
  [model removeSectionWithIdentifier:SectionIdentifierWeasley];

  // Check that the section was removed.
  EXPECT_EQ(1, [model numberOfSections]);
  EXPECT_EQ(2, [model numberOfItemsInSection:0]);

  // Remove a section with items.
  [model removeSectionWithIdentifier:SectionIdentifierCheese];

  // Check that the section and its items were removed.
  EXPECT_EQ(0, [model numberOfSections]);
}

TEST(CollectionViewModelTest, QueryItemsFromModelCoordinates) {
  CollectionViewModel* model = [[CollectionViewModel alloc] init];

  EXPECT_FALSE([model hasSectionForSectionIdentifier:SectionIdentifierWeasley]);
  EXPECT_FALSE([model hasItemForItemType:ItemTypeCheeseCheddar
                       sectionIdentifier:SectionIdentifierCheese]);
  EXPECT_FALSE([model hasItemForItemType:ItemTypeCheeseGouda
                       sectionIdentifier:SectionIdentifierCheese
                                 atIndex:1]);

  // Section with items.
  [model addSectionWithIdentifier:SectionIdentifierCheese];
  [model addItemWithType:ItemTypeCheesePepperJack
      toSectionWithIdentifier:SectionIdentifierCheese];
  [model addItemWithType:ItemTypeCheeseGouda
      toSectionWithIdentifier:SectionIdentifierCheese];
  [model addItemWithType:ItemTypeCheeseGouda
      toSectionWithIdentifier:SectionIdentifierCheese];

  EXPECT_TRUE([model hasSectionForSectionIdentifier:SectionIdentifierCheese]);
  EXPECT_FALSE([model hasItemForItemType:ItemTypeCheeseCheddar
                       sectionIdentifier:SectionIdentifierCheese]);
  EXPECT_TRUE([model hasItemForItemType:ItemTypeCheesePepperJack
                      sectionIdentifier:SectionIdentifierCheese]);
  EXPECT_TRUE([model hasItemForItemType:ItemTypeCheeseGouda
                      sectionIdentifier:SectionIdentifierCheese
                                atIndex:1]);
}

// Tests that inserted sections are added at the correct index.
TEST(CollectionViewModelTest, InsertSections) {
  CollectionViewModel* model = [[CollectionViewModel alloc] init];

  [model addSectionWithIdentifier:SectionIdentifierWeasley];
  EXPECT_EQ(1, [model numberOfSections]);
  EXPECT_EQ(0, [model sectionForSectionIdentifier:SectionIdentifierWeasley]);

  [model insertSectionWithIdentifier:SectionIdentifierCheese atIndex:0];
  EXPECT_EQ(2, [model numberOfSections]);
  EXPECT_EQ(1, [model sectionForSectionIdentifier:SectionIdentifierWeasley]);
  EXPECT_EQ(0, [model sectionForSectionIdentifier:SectionIdentifierCheese]);

  [model removeSectionWithIdentifier:SectionIdentifierCheese];
  [model insertSectionWithIdentifier:SectionIdentifierCheese atIndex:1];
  EXPECT_EQ(2, [model numberOfSections]);
  EXPECT_EQ(0, [model sectionForSectionIdentifier:SectionIdentifierWeasley]);
  EXPECT_EQ(1, [model sectionForSectionIdentifier:SectionIdentifierCheese]);
}

// Tests that inserted items are added at the correct index.
TEST(CollectionViewModelTest, InsertItemAtIndex) {
  CollectionViewModel* model = [[CollectionViewModel alloc] init];

  [model addSectionWithIdentifier:SectionIdentifierCheese];
  [model addItemWithType:ItemTypeCheesePepperJack
      toSectionWithIdentifier:SectionIdentifierCheese];
  [model addItemWithType:ItemTypeCheeseGouda
      toSectionWithIdentifier:SectionIdentifierCheese];
  CollectionViewItem* cheddarItem =
      [[CollectionViewItem alloc] initWithType:ItemTypeCheeseCheddar];
  [model insertItem:cheddarItem
      inSectionWithIdentifier:SectionIdentifierCheese
                      atIndex:1];

  EXPECT_EQ(1, [model numberOfSections]);

  NSIndexPath* pepperJackIndexPath =
      [model indexPathForItemType:ItemTypeCheesePepperJack
                sectionIdentifier:SectionIdentifierCheese];
  EXPECT_EQ(0, pepperJackIndexPath.section);
  EXPECT_EQ(0, pepperJackIndexPath.item);

  NSIndexPath* cheddarIndexPath =
      [model indexPathForItemType:ItemTypeCheeseCheddar
                sectionIdentifier:SectionIdentifierCheese];
  EXPECT_EQ(0, cheddarIndexPath.section);
  EXPECT_EQ(1, cheddarIndexPath.item);

  NSIndexPath* goudaIndexPath =
      [model indexPathForItemType:ItemTypeCheeseGouda
                sectionIdentifier:SectionIdentifierCheese];
  EXPECT_EQ(0, goudaIndexPath.section);
  EXPECT_EQ(2, goudaIndexPath.item);
}

TEST(CollectionViewModelTest, IndexPathsForItems) {
  CollectionViewModel* model = [[CollectionViewModel alloc] init];

  [model addSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyGinny
      toSectionWithIdentifier:SectionIdentifierWeasley];
  // Added at index 1.
  CollectionViewItem* item1 =
      [[CollectionViewItem alloc] initWithType:ItemTypeWeasleyRon];
  [model addItem:item1 toSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyGinny
      toSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyArthur
      toSectionWithIdentifier:SectionIdentifierWeasley];
  // Repeated item added at index 4.
  CollectionViewItem* item4 =
      [[CollectionViewItem alloc] initWithType:ItemTypeWeasleyArthur];
  [model addItem:item4 toSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyArthur
      toSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyArthur
      toSectionWithIdentifier:SectionIdentifierWeasley];
  // Item not added.
  CollectionViewItem* notAddedItem =
      [[CollectionViewItem alloc] initWithType:ItemTypeCheeseGouda];

  EXPECT_TRUE([model hasItem:item1]);
  NSIndexPath* indexPath1 = [model indexPathForItem:item1];
  EXPECT_EQ(0, indexPath1.section);
  EXPECT_EQ(1, indexPath1.item);

  EXPECT_TRUE([model hasItem:item4]);
  NSIndexPath* indexPath4 = [model indexPathForItem:item4];
  EXPECT_EQ(0, indexPath4.section);
  EXPECT_EQ(4, indexPath4.item);

  EXPECT_FALSE([model hasItem:notAddedItem]);
}

TEST(CollectionViewModelTest, Headers) {
  CollectionViewModel* model = [[CollectionViewModel alloc] init];

  [model addSectionWithIdentifier:SectionIdentifierCheese];
  CollectionViewItem* cheeseHeader =
      [[CollectionViewItem alloc] initWithType:ItemTypeCheeseHeader];
  [model setHeader:cheeseHeader
      forSectionWithIdentifier:SectionIdentifierCheese];
  [model addItemWithType:ItemTypeCheeseGouda
      toSectionWithIdentifier:SectionIdentifierCheese];
  [model addItemWithType:ItemTypeCheeseCheddar
      toSectionWithIdentifier:SectionIdentifierCheese];
  [model addSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyRon
      toSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyGinny
      toSectionWithIdentifier:SectionIdentifierWeasley];

  NSInteger cheeseSection =
      [model sectionForSectionIdentifier:SectionIdentifierCheese];
  NSInteger weasleySection =
      [model sectionForSectionIdentifier:SectionIdentifierWeasley];

  EXPECT_EQ(cheeseHeader,
            [model headerForSectionWithIdentifier:SectionIdentifierCheese]);
  EXPECT_EQ(cheeseHeader, [model headerForSection:cheeseSection]);

  EXPECT_FALSE([model headerForSectionWithIdentifier:SectionIdentifierWeasley]);
  EXPECT_FALSE([model headerForSection:weasleySection]);
}

TEST(CollectionViewModelTest, Footers) {
  CollectionViewModel* model = [[CollectionViewModel alloc] init];

  [model addSectionWithIdentifier:SectionIdentifierCheese];
  [model addItemWithType:ItemTypeCheeseGouda
      toSectionWithIdentifier:SectionIdentifierCheese];
  [model addItemWithType:ItemTypeCheeseCheddar
      toSectionWithIdentifier:SectionIdentifierCheese];
  [model addSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyRon
      toSectionWithIdentifier:SectionIdentifierWeasley];
  [model addItemWithType:ItemTypeWeasleyGinny
      toSectionWithIdentifier:SectionIdentifierWeasley];
  CollectionViewItem* weasleyFooter =
      [[CollectionViewItem alloc] initWithType:ItemTypeWeasleyFooter];
  [model setFooter:weasleyFooter
      forSectionWithIdentifier:SectionIdentifierWeasley];

  NSInteger cheeseSection =
      [model sectionForSectionIdentifier:SectionIdentifierCheese];
  NSInteger weasleySection =
      [model sectionForSectionIdentifier:SectionIdentifierWeasley];

  EXPECT_FALSE([model footerForSectionWithIdentifier:SectionIdentifierCheese]);
  EXPECT_FALSE([model footerForSection:cheeseSection]);

  EXPECT_EQ(weasleyFooter,
            [model footerForSectionWithIdentifier:SectionIdentifierWeasley]);
  EXPECT_EQ(weasleyFooter, [model footerForSection:weasleySection]);
}

}  // namespace
