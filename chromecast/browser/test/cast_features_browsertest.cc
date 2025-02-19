// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromecast/base/cast_features.h"

#include <cstdint>
#include <unordered_set>

#include "base/feature_list.h"
#include "base/macros.h"
#include "base/metrics/field_trial_params.h"
#include "chromecast/base/pref_names.h"
#include "chromecast/browser/cast_browser_process.h"
#include "chromecast/browser/test/cast_browser_test.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/scoped_user_pref_update.h"

// PLEASE READ:
// 1) These tests are run in groups to simulate a restart of cast_shell. Each
//    test (ex. TestFoo) is accompanied by one or more tests which are
//    guaranteed to run before it (ex. PRE_TestFoo, PRE_PRE_TestFoo). PRE_ is a
//    special prefix used by content::BrowserTest to enforce test ordering among
//    tests of the same name. If a prefixed test fails, the unprefixed test
//    will not run. The tests use this behavior to test persisted state between
//    subsequent cast_shell runs. HOWEVER, tests which do not share a common
//    name have NO guarantee of ordering (ex. TestBar may run before, after, or
//    during PRE_TestFoo). Therefore, tests with different names should be
//    entirely independent.
//
// 2) Because of the last note in (1), when testing persistent features, do not
//    re-use the same feature in more than one set of tests. Because the order
//    of execution is not guaranteed between test sets, this may cause the tests
//    to flake, depending on how gtest decides to schedule them.
//    For example, TestFoo and TestBar should not both try to read and write
//    kTestFeat1 in the PrefStore. This makes the tests flaky and co-dependent.
//
// 3) Generally, the model should look like this:
//
//      IN_PROC_BROWSER_TEST_F(CastFeaturesBrowserTest, PRE_PRE_TestFoo) {
//        // Reset the state of the features.
//        CleareFeaturesFromPrefs({kFooFeature1, kFooFeature2});
//      }
//
//      IN_PROC_BROWSER_TEST_F(CastFeaturesBrowserTest, PRE_TestFoo) {
//        // Test the default state of the features...
//        EXPECT_TRUE(...);
//
//        // ... then persist overrides to disk.
//        SetFeatures(features_dict);  // contains override for kFooFeature1
//      }
//
//      IN_PROC_BROWSER_TEST_F(CastFeaturesBrowserTest, TestFoo) {
//        // Test that the feature override was properly persisted.
//        EXPECT_FALSE(...);
//      }
//
//    Note that in the example above, kFooFeatureX should not be used in tests
//    other than (PRE_)*FooTest, to ensure test indepdendence. An unrelated
//    BarTest should use features like KBarFeatureX.
//

namespace chromecast {
namespace shell {
namespace {

// Note: When adding new features for existing tests, please extend on the
// current block of features, by adding another sequentially-numbered feature.
// When adding a new test, please add a new block of features, the numbering for
// which is based at the next available multiple of 10.

// For use in TestFeaturesActivateOnBoot only.
const base::Feature kTestFeat1{"test_feat_1",
                               base::FEATURE_DISABLED_BY_DEFAULT};
const base::Feature kTestFeat2{"test_feat_2", base::FEATURE_ENABLED_BY_DEFAULT};
const base::Feature kTestFeat3{"test_feat_3",
                               base::FEATURE_DISABLED_BY_DEFAULT};
const base::Feature kTestFeat4{"test_feat_4", base::FEATURE_ENABLED_BY_DEFAULT};

// For use in TestParamsActivateOnBoot only.
const base::Feature kTestFeat11{"test_feat_11",
                                base::FEATURE_DISABLED_BY_DEFAULT};

// For use in TestOnlyWellFormedFeaturesPersisted only.
const base::Feature kTestFeat21{"test_feat_21",
                                base::FEATURE_DISABLED_BY_DEFAULT};
const base::Feature kTestFeat22{"test_feat_22",
                                base::FEATURE_ENABLED_BY_DEFAULT};
const base::Feature kTestFeat23{"test_feat_23",
                                base::FEATURE_DISABLED_BY_DEFAULT};
const base::Feature kTestFeat24{"test_feat_24",
                                base::FEATURE_ENABLED_BY_DEFAULT};

}  // namespace

class CastFeaturesBrowserTest : public CastBrowserTest {
 public:
  CastFeaturesBrowserTest() {}
  ~CastFeaturesBrowserTest() override {}

  static PrefService* pref_service() {
    return CastBrowserProcess::GetInstance()->pref_service();
  }

  // Write |dcs_features| to the pref store. This method is intended to be
  // overridden in internal test to utilize the real production codepath for
  // setting features from the server.
  virtual void SetFeatures(const base::DictionaryValue& dcs_features) {
    auto pref_features = GetOverriddenFeaturesForStorage(dcs_features);
    ScopedUserPrefUpdate<base::DictionaryValue, base::Value::Type::DICTIONARY>
        dict(pref_service(), prefs::kLatestDCSFeatures);
    dict->MergeDictionary(&pref_features);
    pref_service()->CommitPendingWrite();
  }

  // Clears |features| from the PrefStore. Should be called in a PRE_PRE_*
  // method for any tested feature in a test to ensure consistent state.
  void ClearFeaturesFromPrefs(std::vector<base::Feature> features) {
    ScopedUserPrefUpdate<base::DictionaryValue, base::Value::Type::DICTIONARY>
        dict(pref_service(), prefs::kLatestDCSFeatures);
    for (auto f : features)
      dict->Remove(f.name, nullptr);
    pref_service()->CommitPendingWrite();
  }

  // Write |experiment_ids| to the pref store. This method is intended to be
  // overridden in internal test to utilize the real production codepath for
  // setting features from the server.
  virtual void SetExperimentIds(
      const std::unordered_set<int32_t>& experiment_ids) {
    base::ListValue list;
    for (auto id : experiment_ids)
      list.AppendInteger(id);
    pref_service()->Set(prefs::kActiveDCSExperiments, list);
    pref_service()->CommitPendingWrite();
  }

  // Clear the set experiment id's.
  void ClearExperimentIds() {
    pref_service()->Set(prefs::kActiveDCSExperiments, base::ListValue());
    pref_service()->CommitPendingWrite();
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(CastFeaturesBrowserTest);
};

// Test that set features activate on the next boot. Part 1 of 3.
IN_PROC_BROWSER_TEST_F(CastFeaturesBrowserTest,
                       PRE_PRE_TestFeaturesActivateOnBoot) {
  ClearFeaturesFromPrefs({kTestFeat1, kTestFeat2, kTestFeat3, kTestFeat4});
}

// Test that set features activate on the next boot. Part 2 of 3.
IN_PROC_BROWSER_TEST_F(CastFeaturesBrowserTest,
                       PRE_TestFeaturesActivateOnBoot) {
  // Default values should be returned.
  ASSERT_FALSE(base::FeatureList::IsEnabled(kTestFeat1));
  ASSERT_TRUE(base::FeatureList::IsEnabled(kTestFeat2));
  ASSERT_FALSE(base::FeatureList::IsEnabled(kTestFeat3));
  ASSERT_TRUE(base::FeatureList::IsEnabled(kTestFeat4));

  // Set the features to be used on next boot.
  base::DictionaryValue features;
  features.SetBoolean("test_feat_1", true);
  features.SetBoolean("test_feat_4", false);
  SetFeatures(features);

  // Default values should still be returned until next boot.
  EXPECT_FALSE(base::FeatureList::IsEnabled(kTestFeat1));
  EXPECT_TRUE(base::FeatureList::IsEnabled(kTestFeat2));
  EXPECT_FALSE(base::FeatureList::IsEnabled(kTestFeat3));
  EXPECT_TRUE(base::FeatureList::IsEnabled(kTestFeat4));
}

// Test that features activate on the next boot. Part 3 of 3.
IN_PROC_BROWSER_TEST_F(CastFeaturesBrowserTest, TestFeaturesActivateOnBoot) {
  // Overriden values set in test case above should be set.
  ASSERT_TRUE(base::FeatureList::IsEnabled(kTestFeat1));
  ASSERT_TRUE(base::FeatureList::IsEnabled(kTestFeat2));
  ASSERT_FALSE(base::FeatureList::IsEnabled(kTestFeat3));
  ASSERT_FALSE(base::FeatureList::IsEnabled(kTestFeat4));
}

// Test that features with params activate on boot. Part 1 of 3.
IN_PROC_BROWSER_TEST_F(CastFeaturesBrowserTest,
                       PRE_PRE_TestParamsActivateOnBoot) {
  ClearFeaturesFromPrefs({kTestFeat11});
}

// Test that features with params activate on boot. Part 2 of 3.
IN_PROC_BROWSER_TEST_F(CastFeaturesBrowserTest, PRE_TestParamsActivateOnBoot) {
  // Default value should be returned.
  ASSERT_FALSE(base::FeatureList::IsEnabled(kTestFeat11));

  // Set the features to be used on next boot.
  base::DictionaryValue features;
  auto params = base::MakeUnique<base::DictionaryValue>();
  params->SetBoolean("bool_param", true);
  params->SetBoolean("bool_param_2", false);
  params->SetString("str_param", "foo");
  params->SetDouble("doub_param", 3.14159);
  params->SetInteger("int_param", 76543);
  features.Set("test_feat_11", std::move(params));
  SetFeatures(features);

  // Default value should still be returned until next boot.
  EXPECT_FALSE(base::FeatureList::IsEnabled(kTestFeat11));
}

// Test that features with params activate on boot. Part 3 of 3.
IN_PROC_BROWSER_TEST_F(CastFeaturesBrowserTest, TestParamsActivateOnBoot) {
  // Check that the feature is now enabled.
  ASSERT_TRUE(base::FeatureList::IsEnabled(kTestFeat11));

  // Check that the params are populated and correct.
  ASSERT_TRUE(base::GetFieldTrialParamByFeatureAsBool(
      kTestFeat11, "bool_param", false /* default_value */));
  ASSERT_FALSE(base::GetFieldTrialParamByFeatureAsBool(
      kTestFeat11, "bool_param_2", true /* default_value */));
  ASSERT_EQ("foo",
            base::GetFieldTrialParamValueByFeature(kTestFeat11, "str_param"));
  ASSERT_EQ(76543, base::GetFieldTrialParamByFeatureAsInt(
                       kTestFeat11, "int_param", 0 /* default_value */));
  ASSERT_EQ(3.14159, base::GetFieldTrialParamByFeatureAsDouble(
                         kTestFeat11, "doub_param", 0.0 /* default_value */));

  // Check that no extra parameters are set.
  std::map<std::string, std::string> params_out;
  ASSERT_TRUE(base::GetFieldTrialParamsByFeature(kTestFeat11, &params_out));
  ASSERT_EQ(5u, params_out.size());
}

// Test that only well-formed features are persisted to disk. Part 1 of 3.
IN_PROC_BROWSER_TEST_F(CastFeaturesBrowserTest,
                       PRE_PRE_TestOnlyWellFormedFeaturesPersisted) {
  ClearFeaturesFromPrefs({kTestFeat21, kTestFeat22, kTestFeat23, kTestFeat24});
}

// Test that only well-formed features are persisted to disk. Part 2 of 3.
IN_PROC_BROWSER_TEST_F(CastFeaturesBrowserTest,
                       PRE_TestOnlyWellFormedFeaturesPersisted) {
  // Default values should be returned.
  ASSERT_FALSE(base::FeatureList::IsEnabled(kTestFeat21));
  ASSERT_TRUE(base::FeatureList::IsEnabled(kTestFeat22));
  ASSERT_FALSE(base::FeatureList::IsEnabled(kTestFeat23));
  ASSERT_TRUE(base::FeatureList::IsEnabled(kTestFeat24));

  // Set both good parameters...
  base::DictionaryValue features;
  features.SetBoolean("test_feat_21", true);
  features.SetBoolean("test_feat_24", false);

  // ... and bad parameters.
  features.SetString("test_feat_22", "False");
  features.Set("test_feat_23", base::MakeUnique<base::ListValue>());

  SetFeatures(features);
}

// Test that only well-formed features are persisted to disk. Part 2 of 2.
IN_PROC_BROWSER_TEST_F(CastFeaturesBrowserTest,
                       TestOnlyWellFormedFeaturesPersisted) {
  // Only the well-formed parameters should be overriden.
  ASSERT_TRUE(base::FeatureList::IsEnabled(kTestFeat21));
  ASSERT_FALSE(base::FeatureList::IsEnabled(kTestFeat24));

  // The other should take default values.
  ASSERT_TRUE(base::FeatureList::IsEnabled(kTestFeat22));
  ASSERT_FALSE(base::FeatureList::IsEnabled(kTestFeat23));
}

// Test that experiment ids are persisted to disk. Part 1 of 3.
IN_PROC_BROWSER_TEST_F(CastFeaturesBrowserTest,
                       PRE_PRE_TestExperimentIdsPersisted) {
  ClearExperimentIds();
}

// Test that experiment ids are persisted to disk. Part 2 of 3.
IN_PROC_BROWSER_TEST_F(CastFeaturesBrowserTest,
                       PRE_TestExperimentIdsPersisted) {
  // No experiments should be set.
  ASSERT_TRUE(GetDCSExperimentIds().empty());

  // Set a test list of experiments.
  std::unordered_set<int32_t> ids({1234, 1, 4000});
  SetExperimentIds(ids);

  // No experiments should be set, still.
  ASSERT_TRUE(GetDCSExperimentIds().empty());
}

#if defined(OS_LINUX)
#define MAYBE_TestExperimentIdsPersisted DISABLED_TestExperimentIdsPersisted
#else
#define MAYBE_TestExperimentIdsPersisted TestExperimentIdsPersisted
#endif
// Test that experiment ids are persisted to disk. Part 3 of 3.
IN_PROC_BROWSER_TEST_F(CastFeaturesBrowserTest,
                       MAYBE_TestExperimentIdsPersisted) {
  // The experiments set in the last run should be active now.
  std::unordered_set<int32_t> ids({1234, 1, 4000});
  ASSERT_EQ(ids, GetDCSExperimentIds());
}

}  // namespace shell
}  // namespace chromecast
