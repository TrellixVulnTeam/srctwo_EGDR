// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/payments/core/journey_logger.h"

#include "base/metrics/metrics_hashes.h"
#include "base/test/histogram_tester.h"
#include "base/test/scoped_feature_list.h"
#include "components/metrics/proto/ukm/entry.pb.h"
#include "components/ukm/test_ukm_recorder.h"
#include "components/ukm/ukm_source.h"
#include "services/metrics/public/cpp/ukm_builders.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::ContainerEq;

namespace payments {

// Tests the canMakePayment stats for the case where the merchant does not use
// it and does not show the PaymentRequest to the user.
TEST(JourneyLoggerTest,
     RecordJourneyStatsHistograms_CanMakePaymentNotCalled_NoShow) {
  base::HistogramTester histogram_tester;
  JourneyLogger logger(/*is_incognito=*/false, /*url=*/GURL(""),
                       /*ukm_recorder=*/nullptr);

  logger.SetCompleted();

  // Make sure the correct events were logged.
  std::vector<base::Bucket> buckets =
      histogram_tester.GetAllSamples("PaymentRequest.Events");
  ASSERT_EQ(1U, buckets.size());
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_SHOWN);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_TRUE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_FALSE);
}

// Tests the canMakePayment stats for the case where the merchant does not use
// it and the transaction is aborted.
TEST(JourneyLoggerTest,
     RecordJourneyStatsHistograms_CanMakePaymentNotCalled_ShowAndUserAbort) {
  base::HistogramTester histogram_tester;
  JourneyLogger logger(/*is_incognito=*/false, /*url=*/GURL(""),
                       /*ukm_recorder=*/nullptr);

  // The merchant does not query CanMakePayment, show the PaymentRequest and the
  // user aborts it.
  logger.SetEventOccurred(JourneyLogger::EVENT_SHOWN);
  logger.SetRequestedInformation(true, false, false, false);
  logger.SetAborted(JourneyLogger::ABORT_REASON_ABORTED_BY_USER);

  // Make sure the correct events were logged.
  std::vector<base::Bucket> buckets =
      histogram_tester.GetAllSamples("PaymentRequest.Events");
  ASSERT_EQ(1U, buckets.size());
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_SHOWN);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_TRUE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_FALSE);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_USER_ABORTED);
}

// Tests the canMakePayment stats for the case where the merchant does not use
// it and the transaction is aborted.
TEST(JourneyLoggerTest,
     RecordJourneyStatsHistograms_CanMakePaymentNotCalled_ShowAndOtherAbort) {
  base::HistogramTester histogram_tester;
  JourneyLogger logger(/*is_incognito=*/false, /*url=*/GURL(""),
                       /*ukm_recorder=*/nullptr);

  // The merchant does not query CanMakePayment, show the PaymentRequest and
  // there is an abort not initiated by the user.
  logger.SetEventOccurred(JourneyLogger::EVENT_SHOWN);
  logger.SetRequestedInformation(true, false, false, false);
  logger.SetAborted(JourneyLogger::ABORT_REASON_OTHER);

  // Make sure the correct events were logged.
  std::vector<base::Bucket> buckets =
      histogram_tester.GetAllSamples("PaymentRequest.Events");
  ASSERT_EQ(1U, buckets.size());
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_SHOWN);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_TRUE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_FALSE);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_OTHER_ABORTED);
}

// Tests the canMakePayment stats for the case where the merchant does not use
// it and the transaction is completed.
TEST(JourneyLoggerTest,
     RecordJourneyStatsHistograms_CanMakePaymentNotCalled_ShowAndComplete) {
  base::HistogramTester histogram_tester;
  JourneyLogger logger(/*is_incognito=*/false, /*url=*/GURL(""),
                       /*ukm_recorder=*/nullptr);

  // The merchant does not query CanMakePayment, show the PaymentRequest and the
  // user completes it.
  logger.SetEventOccurred(JourneyLogger::EVENT_SHOWN);
  logger.SetRequestedInformation(true, false, false, false);
  logger.SetCompleted();

  // Make sure the correct events were logged.
  std::vector<base::Bucket> buckets =
      histogram_tester.GetAllSamples("PaymentRequest.Events");
  ASSERT_EQ(1U, buckets.size());
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_SHOWN);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_TRUE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_FALSE);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_COMPLETED);
}

// Tests the canMakePayment stats for the case where the merchant uses it,
// returns false and show is not called.
TEST(JourneyLoggerTest,
     RecordJourneyStatsHistograms_CanMakePaymentCalled_FalseAndNoShow) {
  base::HistogramTester histogram_tester;
  JourneyLogger logger(/*is_incognito=*/false, /*url=*/GURL(""),
                       /*ukm_recorder=*/nullptr);

  // The user cannot make payment and the PaymentRequest is not shown.
  logger.SetCanMakePaymentValue(false);
  logger.SetAborted(JourneyLogger::ABORT_REASON_OTHER);

  // Make sure the correct events were logged.
  std::vector<base::Bucket> buckets =
      histogram_tester.GetAllSamples("PaymentRequest.Events");
  ASSERT_EQ(1U, buckets.size());
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_SHOWN);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_TRUE);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_FALSE);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_OTHER_ABORTED);
}

// Tests the canMakePayment stats for the case where the merchant uses it,
// returns true and show is not called.
TEST(JourneyLoggerTest,
     RecordJourneyStatsHistograms_CanMakePaymentCalled_TrueAndNoShow) {
  base::HistogramTester histogram_tester;
  JourneyLogger logger(/*is_incognito=*/false, /*url=*/GURL(""),
                       /*ukm_recorder=*/nullptr);

  // The user can make payment and the PaymentRequest is not shown.
  logger.SetCanMakePaymentValue(true);
  logger.SetAborted(JourneyLogger::ABORT_REASON_OTHER);

  // Make sure the correct events were logged.
  std::vector<base::Bucket> buckets =
      histogram_tester.GetAllSamples("PaymentRequest.Events");
  ASSERT_EQ(1U, buckets.size());
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_SHOWN);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_TRUE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_FALSE);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_OTHER_ABORTED);
}

// Tests the canMakePayment stats for the case where the merchant uses it,
// returns false, show is called but the transaction is aborted by the user.
TEST(JourneyLoggerTest,
     RecordJourneyStatsHistograms_CanMakePaymentCalled_FalseShowAndUserAbort) {
  base::HistogramTester histogram_tester;
  JourneyLogger logger(/*is_incognito=*/false, /*url=*/GURL(""),
                       /*ukm_recorder=*/nullptr);

  // The user cannot make payment, the Payment Request is shown but is aborted
  // by the user.
  logger.SetEventOccurred(JourneyLogger::EVENT_SHOWN);
  logger.SetRequestedInformation(true, false, false, false);
  logger.SetCanMakePaymentValue(false);
  logger.SetAborted(JourneyLogger::ABORT_REASON_ABORTED_BY_USER);

  // Make sure the correct events were logged.
  std::vector<base::Bucket> buckets =
      histogram_tester.GetAllSamples("PaymentRequest.Events");
  ASSERT_EQ(1U, buckets.size());
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_SHOWN);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_TRUE);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_FALSE);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_USER_ABORTED);
}

// Tests the canMakePayment stats for the case where the merchant uses it,
// returns false, show is called but the transaction is aborted.
TEST(JourneyLoggerTest,
     RecordJourneyStatsHistograms_CanMakePaymentCalled_FalseShowAndOtherAbort) {
  base::HistogramTester histogram_tester;
  JourneyLogger logger(/*is_incognito=*/false, /*url=*/GURL(""),
                       /*ukm_recorder=*/nullptr);

  // The user cannot make payment, the Payment Request is shown but is aborted.
  logger.SetEventOccurred(JourneyLogger::EVENT_SHOWN);
  logger.SetRequestedInformation(true, false, false, false);
  logger.SetCanMakePaymentValue(false);
  logger.SetAborted(JourneyLogger::ABORT_REASON_OTHER);

  // Make sure the correct events were logged.
  std::vector<base::Bucket> buckets =
      histogram_tester.GetAllSamples("PaymentRequest.Events");
  ASSERT_EQ(1U, buckets.size());
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_SHOWN);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_TRUE);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_FALSE);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_OTHER_ABORTED);
}

// Tests the canMakePayment stats for the case where the merchant uses it,
// returns false, show is called and the transaction is completed.
TEST(JourneyLoggerTest,
     RecordJourneyStatsHistograms_CanMakePaymentCalled_FalseShowAndComplete) {
  base::HistogramTester histogram_tester;
  JourneyLogger logger(/*is_incognito=*/false, /*url=*/GURL(""),
                       /*ukm_recorder=*/nullptr);

  // The user cannot make payment, the payment request is shown and is
  // completed.
  logger.SetEventOccurred(JourneyLogger::EVENT_SHOWN);
  logger.SetRequestedInformation(true, false, false, false);
  logger.SetCanMakePaymentValue(false);
  logger.SetCompleted();

  // Make sure the correct events were logged.
  std::vector<base::Bucket> buckets =
      histogram_tester.GetAllSamples("PaymentRequest.Events");
  ASSERT_EQ(1U, buckets.size());
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_SHOWN);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_TRUE);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_FALSE);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_COMPLETED);
}

// Tests the canMakePayment stats for the case where the merchant uses it,
// returns true, show is called but the transaction is aborted by the user.
TEST(JourneyLoggerTest,
     RecordJourneyStatsHistograms_CanMakePaymentCalled_TrueShowAndUserAbort) {
  base::HistogramTester histogram_tester;
  JourneyLogger logger(/*is_incognito=*/false, /*url=*/GURL(""),
                       /*ukm_recorder=*/nullptr);

  // The user can make payment, the Payment Request is shown and aborted by the
  // user.
  logger.SetEventOccurred(JourneyLogger::EVENT_SHOWN);
  logger.SetRequestedInformation(true, false, false, false);
  logger.SetCanMakePaymentValue(true);
  logger.SetAborted(JourneyLogger::ABORT_REASON_ABORTED_BY_USER);

  // Make sure the correct events were logged.
  std::vector<base::Bucket> buckets =
      histogram_tester.GetAllSamples("PaymentRequest.Events");
  ASSERT_EQ(1U, buckets.size());
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_SHOWN);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_TRUE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_FALSE);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_USER_ABORTED);
}

// Tests the canMakePayment stats for the case where the merchant uses it,
// returns true, show is called but the transaction is aborted.
TEST(JourneyLoggerTest,
     RecordJourneyStatsHistograms_CanMakePaymentCalled_TrueShowAndOtherAbort) {
  base::HistogramTester histogram_tester;
  JourneyLogger logger(/*is_incognito=*/false, /*url=*/GURL(""),
                       /*ukm_recorder=*/nullptr);

  // The user can make a payment, the request is shown but the transaction is
  // aborted.
  logger.SetEventOccurred(JourneyLogger::EVENT_SHOWN);
  logger.SetRequestedInformation(true, false, false, false);
  logger.SetCanMakePaymentValue(true);
  logger.SetAborted(JourneyLogger::ABORT_REASON_OTHER);

  // Make sure the correct events were logged.
  std::vector<base::Bucket> buckets =
      histogram_tester.GetAllSamples("PaymentRequest.Events");
  ASSERT_EQ(1U, buckets.size());
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_SHOWN);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_TRUE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_FALSE);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_OTHER_ABORTED);
}

// Tests the canMakePayment stats for the case where the merchant uses it,
// returns true, show is called and the transaction is completed.
TEST(JourneyLoggerTest,
     RecordJourneyStatsHistograms_CanMakePaymentCalled_TrueShowAndComplete) {
  base::HistogramTester histogram_tester;
  JourneyLogger logger(/*is_incognito=*/false, /*url=*/GURL(""),
                       /*ukm_recorder=*/nullptr);

  // The user can make a payment, the request is shown and the user completes
  // the checkout.
  logger.SetEventOccurred(JourneyLogger::EVENT_SHOWN);
  logger.SetRequestedInformation(true, false, false, false);
  logger.SetCanMakePaymentValue(true);
  logger.SetCompleted();

  // Make sure the correct events were logged.
  std::vector<base::Bucket> buckets =
      histogram_tester.GetAllSamples("PaymentRequest.Events");
  ASSERT_EQ(1U, buckets.size());
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_SHOWN);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_TRUE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_FALSE);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_COMPLETED);
}

// Tests the canMakePayment metrics are not logged if the Payment Request was
// done in an incognito tab.
TEST(JourneyLoggerTest,
     RecordJourneyStatsHistograms_CanMakePayment_IncognitoTab) {
  base::HistogramTester histogram_tester;
  JourneyLogger logger(/*is_incognito=*/true, /*url=*/GURL(""),
                       /*ukm_recorder=*/nullptr);

  // The user can make a payment, the request is shown and the user completes
  // the checkout.
  logger.SetEventOccurred(JourneyLogger::EVENT_SHOWN);
  logger.SetRequestedInformation(true, false, false, false);
  logger.SetCanMakePaymentValue(true);
  logger.SetCompleted();

  // Make sure the correct events were logged.
  std::vector<base::Bucket> buckets =
      histogram_tester.GetAllSamples("PaymentRequest.Events");
  ASSERT_EQ(1U, buckets.size());
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_SHOWN);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_TRUE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_FALSE);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_COMPLETED);
}

// Tests that the completion status metrics based on whether the user had
// suggestions for all the requested sections are logged as correctly.
TEST(JourneyLoggerTest,
     RecordJourneyStatsHistograms_SuggestionsForEverything_Completed) {
  base::HistogramTester histogram_tester;
  JourneyLogger logger(/*is_incognito=*/false, /*url=*/GURL(""),
                       /*ukm_recorder=*/nullptr);

  // The merchant only requests payment information.
  logger.SetRequestedInformation(
      /*requested_shipping=*/false, /*requested_email=*/false,
      /*requested_phone=*/false, /*requested_name=*/false);
  logger.SetRequestedPaymentMethodTypes(
      /*requested_basic_card=*/true, /*requested_method_google=*/true,
      /*requested_method_other=*/false);

  // Simulate that the user had suggestions for all the requested sections.
  logger.SetNumberOfSuggestionsShown(JourneyLogger::SECTION_PAYMENT_METHOD, 1,
                                     /*has_complete_suggestion=*/false);

  // Simulate that the Payment Request was shown to the user.
  logger.SetEventOccurred(JourneyLogger::EVENT_SHOWN);

  // Simulate that the user completes the checkout.
  logger.SetCompleted();

  // Make sure the correct events were logged.
  std::vector<base::Bucket> buckets =
      histogram_tester.GetAllSamples("PaymentRequest.Events");
  ASSERT_EQ(1U, buckets.size());
  EXPECT_FALSE(buckets[0].min &
               JourneyLogger::EVENT_HAD_NECESSARY_COMPLETE_SUGGESTIONS);
  EXPECT_TRUE(buckets[0].min &
              JourneyLogger::EVENT_HAD_INITIAL_FORM_OF_PAYMENT);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_COMPLETED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_USER_ABORTED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_OTHER_ABORTED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_SHIPPING);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_NAME);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_PHONE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_EMAIL);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_BASIC_CARD);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_GOOGLE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_OTHER);
}

// Tests that the completion status metrics based on whether the user had
// suggestions for all the requested sections are logged as correctly.
TEST(JourneyLoggerTest,
     RecordJourneyStatsHistograms_SuggestionsForEverything_UserAborted) {
  base::HistogramTester histogram_tester;
  JourneyLogger logger(/*is_incognito=*/false, /*url=*/GURL(""),
                       /*ukm_recorder=*/nullptr);

  // The merchant only requests payment information.
  logger.SetRequestedInformation(
      /*requested_shipping=*/false, /*requested_email=*/false,
      /*requested_phone=*/false, /*requested_name=*/false);
  logger.SetRequestedPaymentMethodTypes(
      /*requested_basic_card=*/true, /*requested_method_google=*/true,
      /*requested_method_other=*/false);

  // Simulate that the user had suggestions for all the requested sections.
  logger.SetNumberOfSuggestionsShown(JourneyLogger::SECTION_PAYMENT_METHOD, 1,
                                     /*has_complete_suggestion=*/false);

  // Simulate that the Payment Request was shown to the user.
  logger.SetEventOccurred(JourneyLogger::EVENT_SHOWN);

  // Simulate that the user aborts the checkout.
  logger.SetAborted(JourneyLogger::ABORT_REASON_ABORTED_BY_USER);

  // Make sure the correct events were logged.
  std::vector<base::Bucket> buckets =
      histogram_tester.GetAllSamples("PaymentRequest.Events");
  ASSERT_EQ(1U, buckets.size());
  EXPECT_FALSE(buckets[0].min &
               JourneyLogger::EVENT_HAD_NECESSARY_COMPLETE_SUGGESTIONS);
  EXPECT_TRUE(buckets[0].min &
              JourneyLogger::EVENT_HAD_INITIAL_FORM_OF_PAYMENT);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_USER_ABORTED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_COMPLETED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_OTHER_ABORTED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_SHIPPING);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_NAME);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_PHONE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_EMAIL);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_BASIC_CARD);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_GOOGLE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_OTHER);
}

// Tests that the completion status metrics based on whether the user had
// suggestions for all the requested sections are logged as correctly.
TEST(JourneyLoggerTest,
     RecordJourneyStatsHistograms_SuggestionsForEverything_OtherAborted) {
  base::HistogramTester histogram_tester;
  JourneyLogger logger(/*is_incognito=*/false, /*url=*/GURL(""),
                       /*ukm_recorder=*/nullptr);

  // The merchant only requests payment information.
  logger.SetRequestedInformation(
      /*requested_shipping=*/false, /*requested_email=*/false,
      /*requested_phone=*/false, /*requested_name=*/false);
  logger.SetRequestedPaymentMethodTypes(
      /*requested_basic_card=*/true, /*requested_method_google=*/true,
      /*requested_method_other=*/false);

  // Simulate that the user had suggestions for all the requested sections.
  logger.SetNumberOfSuggestionsShown(JourneyLogger::SECTION_PAYMENT_METHOD, 1,
                                     /*has_complete_suggestion=*/false);

  // Simulate that the Payment Request was shown to the user.
  logger.SetEventOccurred(JourneyLogger::EVENT_SHOWN);

  // Simulate that the checkout is aborted.
  logger.SetAborted(JourneyLogger::ABORT_REASON_OTHER);

  // Make sure the correct events were logged.
  std::vector<base::Bucket> buckets =
      histogram_tester.GetAllSamples("PaymentRequest.Events");
  ASSERT_EQ(1U, buckets.size());
  EXPECT_FALSE(buckets[0].min &
               JourneyLogger::EVENT_HAD_NECESSARY_COMPLETE_SUGGESTIONS);
  EXPECT_TRUE(buckets[0].min &
              JourneyLogger::EVENT_HAD_INITIAL_FORM_OF_PAYMENT);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_OTHER_ABORTED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_COMPLETED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_USER_ABORTED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_SHIPPING);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_NAME);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_PHONE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_EMAIL);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_BASIC_CARD);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_GOOGLE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_OTHER);
}

// Tests that the completion status metrics based on whether the user had
// suggestions for all the requested sections are logged as correctly, even in
// incognito mode.
TEST(JourneyLoggerTest,
     RecordJourneyStatsHistograms_SuggestionsForEverything_Incognito) {
  base::HistogramTester histogram_tester;
  JourneyLogger logger(/*is_incognito=*/true, /*url=*/GURL(""),
                       /*ukm_recorder=*/nullptr);

  // The merchant only requests payment information.
  logger.SetRequestedInformation(
      /*requested_shipping=*/false, /*requested_email=*/false,
      /*requested_phone=*/false, /*requested_name=*/false);
  logger.SetRequestedPaymentMethodTypes(
      /*requested_basic_card=*/true, /*requested_method_google=*/true,
      /*requested_method_other=*/false);

  // Simulate that the user had suggestions for all the requested sections.
  logger.SetNumberOfSuggestionsShown(JourneyLogger::SECTION_PAYMENT_METHOD, 1,
                                     /*has_complete_suggestion=*/false);

  // Simulate that the Payment Request was shown to the user.
  logger.SetEventOccurred(JourneyLogger::EVENT_SHOWN);

  // Simulate that the user completes the checkout.
  logger.SetCompleted();

  // Make sure the correct events were logged.
  std::vector<base::Bucket> buckets =
      histogram_tester.GetAllSamples("PaymentRequest.Events");
  ASSERT_EQ(1U, buckets.size());
  EXPECT_FALSE(buckets[0].min &
               JourneyLogger::EVENT_HAD_NECESSARY_COMPLETE_SUGGESTIONS);
  EXPECT_TRUE(buckets[0].min &
              JourneyLogger::EVENT_HAD_INITIAL_FORM_OF_PAYMENT);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_COMPLETED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_USER_ABORTED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_OTHER_ABORTED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_SHIPPING);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_NAME);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_PHONE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_EMAIL);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_BASIC_CARD);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_GOOGLE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_OTHER);
}

// Tests that the completion status metrics based on whether the user had
// suggestions for all the requested sections are logged as correctly.
TEST(JourneyLoggerTest,
     RecordJourneyStatsHistograms_NoSuggestionsForEverything_Completed) {
  base::HistogramTester histogram_tester;
  JourneyLogger logger(/*is_incognito=*/false, /*url=*/GURL(""),
                       /*ukm_recorder=*/nullptr);

  // The merchant only requests payment information.
  logger.SetRequestedInformation(
      /*requested_shipping=*/false, /*requested_email=*/false,
      /*requested_phone=*/false, /*requested_name=*/false);
  logger.SetRequestedPaymentMethodTypes(
      /*requested_basic_card=*/true, /*requested_method_google=*/true,
      /*requested_method_other=*/false);

  // Simulate that the user had suggestions for none of the requested sections.
  logger.SetNumberOfSuggestionsShown(JourneyLogger::SECTION_PAYMENT_METHOD, 0,
                                     /*has_complete_suggestion=*/false);

  // Simulate that the Payment Request was shown to the user.
  logger.SetEventOccurred(JourneyLogger::EVENT_SHOWN);

  // Simulate that the user completes the checkout.
  logger.SetCompleted();

  // Make sure the correct events were logged.
  std::vector<base::Bucket> buckets =
      histogram_tester.GetAllSamples("PaymentRequest.Events");
  ASSERT_EQ(1U, buckets.size());
  EXPECT_FALSE(buckets[0].min &
               JourneyLogger::EVENT_HAD_NECESSARY_COMPLETE_SUGGESTIONS);
  EXPECT_FALSE(buckets[0].min &
               JourneyLogger::EVENT_HAD_INITIAL_FORM_OF_PAYMENT);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_COMPLETED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_USER_ABORTED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_OTHER_ABORTED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_SHIPPING);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_NAME);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_PHONE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_EMAIL);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_BASIC_CARD);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_GOOGLE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_OTHER);
}

// Tests that the completion status metrics based on whether the user had
// suggestions for all the requested sections are logged as correctly.
TEST(JourneyLoggerTest,
     RecordJourneyStatsHistograms_NoSuggestionsForEverything_UserAborted) {
  base::HistogramTester histogram_tester;
  JourneyLogger logger(/*is_incognito=*/false, /*url=*/GURL(""),
                       /*ukm_recorder=*/nullptr);

  // The merchant only requests payment information.
  logger.SetRequestedInformation(
      /*requested_shipping=*/false, /*requested_email=*/false,
      /*requested_phone=*/false, /*requested_name=*/false);
  logger.SetRequestedPaymentMethodTypes(
      /*requested_basic_card=*/true, /*requested_method_google=*/true,
      /*requested_method_other=*/false);

  // Simulate that the user had suggestions for none of the requested sections.
  logger.SetNumberOfSuggestionsShown(JourneyLogger::SECTION_PAYMENT_METHOD, 0,
                                     /*has_complete_suggestion=*/false);

  // Simulate that the Payment Request was shown to the user.
  logger.SetEventOccurred(JourneyLogger::EVENT_SHOWN);

  // Simulate that the user aborts the checkout.
  logger.SetAborted(JourneyLogger::ABORT_REASON_ABORTED_BY_USER);

  // Make sure the correct events were logged.
  std::vector<base::Bucket> buckets =
      histogram_tester.GetAllSamples("PaymentRequest.Events");
  ASSERT_EQ(1U, buckets.size());
  EXPECT_FALSE(buckets[0].min &
               JourneyLogger::EVENT_HAD_NECESSARY_COMPLETE_SUGGESTIONS);
  EXPECT_FALSE(buckets[0].min &
               JourneyLogger::EVENT_HAD_INITIAL_FORM_OF_PAYMENT);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_USER_ABORTED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_COMPLETED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_OTHER_ABORTED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_SHIPPING);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_NAME);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_PHONE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_EMAIL);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_BASIC_CARD);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_GOOGLE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_OTHER);
}

// Tests that the completion status metrics based on whether the user had
// suggestions for all the requested sections are logged as correctly.
TEST(JourneyLoggerTest,
     RecordJourneyStatsHistograms_NoSuggestionsForEverything_OtherAborted) {
  base::HistogramTester histogram_tester;
  JourneyLogger logger(/*is_incognito=*/false, /*url=*/GURL(""),
                       /*ukm_recorder=*/nullptr);

  // The merchant only requests payment information.
  logger.SetRequestedInformation(
      /*requested_shipping=*/false, /*requested_email=*/false,
      /*requested_phone=*/false, /*requested_name=*/false);
  logger.SetRequestedPaymentMethodTypes(
      /*requested_basic_card=*/true, /*requested_method_google=*/true,
      /*requested_method_other=*/false);

  // Simulate that the user had suggestions for none of the requested sections.
  logger.SetNumberOfSuggestionsShown(JourneyLogger::SECTION_PAYMENT_METHOD, 0,
                                     /*has_complete_suggestion=*/false);

  // Simulate that the Payment Request was shown to the user.
  logger.SetEventOccurred(JourneyLogger::EVENT_SHOWN);

  // Simulate that the the checkout is aborted.
  logger.SetAborted(JourneyLogger::ABORT_REASON_OTHER);

  // Make sure the correct events were logged.
  std::vector<base::Bucket> buckets =
      histogram_tester.GetAllSamples("PaymentRequest.Events");
  ASSERT_EQ(1U, buckets.size());
  EXPECT_FALSE(buckets[0].min &
               JourneyLogger::EVENT_HAD_NECESSARY_COMPLETE_SUGGESTIONS);
  EXPECT_FALSE(buckets[0].min &
               JourneyLogger::EVENT_HAD_INITIAL_FORM_OF_PAYMENT);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_OTHER_ABORTED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_COMPLETED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_USER_ABORTED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_SHIPPING);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_NAME);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_PHONE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_EMAIL);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_BASIC_CARD);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_GOOGLE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_OTHER);
}

// Tests that the completion status metrics based on whether the user had
// suggestions for all the requested sections are logged as correctly, even in
// incognito mode.
TEST(JourneyLoggerTest,
     RecordJourneyStatsHistograms_NoSuggestionsForEverything_Incognito) {
  base::HistogramTester histogram_tester;
  JourneyLogger logger(/*is_incognito=*/true, /*url=*/GURL(""),
                       /*ukm_recorder=*/nullptr);

  // The merchant only requests payment information.
  logger.SetRequestedInformation(
      /*requested_shipping=*/false, /*requested_email=*/false,
      /*requested_phone=*/false, /*requested_name=*/false);
  logger.SetRequestedPaymentMethodTypes(
      /*requested_basic_card=*/true, /*requested_method_google=*/true,
      /*requested_method_other=*/false);

  // Simulate that the user had suggestions for none of the requested sections.
  logger.SetNumberOfSuggestionsShown(JourneyLogger::SECTION_PAYMENT_METHOD, 0,
                                     /*has_complete_suggestion=*/false);

  // Simulate that the Payment Request was shown to the user.
  logger.SetEventOccurred(JourneyLogger::EVENT_SHOWN);

  // Simulate that the user aborts the checkout.
  logger.SetAborted(JourneyLogger::ABORT_REASON_ABORTED_BY_USER);

  // Make sure the correct events were logged.
  std::vector<base::Bucket> buckets =
      histogram_tester.GetAllSamples("PaymentRequest.Events");
  ASSERT_EQ(1U, buckets.size());
  EXPECT_FALSE(buckets[0].min &
               JourneyLogger::EVENT_HAD_NECESSARY_COMPLETE_SUGGESTIONS);
  EXPECT_FALSE(buckets[0].min &
               JourneyLogger::EVENT_HAD_INITIAL_FORM_OF_PAYMENT);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_USER_ABORTED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_COMPLETED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_OTHER_ABORTED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_SHIPPING);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_NAME);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_PHONE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_EMAIL);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_BASIC_CARD);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_GOOGLE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_OTHER);
}

// Tests that the completion status metrics based on whether the user had
// suggestions for all the requested sections are logged as correctly.
TEST(
    JourneyLoggerTest,
    RecordJourneyStatsHistograms_NoCompleteSuggestionsForEverything_OtherAborted) {
  base::HistogramTester histogram_tester;
  JourneyLogger logger(/*is_incognito=*/false, /*url=*/GURL(""),
                       /*ukm_recorder=*/nullptr);

  // The merchant only requests payment information.
  logger.SetRequestedInformation(
      /*requested_shipping=*/false, /*requested_email=*/false,
      /*requested_phone=*/false, /*requested_name=*/false);
  logger.SetRequestedPaymentMethodTypes(
      /*requested_basic_card=*/true, /*requested_method_google=*/true,
      /*requested_method_other=*/false);

  // Simulate that the user had incomplete suggestions for the requested
  // sections.
  logger.SetNumberOfSuggestionsShown(JourneyLogger::SECTION_PAYMENT_METHOD, 2,
                                     /*has_complete_suggestion=*/false);

  // Simulate that the Payment Request was shown to the user.
  logger.SetEventOccurred(JourneyLogger::EVENT_SHOWN);

  // Simulate that the the checkout is aborted.
  logger.SetAborted(JourneyLogger::ABORT_REASON_OTHER);

  // Make sure the correct events were logged.
  std::vector<base::Bucket> buckets =
      histogram_tester.GetAllSamples("PaymentRequest.Events");
  ASSERT_EQ(1U, buckets.size());
  EXPECT_FALSE(buckets[0].min &
               JourneyLogger::EVENT_HAD_NECESSARY_COMPLETE_SUGGESTIONS);
  EXPECT_TRUE(buckets[0].min &
              JourneyLogger::EVENT_HAD_INITIAL_FORM_OF_PAYMENT);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_OTHER_ABORTED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_COMPLETED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_USER_ABORTED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_SHIPPING);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_NAME);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_PHONE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_EMAIL);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_BASIC_CARD);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_GOOGLE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_OTHER);
}

// Tests that the completion status metrics based on whether the user had
// suggestions for all the requested sections are logged as correctly.
TEST(
    JourneyLoggerTest,
    RecordJourneyStatsHistograms_NoCompleteSuggestionsForEverything_SomeComplete_OtherAborted) {
  base::HistogramTester histogram_tester;
  JourneyLogger logger(/*is_incognito=*/false, /*url=*/GURL(""),
                       /*ukm_recorder=*/nullptr);

  // The merchant only requests payment information.
  logger.SetRequestedInformation(
      /*requested_shipping=*/true, /*requested_email=*/false,
      /*requested_phone=*/false, /*requested_name=*/false);
  logger.SetRequestedPaymentMethodTypes(
      /*requested_basic_card=*/true, /*requested_method_google=*/true,
      /*requested_method_other=*/false);

  // Simulate that the user had incomplete suggestions for one of the requested
  // sections.
  logger.SetNumberOfSuggestionsShown(JourneyLogger::SECTION_PAYMENT_METHOD, 2,
                                     /*has_complete_suggestion=*/false);
  logger.SetNumberOfSuggestionsShown(JourneyLogger::SECTION_SHIPPING_ADDRESS, 1,
                                     /*has_complete_suggestion=*/true);

  // Simulate that the Payment Request was shown to the user.
  logger.SetEventOccurred(JourneyLogger::EVENT_SHOWN);

  // Simulate that the the checkout is aborted.
  logger.SetAborted(JourneyLogger::ABORT_REASON_OTHER);

  // Make sure the correct events were logged.
  std::vector<base::Bucket> buckets =
      histogram_tester.GetAllSamples("PaymentRequest.Events");
  ASSERT_EQ(1U, buckets.size());
  EXPECT_FALSE(buckets[0].min &
               JourneyLogger::EVENT_HAD_NECESSARY_COMPLETE_SUGGESTIONS);
  EXPECT_TRUE(buckets[0].min &
              JourneyLogger::EVENT_HAD_INITIAL_FORM_OF_PAYMENT);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_OTHER_ABORTED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_COMPLETED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_USER_ABORTED);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_REQUEST_SHIPPING);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_NAME);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_PHONE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_EMAIL);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_BASIC_CARD);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_GOOGLE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_OTHER);
}

// Tests that the completion status metrics based on whether the user had
// suggestions for all the requested sections are logged as correctly.
TEST(
    JourneyLoggerTest,
    RecordJourneyStatsHistograms_CompleteSuggestionsForEverything_OtherAborted) {
  base::HistogramTester histogram_tester;
  JourneyLogger logger(/*is_incognito=*/false, /*url=*/GURL(""),
                       /*ukm_recorder=*/nullptr);

  // The merchant only requests payment information.
  logger.SetRequestedInformation(
      /*requested_shipping=*/true, /*requested_email=*/false,
      /*requested_phone=*/false, /*requested_name=*/false);
  logger.SetRequestedPaymentMethodTypes(
      /*requested_basic_card=*/true, /*requested_method_google=*/true,
      /*requested_method_other=*/false);

  // Simulate that the user had incomplete suggestions for one of the requested
  // sections.
  logger.SetNumberOfSuggestionsShown(JourneyLogger::SECTION_PAYMENT_METHOD, 2,
                                     /*has_complete_suggestion=*/true);
  logger.SetNumberOfSuggestionsShown(JourneyLogger::SECTION_SHIPPING_ADDRESS, 1,
                                     /*has_complete_suggestion=*/true);

  // Simulate that the Payment Request was shown to the user.
  logger.SetEventOccurred(JourneyLogger::EVENT_SHOWN);

  // Simulate that the the checkout is aborted.
  logger.SetAborted(JourneyLogger::ABORT_REASON_OTHER);

  // Make sure the correct events were logged.
  std::vector<base::Bucket> buckets =
      histogram_tester.GetAllSamples("PaymentRequest.Events");
  ASSERT_EQ(1U, buckets.size());
  EXPECT_TRUE(buckets[0].min &
              JourneyLogger::EVENT_HAD_NECESSARY_COMPLETE_SUGGESTIONS);
  EXPECT_TRUE(buckets[0].min &
              JourneyLogger::EVENT_HAD_INITIAL_FORM_OF_PAYMENT);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_OTHER_ABORTED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_COMPLETED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_USER_ABORTED);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_REQUEST_SHIPPING);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_NAME);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_PHONE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_EMAIL);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_BASIC_CARD);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_GOOGLE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_OTHER);
}

// Tests that the metrics are logged correctly for two simultaneous Payment
// Requests.
TEST(JourneyLoggerTest, RecordJourneyStatsHistograms_TwoPaymentRequests) {
  base::HistogramTester histogram_tester;
  JourneyLogger logger1(/*is_incognito=*/false, /*url=*/GURL(""),
                        /*ukm_recorder=*/nullptr);
  JourneyLogger logger2(/*is_incognito=*/false, /*url=*/GURL(""),
                        /*ukm_recorder=*/nullptr);

  // Make the two loggers have different data.
  logger1.SetEventOccurred(JourneyLogger::EVENT_SHOWN);
  logger1.SetRequestedInformation(
      /*requested_shipping=*/true, /*requested_email=*/true,
      /*requested_phone=*/false, /*requested_name=*/false);
  logger1.SetRequestedPaymentMethodTypes(
      /*requested_basic_card=*/true, /*requested_method_google=*/true,
      /*requested_method_other=*/true);
  logger2.SetEventOccurred(JourneyLogger::EVENT_SHOWN);
  logger2.SetRequestedInformation(
      /*requested_shipping=*/true, /*requested_email=*/false,
      /*requested_phone=*/false, /*requested_name=*/false);
  logger2.SetRequestedPaymentMethodTypes(
      /*requested_basic_card=*/false, /*requested_method_google=*/false,
      /*requested_method_other=*/true);

  logger1.SetCanMakePaymentValue(true);

  logger1.SetNumberOfSuggestionsShown(JourneyLogger::SECTION_PAYMENT_METHOD, 1,
                                      /*has_complete_suggestion=*/false);
  logger2.SetNumberOfSuggestionsShown(JourneyLogger::SECTION_PAYMENT_METHOD, 0,
                                      /*has_complete_suggestion=*/false);

  // Simulate that the user completes one checkout and aborts the other.
  logger1.SetCompleted();
  logger2.SetAborted(JourneyLogger::ABORT_REASON_ABORTED_BY_USER);

  // Make sure the correct events were logged.
  std::vector<base::Bucket> buckets =
      histogram_tester.GetAllSamples("PaymentRequest.Events");
  ASSERT_EQ(2U, buckets.size());
  // logger2
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_SHOWN);
  EXPECT_FALSE(buckets[0].min &
               JourneyLogger::EVENT_HAD_NECESSARY_COMPLETE_SUGGESTIONS);
  EXPECT_FALSE(buckets[0].min &
               JourneyLogger::EVENT_HAD_INITIAL_FORM_OF_PAYMENT);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_OTHER_ABORTED);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_COMPLETED);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_USER_ABORTED);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_REQUEST_SHIPPING);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_NAME);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_PHONE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_PAYER_EMAIL);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_TRUE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_FALSE);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_BASIC_CARD);
  EXPECT_FALSE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_GOOGLE);
  EXPECT_TRUE(buckets[0].min & JourneyLogger::EVENT_REQUEST_METHOD_OTHER);
  // logger1
  EXPECT_TRUE(buckets[1].min & JourneyLogger::EVENT_SHOWN);
  EXPECT_FALSE(buckets[1].min &
               JourneyLogger::EVENT_HAD_NECESSARY_COMPLETE_SUGGESTIONS);
  EXPECT_TRUE(buckets[1].min &
              JourneyLogger::EVENT_HAD_INITIAL_FORM_OF_PAYMENT);
  EXPECT_FALSE(buckets[1].min & JourneyLogger::EVENT_OTHER_ABORTED);
  EXPECT_TRUE(buckets[1].min & JourneyLogger::EVENT_COMPLETED);
  EXPECT_FALSE(buckets[1].min & JourneyLogger::EVENT_USER_ABORTED);
  EXPECT_TRUE(buckets[1].min & JourneyLogger::EVENT_REQUEST_SHIPPING);
  EXPECT_FALSE(buckets[1].min & JourneyLogger::EVENT_REQUEST_PAYER_NAME);
  EXPECT_FALSE(buckets[1].min & JourneyLogger::EVENT_REQUEST_PAYER_PHONE);
  EXPECT_TRUE(buckets[1].min & JourneyLogger::EVENT_REQUEST_PAYER_EMAIL);
  EXPECT_TRUE(buckets[1].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_TRUE);
  EXPECT_FALSE(buckets[1].min & JourneyLogger::EVENT_CAN_MAKE_PAYMENT_FALSE);
  EXPECT_TRUE(buckets[1].min & JourneyLogger::EVENT_REQUEST_METHOD_BASIC_CARD);
  EXPECT_TRUE(buckets[1].min & JourneyLogger::EVENT_REQUEST_METHOD_GOOGLE);
  EXPECT_TRUE(buckets[1].min & JourneyLogger::EVENT_REQUEST_METHOD_OTHER);
}

// Tests that the Payment Request UKMs are logged correctly when the user aborts
// the Payment Request.
TEST(JourneyLoggerTest,
     RecordJourneyStatsHistograms_CheckoutFunnelUkm_UserAborted) {
  using UkmEntry = ukm::builders::PaymentRequest_CheckoutEvents;
  ukm::TestAutoSetUkmRecorder ukm_recorder;
  char test_url[] = "http://www.google.com/";

  base::HistogramTester histogram_tester;
  JourneyLogger logger(/*is_incognito=*/true, /*url=*/GURL(test_url),
                       /*ukm_recorder=*/&ukm_recorder);
  logger.SetRequestedInformation(
      /*requested_shipping=*/true, /*requested_email=*/true,
      /*requested_phone=*/false, /*requested_name=*/false);
  logger.SetRequestedPaymentMethodTypes(
      /*requested_basic_card=*/true, /*requested_method_google=*/false,
      /*requested_method_other=*/false);

  // Simulate that the user aborts after being shown the Payment Request and
  // clicking pay.
  logger.SetEventOccurred(JourneyLogger::EVENT_SHOWN);
  logger.SetEventOccurred(JourneyLogger::EVENT_PAY_CLICKED);
  logger.SetAborted(JourneyLogger::ABORT_REASON_ABORTED_BY_USER);

  // Make sure the UKM was logged correctly.
  ASSERT_EQ(1U, ukm_recorder.sources_count());
  const ukm::UkmSource* source = ukm_recorder.GetSourceForUrl(test_url);
  ASSERT_NE(nullptr, source);

  ASSERT_EQ(1U, ukm_recorder.entries_count());
  const ukm::mojom::UkmEntry* entry = ukm_recorder.GetEntry(0);
  EXPECT_EQ(source->id(), entry->source_id);
  EXPECT_EQ(base::HashMetricName(UkmEntry::kEntryName), entry->event_hash);

  const ukm::mojom::UkmMetric* status_metric =
      ukm::TestUkmRecorder::FindMetric(entry, UkmEntry::kCompletionStatusName);
  ASSERT_NE(nullptr, status_metric);
  EXPECT_EQ(JourneyLogger::COMPLETION_STATUS_USER_ABORTED,
            status_metric->value);

  const ukm::mojom::UkmMetric* step_metric =
      ukm::TestUkmRecorder::FindMetric(entry, UkmEntry::kEventsName);
  ASSERT_NE(nullptr, step_metric);
  EXPECT_EQ(JourneyLogger::EVENT_SHOWN | JourneyLogger::EVENT_PAY_CLICKED |
                JourneyLogger::EVENT_REQUEST_SHIPPING |
                JourneyLogger::EVENT_REQUEST_PAYER_EMAIL |
                JourneyLogger::EVENT_REQUEST_METHOD_BASIC_CARD,
            step_metric->value);
}

// Tests that the Payment Request UKMs are logged correctly when the user
// completes the Payment Request.
TEST(JourneyLoggerTest,
     RecordJourneyStatsHistograms_CheckoutFunnelUkm_Completed) {
  using UkmEntry = ukm::builders::PaymentRequest_CheckoutEvents;
  ukm::TestAutoSetUkmRecorder ukm_recorder;
  char test_url[] = "http://www.google.com/";

  base::HistogramTester histogram_tester;
  JourneyLogger logger(/*is_incognito=*/true, /*url=*/GURL(test_url),
                       /*ukm_recorder=*/&ukm_recorder);
  logger.SetRequestedInformation(
      /*requested_shipping=*/true, /*requested_email=*/true,
      /*requested_phone=*/false, /*requested_name=*/false);
  logger.SetRequestedPaymentMethodTypes(
      /*requested_basic_card=*/true, /*requested_method_google=*/false,
      /*requested_method_other=*/false);

  // Simulate that the user aborts after being shown the Payment Request.
  logger.SetEventOccurred(JourneyLogger::EVENT_SHOWN);
  logger.SetCompleted();

  // Make sure the UKM was logged correctly.
  ASSERT_EQ(1U, ukm_recorder.sources_count());
  const ukm::UkmSource* source = ukm_recorder.GetSourceForUrl(test_url);
  ASSERT_NE(nullptr, source);

  ASSERT_EQ(1U, ukm_recorder.entries_count());
  const ukm::mojom::UkmEntry* entry = ukm_recorder.GetEntry(0);
  EXPECT_EQ(source->id(), entry->source_id);
  EXPECT_EQ(base::HashMetricName(UkmEntry::kEntryName), entry->event_hash);

  const ukm::mojom::UkmMetric* status_metric =
      ukm::TestUkmRecorder::FindMetric(entry, UkmEntry::kCompletionStatusName);
  ASSERT_NE(nullptr, status_metric);
  EXPECT_EQ(JourneyLogger::COMPLETION_STATUS_COMPLETED, status_metric->value);

  const ukm::mojom::UkmMetric* step_metric =
      ukm::TestUkmRecorder::FindMetric(entry, UkmEntry::kEventsName);
  ASSERT_NE(nullptr, step_metric);
  EXPECT_EQ(JourneyLogger::EVENT_SHOWN | JourneyLogger::EVENT_REQUEST_SHIPPING |
                JourneyLogger::EVENT_REQUEST_PAYER_EMAIL |
                JourneyLogger::EVENT_REQUEST_METHOD_BASIC_CARD,
            step_metric->value);
}

}  // namespace payments
