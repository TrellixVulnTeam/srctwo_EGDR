// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/budget_service/budget_database.h"

#include <math.h>
#include <vector>

#include "base/memory/ptr_util.h"
#include "base/run_loop.h"
#include "base/test/histogram_tester.h"
#include "base/test/simple_test_clock.h"
#include "chrome/browser/budget_service/budget.pb.h"
#include "chrome/browser/engagement/site_engagement_score.h"
#include "chrome/browser/engagement/site_engagement_service.h"
#include "chrome/test/base/testing_profile.h"
#include "components/leveldb_proto/proto_database.h"
#include "components/leveldb_proto/proto_database_impl.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/test/test_browser_thread_bundle.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "url/gurl.h"
#include "url/origin.h"

namespace {

// These values mirror the defaults in budget_database.cc
const double kDefaultExpirationInDays = 4;
const double kMaxDailyBudget = 12;

const double kEngagement = 25;

const char kTestOrigin[] = "https://example.com";

}  // namespace

class BudgetDatabaseTest : public ::testing::Test {
 public:
  BudgetDatabaseTest()
      : success_(false),
        db_(&profile_,
            profile_.GetPath().Append(FILE_PATH_LITERAL("BudgetDatabase"))),
        origin_(url::Origin(GURL(kTestOrigin))) {}

  void WriteBudgetComplete(base::Closure run_loop_closure,
                           blink::mojom::BudgetServiceErrorType error,
                           bool success) {
    success_ = (error == blink::mojom::BudgetServiceErrorType::NONE) && success;
    run_loop_closure.Run();
  }

  // Spend budget for the origin.
  bool SpendBudget(double amount) {
    base::RunLoop run_loop;
    db_.SpendBudget(origin(), amount,
                    base::Bind(&BudgetDatabaseTest::WriteBudgetComplete,
                               base::Unretained(this), run_loop.QuitClosure()));
    run_loop.Run();
    return success_;
  }

  void GetBudgetDetailsComplete(
      base::Closure run_loop_closure,
      blink::mojom::BudgetServiceErrorType error,
      std::vector<blink::mojom::BudgetStatePtr> predictions) {
    success_ = (error == blink::mojom::BudgetServiceErrorType::NONE);
    prediction_.swap(predictions);
    run_loop_closure.Run();
  }

  // Get the full set of budget predictions for the origin.
  void GetBudgetDetails() {
    base::RunLoop run_loop;
    db_.GetBudgetDetails(
        origin(),
        base::BindOnce(&BudgetDatabaseTest::GetBudgetDetailsComplete,
                       base::Unretained(this), run_loop.QuitClosure()));
    run_loop.Run();
  }

  Profile* profile() { return &profile_; }
  BudgetDatabase* database() { return &db_; }
  const url::Origin& origin() const { return origin_; }

  // Setup a test clock so that the tests can control time.
  base::SimpleTestClock* SetClockForTesting() {
    base::SimpleTestClock* clock = new base::SimpleTestClock();
    db_.SetClockForTesting(base::WrapUnique(clock));
    return clock;
  }

  void SetSiteEngagementScore(double score) {
    SiteEngagementService* service = SiteEngagementService::Get(&profile_);
    service->ResetBaseScoreForURL(GURL(kTestOrigin), score);
  }

 protected:
  base::HistogramTester* GetHistogramTester() { return &histogram_tester_; }
  bool success_;
  std::vector<blink::mojom::BudgetStatePtr> prediction_;

 private:
  content::TestBrowserThreadBundle thread_bundle_;
  std::unique_ptr<budget_service::Budget> budget_;
  TestingProfile profile_;
  BudgetDatabase db_;
  base::HistogramTester histogram_tester_;
  const url::Origin origin_;
};

TEST_F(BudgetDatabaseTest, GetBudgetNoBudgetOrSES) {
  GetBudgetDetails();
  ASSERT_TRUE(success_);
  ASSERT_EQ(2U, prediction_.size());
  EXPECT_EQ(0, prediction_[0]->budget_at);
}

TEST_F(BudgetDatabaseTest, AddEngagementBudgetTest) {
  base::SimpleTestClock* clock = SetClockForTesting();
  base::Time expiration_time =
      clock->Now() + base::TimeDelta::FromDays(kDefaultExpirationInDays);

  // Set the default site engagement.
  SetSiteEngagementScore(kEngagement);

  // The budget should include kDefaultExpirationInDays days worth of
  // engagement.
  double daily_budget =
      kMaxDailyBudget * (kEngagement / SiteEngagementScore::kMaxPoints);
  GetBudgetDetails();
  ASSERT_TRUE(success_);
  ASSERT_EQ(2U, prediction_.size());
  ASSERT_DOUBLE_EQ(daily_budget * kDefaultExpirationInDays,
                   prediction_[0]->budget_at);
  ASSERT_EQ(0, prediction_[1]->budget_at);
  ASSERT_EQ(expiration_time.ToJsTime(), prediction_[1]->time);

  // Advance time 1 day and add more engagement budget.
  clock->Advance(base::TimeDelta::FromDays(1));
  GetBudgetDetails();

  // The budget should now have 1 full share plus 1 daily budget.
  ASSERT_TRUE(success_);
  ASSERT_EQ(3U, prediction_.size());
  ASSERT_DOUBLE_EQ(daily_budget * (kDefaultExpirationInDays + 1),
                   prediction_[0]->budget_at);
  ASSERT_DOUBLE_EQ(daily_budget, prediction_[1]->budget_at);
  ASSERT_EQ(expiration_time.ToJsTime(), prediction_[1]->time);
  ASSERT_DOUBLE_EQ(0, prediction_[2]->budget_at);
  ASSERT_EQ((expiration_time + base::TimeDelta::FromDays(1)).ToJsTime(),
            prediction_[2]->time);

  // Advance time by 59 minutes and check that no engagement budget is added
  // since budget should only be added for > 1 hour increments.
  clock->Advance(base::TimeDelta::FromMinutes(59));
  GetBudgetDetails();

  // The budget should be the same as before the attempted add.
  ASSERT_TRUE(success_);
  ASSERT_EQ(3U, prediction_.size());
  ASSERT_DOUBLE_EQ(daily_budget * (kDefaultExpirationInDays + 1),
                   prediction_[0]->budget_at);
}

TEST_F(BudgetDatabaseTest, SpendBudgetTest) {
  base::SimpleTestClock* clock = SetClockForTesting();

  // Set the default site engagement.
  SetSiteEngagementScore(kEngagement);

  // Intialize the budget with several chunks.
  GetBudgetDetails();
  clock->Advance(base::TimeDelta::FromDays(1));
  GetBudgetDetails();
  clock->Advance(base::TimeDelta::FromDays(1));
  GetBudgetDetails();

  // Spend an amount of budget less than the daily budget.
  ASSERT_TRUE(SpendBudget(1));
  GetBudgetDetails();

  // There should still be three chunks of budget of size daily_budget-1,
  // daily_budget, and kDefaultExpirationInDays * daily_budget.
  double daily_budget =
      kMaxDailyBudget * (kEngagement / SiteEngagementScore::kMaxPoints);
  ASSERT_EQ(4U, prediction_.size());
  ASSERT_DOUBLE_EQ((2 + kDefaultExpirationInDays) * daily_budget - 1,
                   prediction_[0]->budget_at);
  ASSERT_DOUBLE_EQ(daily_budget * 2, prediction_[1]->budget_at);
  ASSERT_DOUBLE_EQ(daily_budget, prediction_[2]->budget_at);
  ASSERT_DOUBLE_EQ(0, prediction_[3]->budget_at);

  // Now spend enough that it will use up the rest of the first chunk and all of
  // the second chunk, but not all of the third chunk.
  ASSERT_TRUE(SpendBudget((1 + kDefaultExpirationInDays) * daily_budget));
  GetBudgetDetails();
  ASSERT_EQ(2U, prediction_.size());
  ASSERT_DOUBLE_EQ(daily_budget - 1, prediction_[0]->budget_at);

  // Validate that the code returns false if SpendBudget tries to spend more
  // budget than the origin has.
  EXPECT_FALSE(SpendBudget(kEngagement));
  GetBudgetDetails();
  ASSERT_EQ(2U, prediction_.size());
  ASSERT_DOUBLE_EQ(daily_budget - 1, prediction_[0]->budget_at);

  // Advance time until the last remaining chunk should be expired, then query
  // for the full engagement worth of budget.
  clock->Advance(base::TimeDelta::FromDays(kDefaultExpirationInDays + 1));
  EXPECT_TRUE(SpendBudget(daily_budget * kDefaultExpirationInDays));
}

// There are times when a device's clock could move backwards in time, either
// due to hardware issues or user actions. Test here to make sure that even if
// time goes backwards and then forwards again, the origin isn't granted extra
// budget.
TEST_F(BudgetDatabaseTest, GetBudgetNegativeTime) {
  base::SimpleTestClock* clock = SetClockForTesting();

  // Set the default site engagement.
  SetSiteEngagementScore(kEngagement);

  // Initialize the budget with two chunks.
  GetBudgetDetails();
  clock->Advance(base::TimeDelta::FromDays(1));
  GetBudgetDetails();

  // Save off the budget total.
  ASSERT_EQ(3U, prediction_.size());
  double budget = prediction_[0]->budget_at;

  // Move the clock backwards in time to before the budget awards.
  clock->SetNow(clock->Now() - base::TimeDelta::FromDays(5));

  // Make sure the budget is the same.
  GetBudgetDetails();
  ASSERT_EQ(3U, prediction_.size());
  ASSERT_EQ(budget, prediction_[0]->budget_at);

  // Now move the clock back to the original time and check that no extra budget
  // is awarded.
  clock->SetNow(clock->Now() + base::TimeDelta::FromDays(5));
  GetBudgetDetails();
  ASSERT_EQ(3U, prediction_.size());
  ASSERT_EQ(budget, prediction_[0]->budget_at);
}

TEST_F(BudgetDatabaseTest, CheckBackgroundBudgetHistogram) {
  base::SimpleTestClock* clock = SetClockForTesting();

  // Set the default site engagement.
  SetSiteEngagementScore(kEngagement);

  // Initialize the budget with some interesting chunks: 30 budget (full
  // engagement), 15 budget (half of the engagement), 0 budget (less than an
  // hour), and then after the first two expire, another 30 budget.
  GetBudgetDetails();
  clock->Advance(base::TimeDelta::FromDays(kDefaultExpirationInDays / 2));
  GetBudgetDetails();
  clock->Advance(base::TimeDelta::FromMinutes(59));
  GetBudgetDetails();
  clock->Advance(base::TimeDelta::FromDays(kDefaultExpirationInDays + 1));
  GetBudgetDetails();

  // The BackgroundBudget UMA is recorded when budget is added to the origin.
  // This can happen a maximum of once per hour so there should be two entries.
  std::vector<base::Bucket> buckets =
      GetHistogramTester()->GetAllSamples("PushMessaging.BackgroundBudget");
  ASSERT_EQ(2U, buckets.size());
  // First bucket is for full award, which should have 2 entries.
  double full_award = kMaxDailyBudget * kEngagement /
                      SiteEngagementScore::kMaxPoints *
                      kDefaultExpirationInDays;
  EXPECT_EQ(floor(full_award), buckets[0].min);
  EXPECT_EQ(2, buckets[0].count);
  // Second bucket is for 1.5 * award, which should have 1 entry.
  EXPECT_EQ(floor(full_award * 1.5), buckets[1].min);
  EXPECT_EQ(1, buckets[1].count);
}

TEST_F(BudgetDatabaseTest, CheckEngagementHistograms) {
  base::SimpleTestClock* clock = SetClockForTesting();

  // Manipulate the engagement so that the budget is twice the cost of an
  // action.
  double cost = 2;
  double engagement = 2 * cost * SiteEngagementScore::kMaxPoints /
                      kDefaultExpirationInDays / kMaxDailyBudget;
  SetSiteEngagementScore(engagement);

  // Get the budget, which will award a chunk of budget equal to engagement.
  GetBudgetDetails();

  // Now spend the budget to trigger the UMA recording the SES score. The first
  // call shouldn't write any UMA. The second should write a lowSES entry, and
  // the third should write a noSES entry.
  ASSERT_TRUE(SpendBudget(cost));
  ASSERT_TRUE(SpendBudget(cost));
  ASSERT_FALSE(SpendBudget(cost));

  // Advance the clock by 12 days (to guarantee a full new engagement grant)
  // then change the SES score to get a different UMA entry, then spend the
  // budget again.
  clock->Advance(base::TimeDelta::FromDays(12));
  GetBudgetDetails();
  SetSiteEngagementScore(engagement * 2);
  ASSERT_TRUE(SpendBudget(cost));
  ASSERT_TRUE(SpendBudget(cost));
  ASSERT_FALSE(SpendBudget(cost));

  // Now check the UMA. Both UMA should have 2 buckets with 1 entry each.
  std::vector<base::Bucket> no_budget_buckets =
      GetHistogramTester()->GetAllSamples("PushMessaging.SESForNoBudgetOrigin");
  ASSERT_EQ(2U, no_budget_buckets.size());
  EXPECT_EQ(floor(engagement), no_budget_buckets[0].min);
  EXPECT_EQ(1, no_budget_buckets[0].count);
  EXPECT_EQ(floor(engagement * 2), no_budget_buckets[1].min);
  EXPECT_EQ(1, no_budget_buckets[1].count);

  std::vector<base::Bucket> low_budget_buckets =
      GetHistogramTester()->GetAllSamples(
          "PushMessaging.SESForLowBudgetOrigin");
  ASSERT_EQ(2U, low_budget_buckets.size());
  EXPECT_EQ(floor(engagement), low_budget_buckets[0].min);
  EXPECT_EQ(1, low_budget_buckets[0].count);
  EXPECT_EQ(floor(engagement * 2), low_budget_buckets[1].min);
  EXPECT_EQ(1, low_budget_buckets[1].count);
}

TEST_F(BudgetDatabaseTest, DefaultSiteEngagementInIncognitoProfile) {
  TestingProfile second_profile;
  TestingProfile* second_profile_incognito =
      TestingProfile::Builder().BuildIncognito(&second_profile);

  // Create a second BudgetDatabase instance for the off-the-record version of
  // a second profile. This will not have been influenced by the |profile_|.
  std::unique_ptr<BudgetDatabase> second_database =
      base::MakeUnique<BudgetDatabase>(second_profile_incognito,
                                       base::FilePath() /* in memory */);

  ASSERT_FALSE(profile()->IsOffTheRecord());
  ASSERT_FALSE(second_profile.IsOffTheRecord());
  ASSERT_TRUE(second_profile_incognito->IsOffTheRecord());

  // The Site Engagement Score considered by an Incognito profile must be equal
  // to the score considered in a regular profile visting a page for the first
  // time. This may grant a small amount of budget, but does mean that Incognito
  // mode cannot be detected through the Budget API.
  EXPECT_EQ(database()->GetSiteEngagementScoreForOrigin(origin()),
            second_database->GetSiteEngagementScoreForOrigin(origin()));
}
