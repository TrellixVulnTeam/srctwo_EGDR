// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/css/invalidation/InvalidationSet.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

// Once we setWholeSubtreeInvalid, we should not keep the HashSets.
TEST(InvalidationSetTest, SubtreeInvalid_AddBefore) {
  RefPtr<InvalidationSet> set = DescendantInvalidationSet::Create();
  set->AddClass("a");
  set->SetWholeSubtreeInvalid();

  ASSERT_TRUE(set->IsEmpty());
}

// Don't (re)create HashSets if we've already setWholeSubtreeInvalid.
TEST(InvalidationSetTest, SubtreeInvalid_AddAfter) {
  RefPtr<InvalidationSet> set = DescendantInvalidationSet::Create();
  set->SetWholeSubtreeInvalid();
  set->AddTagName("a");

  ASSERT_TRUE(set->IsEmpty());
}

// No need to keep the HashSets when combining with a wholeSubtreeInvalid set.
TEST(InvalidationSetTest, SubtreeInvalid_Combine_1) {
  RefPtr<DescendantInvalidationSet> set1 = DescendantInvalidationSet::Create();
  RefPtr<DescendantInvalidationSet> set2 = DescendantInvalidationSet::Create();

  set1->AddId("a");
  set2->SetWholeSubtreeInvalid();

  set1->Combine(*set2);

  ASSERT_TRUE(set1->WholeSubtreeInvalid());
  ASSERT_TRUE(set1->IsEmpty());
}

// No need to add HashSets from combining set when we already have
// wholeSubtreeInvalid.
TEST(InvalidationSetTest, SubtreeInvalid_Combine_2) {
  RefPtr<DescendantInvalidationSet> set1 = DescendantInvalidationSet::Create();
  RefPtr<DescendantInvalidationSet> set2 = DescendantInvalidationSet::Create();

  set1->SetWholeSubtreeInvalid();
  set2->AddAttribute("a");

  set1->Combine(*set2);

  ASSERT_TRUE(set1->WholeSubtreeInvalid());
  ASSERT_TRUE(set1->IsEmpty());
}

TEST(InvalidationSetTest, SubtreeInvalid_AddCustomPseudoBefore) {
  RefPtr<InvalidationSet> set = DescendantInvalidationSet::Create();
  set->SetCustomPseudoInvalid();
  ASSERT_FALSE(set->IsEmpty());

  set->SetWholeSubtreeInvalid();
  ASSERT_TRUE(set->IsEmpty());
}

#ifndef NDEBUG
TEST(InvalidationSetTest, ShowDebug) {
  RefPtr<InvalidationSet> set = DescendantInvalidationSet::Create();
  set->Show();
}
#endif  // NDEBUG

}  // namespace blink
