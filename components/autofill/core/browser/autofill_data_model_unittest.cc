// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/core/browser/autofill_data_model.h"

#include <stddef.h>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/time/time.h"
#include "components/autofill/core/common/autofill_constants.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace autofill {

namespace {

// Provides concrete implementations for pure virtual methods.
class TestAutofillDataModel : public AutofillDataModel {
 public:
  TestAutofillDataModel(const std::string& guid, const std::string& origin)
      : AutofillDataModel(guid, origin) {}
  TestAutofillDataModel(const std::string& guid,
                        size_t use_count,
                        base::Time use_date)
      : AutofillDataModel(guid, std::string()) {
    set_use_count(use_count);
    set_use_date(use_date);
  }
  ~TestAutofillDataModel() override {}

 private:
  base::string16 GetRawInfo(ServerFieldType type) const override {
    return base::string16();
  }
  void SetRawInfo(ServerFieldType type, const base::string16& value) override {}
  void GetSupportedTypes(ServerFieldTypeSet* supported_types) const override {}

  DISALLOW_COPY_AND_ASSIGN(TestAutofillDataModel);
};

}  // namespace

TEST(AutofillDataModelTest, IsVerified) {
  TestAutofillDataModel model("guid", std::string());
  EXPECT_FALSE(model.IsVerified());

  model.set_origin("http://www.example.com");
  EXPECT_FALSE(model.IsVerified());

  model.set_origin("https://www.example.com");
  EXPECT_FALSE(model.IsVerified());

  model.set_origin("file:///tmp/example.txt");
  EXPECT_FALSE(model.IsVerified());

  model.set_origin("data:text/plain;charset=utf-8;base64,ZXhhbXBsZQ==");
  EXPECT_FALSE(model.IsVerified());

  model.set_origin("chrome://settings/autofill");
  EXPECT_FALSE(model.IsVerified());

  model.set_origin(kSettingsOrigin);
  EXPECT_TRUE(model.IsVerified());

  model.set_origin("Some gibberish string");
  EXPECT_TRUE(model.IsVerified());

  model.set_origin(std::string());
  EXPECT_FALSE(model.IsVerified());
}

enum Expectation { GREATER, LESS };
struct CompareFrecencyTestCase {
  const std::string guid_a;
  const int use_count_a;
  const base::Time use_date_a;
  const std::string guid_b;
  const int use_count_b;
  const base::Time use_date_b;
  Expectation expectation;
};

base::Time now = base::Time::Now();

class CompareFrecencyTest
    : public testing::TestWithParam<CompareFrecencyTestCase> {};

TEST_P(CompareFrecencyTest, CompareFrecency) {
  auto test_case = GetParam();
  TestAutofillDataModel model_a(test_case.guid_a, test_case.use_count_a,
                                test_case.use_date_a);
  TestAutofillDataModel model_b(test_case.guid_b, test_case.use_count_b,
                                test_case.use_date_b);

  EXPECT_EQ(test_case.expectation == GREATER,
            model_a.CompareFrecency(&model_b, now));
  EXPECT_NE(test_case.expectation == GREATER,
            model_b.CompareFrecency(&model_a, now));
}

INSTANTIATE_TEST_CASE_P(
    AutofillDataModelTest,
    CompareFrecencyTest,
    testing::Values(
        // Same frecency, model_a has a smaller GUID (tie breaker).
        CompareFrecencyTestCase{"guid_a", 8, now, "guid_b", 8, now, LESS},
        // Same recency, model_a has a bigger frequency.
        CompareFrecencyTestCase{"guid_a", 10, now, "guid_b", 8, now, GREATER},
        // Same recency, model_a has a smaller frequency.
        CompareFrecencyTestCase{"guid_a", 8, now, "guid_b", 10, now, LESS},
        // Same frequency, model_a is more recent.
        CompareFrecencyTestCase{"guid_a", 8, now, "guid_b", 8,
                                now - base::TimeDelta::FromDays(1), GREATER},
        // Same frequency, model_a is less recent.
        CompareFrecencyTestCase{"guid_a", 8, now - base::TimeDelta::FromDays(1),
                                "guid_b", 8, now, LESS},
        // Special case: occasional profiles. A profile with relatively low
        // usage and used recently (model_b) should not rank higher than a more
        // used profile that has been unused for a short amount of time
        // (model_a).
        CompareFrecencyTestCase{
            "guid_a", 300, now - base::TimeDelta::FromDays(5), "guid_b", 10,
            now - base::TimeDelta::FromDays(1), GREATER},
        // Special case: moving. A new profile used frequently (model_b) should
        // rank higher than a profile with more usage that has not been used for
        // a while (model_a).
        CompareFrecencyTestCase{"guid_a", 300,
                                now - base::TimeDelta::FromDays(15), "guid_b",
                                10, now - base::TimeDelta::FromDays(1), LESS}));

}  // namespace autofill
