// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/certificate_reporting/error_report.h"

#include <set>
#include <string>

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/memory/ptr_util.h"
#include "base/message_loop/message_loop.h"
#include "base/path_service.h"
#include "base/threading/thread.h"
#include "base/time/default_clock.h"
#include "base/time/default_tick_clock.h"
#include "build/build_config.h"
#include "components/certificate_reporting/cert_logger.pb.h"
#include "components/network_time/network_time_test_utils.h"
#include "components/prefs/testing_pref_service.h"
#include "components/version_info/version_info.h"
#include "net/cert/cert_status_flags.h"
#include "net/ssl/ssl_info.h"
#include "net/test/cert_test_util.h"
#include "net/test/test_data_directory.h"
#include "net/url_request/url_request_test_util.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

#if defined(OS_ANDROID)
#include "base/test/scoped_feature_list.h"
#include "net/cert/cert_verify_proc_android.h"
#endif

using net::SSLInfo;
using testing::UnorderedElementsAre;
using testing::UnorderedElementsAreArray;

namespace certificate_reporting {

namespace {

const char kDummyHostname[] = "dummy.hostname.com";
const char kDummyFailureLog[] = "dummy failure log";
const char kTestCertFilename[] = "x509_verify_results.chain.pem";

const net::CertStatus kCertStatus =
    net::CERT_STATUS_COMMON_NAME_INVALID | net::CERT_STATUS_REVOKED;

const CertLoggerRequest::CertError kFirstReportedCertError =
    CertLoggerRequest::ERR_CERT_COMMON_NAME_INVALID;
const CertLoggerRequest::CertError kSecondReportedCertError =
    CertLoggerRequest::ERR_CERT_REVOKED;

// Whether to include an unverified certificate chain in the test
// SSLInfo. In production code, an unverified cert chain will not be
// present if the resource was loaded from cache.
enum UnverifiedCertChainStatus {
  INCLUDE_UNVERIFIED_CERT_CHAIN,
  EXCLUDE_UNVERIFIED_CERT_CHAIN
};

void GetTestSSLInfo(UnverifiedCertChainStatus unverified_cert_chain_status,
                    SSLInfo* info,
                    net::CertStatus cert_status) {
  info->cert =
      net::ImportCertFromFile(net::GetTestCertsDirectory(), kTestCertFilename);
  ASSERT_TRUE(info->cert);
  if (unverified_cert_chain_status == INCLUDE_UNVERIFIED_CERT_CHAIN) {
    info->unverified_cert = net::ImportCertFromFile(
        net::GetTestCertsDirectory(), kTestCertFilename);
    ASSERT_TRUE(info->unverified_cert);
  }
  info->is_issued_by_known_root = true;
  info->cert_status = cert_status;
  info->pinning_failure_log = kDummyFailureLog;
}

std::string GetPEMEncodedChain() {
  std::string cert_data;
  std::vector<std::string> pem_certs;
  scoped_refptr<net::X509Certificate> cert =
      net::ImportCertFromFile(net::GetTestCertsDirectory(), kTestCertFilename);
  if (!cert || !cert->GetPEMEncodedChain(&pem_certs)) {
    ADD_FAILURE();
    return cert_data;
  }
  for (const auto& cert : pem_certs) {
    cert_data += cert;
  }
  return cert_data;
}

void VerifyErrorReportSerialization(
    const ErrorReport& report,
    const SSLInfo& ssl_info,
    std::vector<CertLoggerRequest::CertError> cert_errors) {
  std::string serialized_report;
  ASSERT_TRUE(report.Serialize(&serialized_report));

  CertLoggerRequest deserialized_report;
  ASSERT_TRUE(deserialized_report.ParseFromString(serialized_report));
  EXPECT_EQ(kDummyHostname, deserialized_report.hostname());
  EXPECT_EQ(GetPEMEncodedChain(), deserialized_report.cert_chain());
  EXPECT_EQ(GetPEMEncodedChain(), deserialized_report.unverified_cert_chain());
  EXPECT_EQ(1, deserialized_report.pin().size());
  EXPECT_EQ(kDummyFailureLog, deserialized_report.pin().Get(0));
  EXPECT_EQ(
      ssl_info.is_issued_by_known_root,
      deserialized_report.is_issued_by_known_root());
  EXPECT_THAT(deserialized_report.cert_error(),
              UnorderedElementsAreArray(cert_errors));
}

// Test that a serialized ErrorReport can be deserialized as
// a CertLoggerRequest protobuf (which is the format that the receiving
// server expects it in) with the right data in it.
TEST(ErrorReportTest, SerializedReportAsProtobuf) {
  SSLInfo ssl_info;
  ASSERT_NO_FATAL_FAILURE(
      GetTestSSLInfo(INCLUDE_UNVERIFIED_CERT_CHAIN, &ssl_info, kCertStatus));
  ErrorReport report_known(kDummyHostname, ssl_info);
  std::vector<CertLoggerRequest::CertError> cert_errors;
  cert_errors.push_back(kFirstReportedCertError);
  cert_errors.push_back(kSecondReportedCertError);
  ASSERT_NO_FATAL_FAILURE(
      VerifyErrorReportSerialization(report_known, ssl_info, cert_errors));
  // Test that both values for |is_issued_by_known_root| are serialized
  // correctly.
  ssl_info.is_issued_by_known_root = false;
  ErrorReport report_unknown(kDummyHostname, ssl_info);
  ASSERT_NO_FATAL_FAILURE(
      VerifyErrorReportSerialization(report_unknown, ssl_info, cert_errors));
}

TEST(ErrorReportTest, SerializedReportAsProtobufWithInterstitialInfo) {
  std::string serialized_report;
  SSLInfo ssl_info;
  // Use EXCLUDE_UNVERIFIED_CERT_CHAIN here to exercise the code path
  // where SSLInfo does not contain the unverified cert chain. (The test
  // above exercises the path where it does.)
  ASSERT_NO_FATAL_FAILURE(
      GetTestSSLInfo(EXCLUDE_UNVERIFIED_CERT_CHAIN, &ssl_info, kCertStatus));
  ErrorReport report(kDummyHostname, ssl_info);

  const base::Time interstitial_time = base::Time::Now();
  report.SetInterstitialInfo(
      ErrorReport::INTERSTITIAL_CLOCK, ErrorReport::USER_PROCEEDED,
      ErrorReport::INTERSTITIAL_OVERRIDABLE, interstitial_time);

  ASSERT_TRUE(report.Serialize(&serialized_report));

  CertLoggerRequest deserialized_report;
  ASSERT_TRUE(deserialized_report.ParseFromString(serialized_report));
  EXPECT_EQ(kDummyHostname, deserialized_report.hostname());
  EXPECT_EQ(GetPEMEncodedChain(), deserialized_report.cert_chain());
  EXPECT_EQ(std::string(), deserialized_report.unverified_cert_chain());
  EXPECT_EQ(1, deserialized_report.pin().size());
  EXPECT_EQ(kDummyFailureLog, deserialized_report.pin().Get(0));

  EXPECT_EQ(CertLoggerInterstitialInfo::INTERSTITIAL_CLOCK,
            deserialized_report.interstitial_info().interstitial_reason());
  EXPECT_EQ(true, deserialized_report.interstitial_info().user_proceeded());
  EXPECT_EQ(true, deserialized_report.interstitial_info().overridable());
  EXPECT_EQ(
      ssl_info.is_issued_by_known_root,
      deserialized_report.is_issued_by_known_root());

  EXPECT_THAT(
      deserialized_report.cert_error(),
      UnorderedElementsAre(kFirstReportedCertError, kSecondReportedCertError));

  EXPECT_EQ(
      interstitial_time.ToInternalValue(),
      deserialized_report.interstitial_info().interstitial_created_time_usec());
}

// Test that a serialized report can be parsed.
TEST(ErrorReportTest, ParseSerializedReport) {
  std::string serialized_report;
  SSLInfo ssl_info;
  ASSERT_NO_FATAL_FAILURE(
      GetTestSSLInfo(INCLUDE_UNVERIFIED_CERT_CHAIN, &ssl_info, kCertStatus));
  ErrorReport report(kDummyHostname, ssl_info);
  EXPECT_EQ(kDummyHostname, report.hostname());
  ASSERT_TRUE(report.Serialize(&serialized_report));

  ErrorReport parsed;
  ASSERT_TRUE(parsed.InitializeFromString(serialized_report));
  EXPECT_EQ(report.hostname(), parsed.hostname());
}

// Check that CT errors are handled correctly.
TEST(ErrorReportTest, CertificateTransparencyError) {
  SSLInfo ssl_info;
  ASSERT_NO_FATAL_FAILURE(
      GetTestSSLInfo(INCLUDE_UNVERIFIED_CERT_CHAIN, &ssl_info,
                     net::CERT_STATUS_CERTIFICATE_TRANSPARENCY_REQUIRED));
  ErrorReport report_known(kDummyHostname, ssl_info);
  std::vector<CertLoggerRequest::CertError> cert_errors;
  cert_errors.push_back(
      CertLoggerRequest::ERR_CERTIFICATE_TRANSPARENCY_REQUIRED);
  ASSERT_NO_FATAL_FAILURE(
      VerifyErrorReportSerialization(report_known, ssl_info, cert_errors));
}

// Tests that information about network time querying is included in the
// report.
TEST(ErrorReportTest, NetworkTimeQueryingFeatureInfo) {
  base::Thread io_thread("IO thread");
  base::Thread::Options thread_options;
  thread_options.message_loop_type = base::MessageLoop::TYPE_IO;
  EXPECT_TRUE(io_thread.StartWithOptions(thread_options));

  std::unique_ptr<network_time::FieldTrialTest> field_trial_test(
      new network_time::FieldTrialTest());
  field_trial_test->SetNetworkQueriesWithVariationsService(
      true, 0.0, network_time::NetworkTimeTracker::FETCHES_ON_DEMAND_ONLY);

  TestingPrefServiceSimple pref_service;
  network_time::NetworkTimeTracker::RegisterPrefs(pref_service.registry());
  network_time::NetworkTimeTracker network_time_tracker(
      base::MakeUnique<base::DefaultClock>(),
      base::MakeUnique<base::DefaultTickClock>(), &pref_service,
      new net::TestURLRequestContextGetter(io_thread.task_runner()));

  // Serialize a report containing information about the network time querying
  // feature.
  SSLInfo ssl_info;
  ASSERT_NO_FATAL_FAILURE(
      GetTestSSLInfo(INCLUDE_UNVERIFIED_CERT_CHAIN, &ssl_info, kCertStatus));
  ErrorReport report(kDummyHostname, ssl_info);
  report.AddNetworkTimeInfo(&network_time_tracker);
  std::string serialized_report;
  ASSERT_TRUE(report.Serialize(&serialized_report));

  // Check that the report contains the network time querying feature
  // information.
  CertLoggerRequest parsed;
  ASSERT_TRUE(parsed.ParseFromString(serialized_report));
  EXPECT_TRUE(parsed.features_info()
                  .network_time_querying_info()
                  .network_time_queries_enabled());
  EXPECT_EQ(CertLoggerFeaturesInfo::NetworkTimeQueryingInfo::
                NETWORK_TIME_FETCHES_ON_DEMAND_ONLY,
            parsed.features_info()
                .network_time_querying_info()
                .network_time_query_behavior());
}

TEST(ErrorReportTest, TestChromeChannelIncluded) {
  struct ChannelTestCase {
    version_info::Channel channel;
    CertLoggerRequest::ChromeChannel expected_channel;
  } kTestCases[] = {
      {version_info::Channel::UNKNOWN,
       CertLoggerRequest::CHROME_CHANNEL_UNKNOWN},
      {version_info::Channel::DEV, CertLoggerRequest::CHROME_CHANNEL_DEV},
      {version_info::Channel::CANARY, CertLoggerRequest::CHROME_CHANNEL_CANARY},
      {version_info::Channel::BETA, CertLoggerRequest::CHROME_CHANNEL_BETA},
      {version_info::Channel::STABLE,
       CertLoggerRequest::CHROME_CHANNEL_STABLE}};

  // Create a report, set its channel value and check if we
  // get back test_case.expected_channel.
  for (const ChannelTestCase& test_case : kTestCases) {
    SSLInfo ssl_info;
    ASSERT_NO_FATAL_FAILURE(
        GetTestSSLInfo(INCLUDE_UNVERIFIED_CERT_CHAIN, &ssl_info, kCertStatus));
    ErrorReport report(kDummyHostname, ssl_info);

    report.AddChromeChannel(test_case.channel);
    std::string serialized_report;
    ASSERT_TRUE(report.Serialize(&serialized_report));

    CertLoggerRequest parsed;
    ASSERT_TRUE(parsed.ParseFromString(serialized_report));
    EXPECT_EQ(test_case.expected_channel, parsed.chrome_channel());
  }
}

#if defined(OS_WIN) || defined(OS_CHROMEOS)
// Tests that the SetIsEnterpriseManaged() function populates
// is_enterprise_managed correctly on Windows, and that value is correctly
// extracted from the parsed report.
// These tests are OS specific because SetIsEnterpriseManaged is called only
// on the Windows and ChromeOS OS.
TEST(ErrorReportTest, TestIsEnterpriseManagedPopulatedOnWindows) {
  SSLInfo ssl_info;
  ASSERT_NO_FATAL_FAILURE(
      GetTestSSLInfo(INCLUDE_UNVERIFIED_CERT_CHAIN, &ssl_info, kCertStatus));
  ErrorReport report(kDummyHostname, ssl_info);

  report.SetIsEnterpriseManaged(true);
  std::string serialized_report;
  ASSERT_TRUE(report.Serialize(&serialized_report));

  CertLoggerRequest parsed;
  ASSERT_TRUE(parsed.ParseFromString(serialized_report));
  EXPECT_EQ(true, parsed.is_enterprise_managed());
}
#endif

#if defined(OS_ANDROID)
// Tests that information about the Android AIA fetching feature is included in
// the report.
TEST(ErrorReportTest, AndroidAIAFetchingFeatureEnabled) {
  SSLInfo ssl_info;
  ASSERT_NO_FATAL_FAILURE(
      GetTestSSLInfo(INCLUDE_UNVERIFIED_CERT_CHAIN, &ssl_info, kCertStatus));
  ErrorReport report(kDummyHostname, ssl_info);
  std::string serialized_report;
  ASSERT_TRUE(report.Serialize(&serialized_report));
  CertLoggerRequest parsed;
  ASSERT_TRUE(parsed.ParseFromString(serialized_report));
  EXPECT_EQ(CertLoggerFeaturesInfo::ANDROID_AIA_FETCHING_ENABLED,
            parsed.features_info().android_aia_fetching_status());
}
#endif

}  // namespace

}  // namespace certificate_reporting
