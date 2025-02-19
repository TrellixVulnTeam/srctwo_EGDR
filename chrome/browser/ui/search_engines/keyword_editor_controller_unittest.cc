// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/compiler_specific.h"
#include "base/memory/ptr_util.h"
#include "base/strings/string16.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/search_engines/template_url_service_factory_test_util.h"
#include "chrome/browser/search_engines/template_url_service_test_util.h"
#include "chrome/browser/ui/search_engines/keyword_editor_controller.h"
#include "chrome/browser/ui/search_engines/template_url_table_model.h"
#include "chrome/test/base/testing_profile.h"
#include "components/search_engines/template_url.h"
#include "components/search_engines/template_url_data.h"
#include "components/search_engines/template_url_service.h"
#include "components/sync_preferences/testing_pref_service_syncable.h"
#include "content/public/test/test_browser_thread_bundle.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/base/models/table_model_observer.h"

using base::ASCIIToUTF16;

static const base::string16 kA(ASCIIToUTF16("a"));
static const base::string16 kA1(ASCIIToUTF16("a1"));
static const base::string16 kB(ASCIIToUTF16("b"));
static const base::string16 kB1(ASCIIToUTF16("b1"));

// Base class for keyword editor tests. Creates a profile containing an
// empty TemplateURLService.
class KeywordEditorControllerTest : public testing::Test,
                                    public ui::TableModelObserver {
 public:
  KeywordEditorControllerTest()
      : util_(&profile_),
        simulate_load_failure_(false),
        model_changed_count_(0),
        items_changed_count_(0),
        added_count_(0),
        removed_count_(0) {}

  explicit KeywordEditorControllerTest(bool simulate_load_failure)
      : util_(&profile_),
        simulate_load_failure_(simulate_load_failure),
        model_changed_count_(0),
        items_changed_count_(0),
        added_count_(0),
        removed_count_(0) {}

  void SetUp() override {
    if (simulate_load_failure_)
      util_.model()->OnWebDataServiceRequestDone(0, nullptr);
    else
      util_.VerifyLoad();

    controller_.reset(new KeywordEditorController(&profile_));
    controller_->table_model()->SetObserver(this);
  }

  void TearDown() override { controller_.reset(); }

  void OnModelChanged() override { model_changed_count_++; }

  void OnItemsChanged(int start, int length) override {
    items_changed_count_++;
  }

  void OnItemsAdded(int start, int length) override { added_count_++; }

  void OnItemsRemoved(int start, int length) override { removed_count_++; }

  void VerifyChangeCount(int model_changed_count, int item_changed_count,
                         int added_count, int removed_count) {
    ASSERT_EQ(model_changed_count, model_changed_count_);
    ASSERT_EQ(item_changed_count, items_changed_count_);
    ASSERT_EQ(added_count, added_count_);
    ASSERT_EQ(removed_count, removed_count_);
    ClearChangeCount();
  }

  void ClearChangeCount() {
    model_changed_count_ = items_changed_count_ = added_count_ =
        removed_count_ = 0;
  }

  void SimulateDefaultSearchIsManaged(const std::string& url) {
    TemplateURLData managed_engine;
    managed_engine.SetShortName(ASCIIToUTF16("managed"));
    managed_engine.SetKeyword(ASCIIToUTF16("managed"));
    managed_engine.SetURL(url);
    SetManagedDefaultSearchPreferences(managed_engine, true, &profile_);
  }

  TemplateURLTableModel* table_model() { return controller_->table_model(); }
  KeywordEditorController* controller() { return controller_.get(); }
  const TemplateURLServiceFactoryTestUtil* util() const { return &util_; }

  int items_changed_count() const { return items_changed_count_; }
  int added_count() const { return added_count_; }
  int removed_count() const { return removed_count_; }

 private:
  content::TestBrowserThreadBundle thread_bundle_;
  TestingProfile profile_;
  std::unique_ptr<KeywordEditorController> controller_;
  TemplateURLServiceFactoryTestUtil util_;
  bool simulate_load_failure_;

  int model_changed_count_;
  int items_changed_count_;
  int added_count_;
  int removed_count_;
};

class KeywordEditorControllerNoWebDataTest
    : public KeywordEditorControllerTest {
 public:
  KeywordEditorControllerNoWebDataTest() : KeywordEditorControllerTest(true) {}
};

// Tests adding a TemplateURL.
TEST_F(KeywordEditorControllerTest, Add) {
  int original_row_count = table_model()->RowCount();
  controller()->AddTemplateURL(kA, kB, "http://c");

  // Verify the observer was notified.
  VerifyChangeCount(0, 0, 1, 0);
  if (HasFatalFailure())
    return;

  // Verify the TableModel has the new data.
  ASSERT_EQ(original_row_count + 1, table_model()->RowCount());

  // Verify the TemplateURLService has the new data.
  const TemplateURL* turl = util()->model()->GetTemplateURLForKeyword(kB);
  ASSERT_TRUE(turl);
  EXPECT_EQ(ASCIIToUTF16("a"), turl->short_name());
  EXPECT_EQ(ASCIIToUTF16("b"), turl->keyword());
  EXPECT_EQ("http://c", turl->url());
}

// Tests modifying a TemplateURL.
TEST_F(KeywordEditorControllerTest, Modify) {
  controller()->AddTemplateURL(kA, kB, "http://c");
  ClearChangeCount();

  // Modify the entry.
  TemplateURL* turl = util()->model()->GetTemplateURLs()[0];
  controller()->ModifyTemplateURL(turl, kA1, kB1, "http://c1");

  // Make sure it was updated appropriately.
  VerifyChangeCount(0, 1, 0, 0);
  EXPECT_EQ(ASCIIToUTF16("a1"), turl->short_name());
  EXPECT_EQ(ASCIIToUTF16("b1"), turl->keyword());
  EXPECT_EQ("http://c1", turl->url());
}

// Tests making a TemplateURL the default search provider.
TEST_F(KeywordEditorControllerTest, MakeDefault) {
  int index = controller()->AddTemplateURL(kA, kB, "http://c{searchTerms}");
  ClearChangeCount();

  const TemplateURL* turl = util()->model()->GetTemplateURLForKeyword(kB);
  int new_default = controller()->MakeDefaultTemplateURL(index);
  EXPECT_EQ(index, new_default);
  // Making an item the default sends a handful of changes. Which are sent isn't
  // important, what is important is 'something' is sent.
  ASSERT_TRUE(items_changed_count() > 0 || added_count() > 0 ||
              removed_count() > 0);
  ASSERT_TRUE(util()->model()->GetDefaultSearchProvider() == turl);

  // Making it default a second time should fail.
  new_default = controller()->MakeDefaultTemplateURL(index);
  EXPECT_EQ(-1, new_default);
}

// Tests that a TemplateURL can't be made the default if the default search
// provider is managed via policy.
TEST_F(KeywordEditorControllerTest, CannotSetDefaultWhileManaged) {
  controller()->AddTemplateURL(kA, kB, "http://c{searchTerms}");
  controller()->AddTemplateURL(kA1, kB1, "http://d{searchTerms}");
  ClearChangeCount();

  const TemplateURL* turl1 =
      util()->model()->GetTemplateURLForKeyword(ASCIIToUTF16("b"));
  ASSERT_TRUE(turl1 != NULL);
  const TemplateURL* turl2 =
      util()->model()->GetTemplateURLForKeyword(ASCIIToUTF16("b1"));
  ASSERT_TRUE(turl2 != NULL);

  EXPECT_TRUE(controller()->CanMakeDefault(turl1));
  EXPECT_TRUE(controller()->CanMakeDefault(turl2));

  SimulateDefaultSearchIsManaged(turl2->url());
  EXPECT_TRUE(util()->model()->is_default_search_managed());

  EXPECT_FALSE(controller()->CanMakeDefault(turl1));
  EXPECT_FALSE(controller()->CanMakeDefault(turl2));
}

// Tests that a TemplateURL can't be edited if it is the managed default search
// provider.
TEST_F(KeywordEditorControllerTest, EditManagedDefault) {
  controller()->AddTemplateURL(kA, kB, "http://c{searchTerms}");
  controller()->AddTemplateURL(kA1, kB1, "http://d{searchTerms}");
  ClearChangeCount();

  const TemplateURL* turl1 =
      util()->model()->GetTemplateURLForKeyword(ASCIIToUTF16("b"));
  ASSERT_TRUE(turl1 != NULL);
  const TemplateURL* turl2 =
      util()->model()->GetTemplateURLForKeyword(ASCIIToUTF16("b1"));
  ASSERT_TRUE(turl2 != NULL);

  EXPECT_TRUE(controller()->CanEdit(turl1));
  EXPECT_TRUE(controller()->CanEdit(turl2));

  // Simulate setting a managed default.  This will add another template URL to
  // the model.
  SimulateDefaultSearchIsManaged(turl2->url());
  EXPECT_TRUE(util()->model()->is_default_search_managed());
  EXPECT_TRUE(controller()->CanEdit(turl1));
  EXPECT_TRUE(controller()->CanEdit(turl2));
  EXPECT_FALSE(
      controller()->CanEdit(util()->model()->GetDefaultSearchProvider()));
}

TEST_F(KeywordEditorControllerNoWebDataTest, MakeDefaultNoWebData) {
  int index = controller()->AddTemplateURL(kA, kB, "http://c{searchTerms}");
  ClearChangeCount();

  // This should not result in a crash.
  int new_default = controller()->MakeDefaultTemplateURL(index);
  EXPECT_EQ(index, new_default);
}

// Mutates the TemplateURLService and make sure table model is updating
// appropriately.
TEST_F(KeywordEditorControllerTest, MutateTemplateURLService) {
  int original_row_count = table_model()->RowCount();

  TemplateURLData data;
  data.SetShortName(ASCIIToUTF16("b"));
  data.SetKeyword(ASCIIToUTF16("a"));
  TemplateURL* turl = util()->model()->Add(base::MakeUnique<TemplateURL>(data));

  // Table model should have updated.
  VerifyChangeCount(1, 0, 0, 0);

  // And should contain the newly added TemplateURL.
  ASSERT_EQ(original_row_count + 1, table_model()->RowCount());
  ASSERT_GE(table_model()->IndexOfTemplateURL(turl), 0);
}
