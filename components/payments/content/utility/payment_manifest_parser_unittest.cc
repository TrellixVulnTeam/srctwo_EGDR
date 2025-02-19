// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/payments/content/utility/payment_manifest_parser.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace payments {
namespace {

// Payment method manifest parsing:

void ExpectUnableToParsePaymentMethodManifest(const std::string& input) {
  std::vector<GURL> actual_web_app_urls;
  std::vector<url::Origin> actual_supported_origins;
  bool actual_all_origins_supported = false;

  PaymentManifestParser::ParsePaymentMethodManifestIntoVectors(
      input, &actual_web_app_urls, &actual_supported_origins,
      &actual_all_origins_supported);

  EXPECT_TRUE(actual_web_app_urls.empty());
  EXPECT_TRUE(actual_supported_origins.empty());
  EXPECT_FALSE(actual_all_origins_supported);
}

void ExpectParsedPaymentMethodManifest(
    const std::string& input,
    const std::vector<GURL>& expected_web_app_urls,
    const std::vector<url::Origin>& expected_supported_origins,
    bool expected_all_origins_supported) {
  std::vector<GURL> actual_web_app_urls;
  std::vector<url::Origin> actual_supported_origins;
  bool actual_all_origins_supported = false;

  PaymentManifestParser::ParsePaymentMethodManifestIntoVectors(
      input, &actual_web_app_urls, &actual_supported_origins,
      &actual_all_origins_supported);

  EXPECT_EQ(expected_web_app_urls, actual_web_app_urls);
  EXPECT_EQ(expected_supported_origins, actual_supported_origins);
  EXPECT_EQ(expected_all_origins_supported, actual_all_origins_supported);
}

TEST(PaymentManifestParserTest, NullPaymentMethodManifestIsMalformed) {
  ExpectUnableToParsePaymentMethodManifest(std::string());
}

TEST(PaymentManifestParserTest, NonJsonPaymentMethodManifestIsMalformed) {
  ExpectUnableToParsePaymentMethodManifest("this is not json");
}

TEST(PaymentManifestParserTest, StringPaymentMethodManifestIsMalformed) {
  ExpectUnableToParsePaymentMethodManifest("\"this is a string\"");
}

TEST(PaymentManifestParserTest,
     EmptyDictionaryPaymentMethodManifestIsMalformed) {
  ExpectUnableToParsePaymentMethodManifest("{}");
}

TEST(PaymentManifestParserTest, NullDefaultApplicationIsMalformed) {
  ExpectUnableToParsePaymentMethodManifest("{\"default_applications\": null}");
}

TEST(PaymentManifestParserTest, NumberDefaultApplicationIsMalformed) {
  ExpectUnableToParsePaymentMethodManifest("{\"default_applications\": 0}");
}

TEST(PaymentManifestParserTest, ListOfNumbersDefaultApplicationIsMalformed) {
  ExpectUnableToParsePaymentMethodManifest("{\"default_applications\": [0]}");
}

TEST(PaymentManifestParserTest, EmptyListOfDefaultApplicationsIsMalformed) {
  ExpectUnableToParsePaymentMethodManifest("{\"default_applications\": []}");
}

TEST(PaymentManifestParserTest, ListOfEmptyDefaultApplicationsIsMalformed) {
  ExpectUnableToParsePaymentMethodManifest(
      "{\"default_applications\": [\"\"]}");
}

TEST(PaymentManifestParserTest, DefaultApplicationsShouldNotHaveNulCharacters) {
  ExpectUnableToParsePaymentMethodManifest(
      "{\"default_applications\": [\"https://bobpay.com/app\0json\"]}");
}

TEST(PaymentManifestParserTest, DefaultApplicationKeyShouldBeLowercase) {
  ExpectUnableToParsePaymentMethodManifest(
      "{\"Default_Applications\": [\"https://bobpay.com/app.json\"]}");
}

TEST(PaymentManifestParserTest, DefaultApplicationsShouldBeAbsoluteUrls) {
  ExpectUnableToParsePaymentMethodManifest(
      "{\"default_applications\": ["
      "\"https://bobpay.com/app.json\","
      "\"app.json\"]}");
}

TEST(PaymentManifestParserTest, DefaultApplicationsShouldBeHttps) {
  ExpectUnableToParsePaymentMethodManifest(
      "{\"default_applications\": ["
      "\"https://bobpay.com/app.json\","
      "\"http://alicepay.com/app.json\"]}");
}

TEST(PaymentManifestParserTest, NullSupportedOriginsIsMalformed) {
  ExpectUnableToParsePaymentMethodManifest("{\"supported_origins\": null}");
}

TEST(PaymentManifestParserTest, NumberSupportedOriginsIsMalformed) {
  ExpectUnableToParsePaymentMethodManifest("{\"supported_origins\": 0}");
}

TEST(PaymentManifestParserTest, EmptyListSupportedOriginsIsMalformed) {
  ExpectUnableToParsePaymentMethodManifest("{\"supported_origins\": []}");
}

TEST(PaymentManifestParserTest, ListOfNumbersSupportedOriginsIsMalformed) {
  ExpectUnableToParsePaymentMethodManifest("{\"supported_origins\": [0]}");
}

TEST(PaymentManifestParserTest, ListOfEmptySupportedOriginsIsMalformed) {
  ExpectUnableToParsePaymentMethodManifest("{\"supported_origins\": [\"\"]}");
}

TEST(PaymentManifestParserTest, SupportedOriginsShouldNotHaveNulCharacters) {
  ExpectUnableToParsePaymentMethodManifest(
      "{\"supported_origins\": [\"https://bob\0pay.com\"]}");
}

TEST(PaymentManifestParserTest, SupportedOriginsShouldBeHttps) {
  ExpectUnableToParsePaymentMethodManifest(
      "{\"supported_origins\": [\"http://bobpay.com\"]}");
}

TEST(PaymentManifestParserTest, SupportedOriginsShouldNotHavePath) {
  ExpectUnableToParsePaymentMethodManifest(
      "{\"supported_origins\": [\"https://bobpay.com/webpay\"]}");
}

TEST(PaymentManifestParserTest, SupportedOriginsShouldNotHaveQuery) {
  ExpectUnableToParsePaymentMethodManifest(
      "{\"supported_origins\": [\"https://bobpay.com/?action=webpay\"]}");
}

TEST(PaymentManifestParserTest, SupportedOriginsShouldNotHaveRef) {
  ExpectUnableToParsePaymentMethodManifest(
      "{\"supported_origins\": [\"https://bobpay.com/#webpay\"]}");
}

TEST(PaymentManifestParserTest, SupportedOriginsShouldBeList) {
  ExpectUnableToParsePaymentMethodManifest(
      "{\"supported_origins\": \"https://bobpay.com\"}");
}

TEST(PaymentManifestParserTest, WellFormedPaymentMethodManifestWithApps) {
  ExpectParsedPaymentMethodManifest(
      "{\"default_applications\": ["
      "\"https://bobpay.com/app.json\","
      "\"https://alicepay.com/app.json\"]}",
      {GURL("https://bobpay.com/app.json"),
       GURL("https://alicepay.com/app.json")},
      std::vector<url::Origin>(), false);
}

TEST(PaymentManifestParserTest,
     WellFormedPaymentMethodManifestWithAppsAndAllSupportedOrigins) {
  ExpectParsedPaymentMethodManifest(
      "{\"default_applications\": [\"https://bobpay.com/app.json\", "
      "\"https://alicepay.com/app.json\"], \"supported_origins\": \"*\"}",
      {GURL("https://bobpay.com/app.json"),
       GURL("https://alicepay.com/app.json")},
      std::vector<url::Origin>(), true);
}

TEST(PaymentManifestParserTest, AllOriginsSupported) {
  ExpectParsedPaymentMethodManifest("{\"supported_origins\": \"*\"}",
                                    std::vector<GURL>(),
                                    std::vector<url::Origin>(), true);
}

TEST(PaymentManifestParserTest,
     InvalidDefaultAppsWillPreventParsingSupportedOrigins) {
  ExpectUnableToParsePaymentMethodManifest(
      "{\"default_applications\": [\"http://bobpay.com/app.json\"], "
      "\"supported_origins\": \"*\"}");
}

TEST(PaymentManifestParserTest,
     InvalidSupportedOriginsWillPreventParsingDefaultApps) {
  ExpectUnableToParsePaymentMethodManifest(
      "{\"default_applications\": [\"https://bobpay.com/app.json\"], "
      "\"supported_origins\": \"+\"}");
}

TEST(PaymentManifestParserTest,
     WellFormedPaymentMethodManifestWithAppsAndSomeSupportedOrigins) {
  ExpectParsedPaymentMethodManifest(
      "{\"default_applications\": [\"https://bobpay.com/app.json\", "
      "\"https://alicepay.com/app.json\"], \"supported_origins\": "
      "[\"https://charliepay.com\", \"https://evepay.com\"]}",
      {GURL("https://bobpay.com/app.json"),
       GURL("https://alicepay.com/app.json")},
      {url::Origin(GURL("https://charliepay.com")),
       url::Origin(GURL("https://evepay.com"))},
      false);
}

TEST(PaymentManifestParserTest,
     WellFormedPaymentMethodManifestWithSomeSupportedOrigins) {
  ExpectParsedPaymentMethodManifest(
      "{\"supported_origins\": [\"https://charliepay.com\", "
      "\"https://evepay.com\"]}",
      std::vector<GURL>(),
      {url::Origin(GURL("https://charliepay.com")),
       url::Origin(GURL("https://evepay.com"))},
      false);
}

TEST(PaymentManifestParserTest,
     WellFormedPaymentMethodManifestWithAllSupportedOrigins) {
  ExpectParsedPaymentMethodManifest("{\"supported_origins\": \"*\"}",
                                    std::vector<GURL>(),
                                    std::vector<url::Origin>(), true);
}

// Web app manifest parsing:

void ExpectUnableToParseWebAppManifest(const std::string& input) {
  std::vector<mojom::WebAppManifestSectionPtr> actual_output =
      PaymentManifestParser::ParseWebAppManifestIntoVector(input);
  EXPECT_TRUE(actual_output.empty());
}

void ExpectParsedWebAppManifest(
    const std::string& input,
    const std::string& expected_id,
    int64_t expected_min_version,
    const std::vector<std::vector<uint8_t>>& expected_fingerprints) {
  std::vector<mojom::WebAppManifestSectionPtr> actual_output =
      PaymentManifestParser::ParseWebAppManifestIntoVector(input);
  ASSERT_EQ(1U, actual_output.size());
  EXPECT_EQ(expected_id, actual_output.front()->id);
  EXPECT_EQ(expected_min_version, actual_output.front()->min_version);
  EXPECT_EQ(expected_fingerprints, actual_output.front()->fingerprints);
}

TEST(PaymentManifestParserTest, NullContentIsMalformed) {
  ExpectUnableToParseWebAppManifest(std::string());
}

TEST(PaymentManifestParserTest, NonJsonContentIsMalformed) {
  ExpectUnableToParseWebAppManifest("this is not json");
}

TEST(PaymentManifestParserTest, StringContentIsMalformed) {
  ExpectUnableToParseWebAppManifest("\"this is a string\"");
}

TEST(PaymentManifestParserTest, EmptyDictionaryIsMalformed) {
  ExpectUnableToParseWebAppManifest("{}");
}

TEST(PaymentManifestParserTest, NullRelatedApplicationsSectionIsMalformed) {
  ExpectUnableToParseWebAppManifest("{\"related_applications\": null}");
}

TEST(PaymentManifestParserTest, NumberRelatedApplicationsectionIsMalformed) {
  ExpectUnableToParseWebAppManifest("{\"related_applications\": 0}");
}

TEST(PaymentManifestParserTest,
     ListOfNumbersRelatedApplicationsSectionIsMalformed) {
  ExpectUnableToParseWebAppManifest("{\"related_applications\": [0]}");
}

TEST(PaymentManifestParserTest,
     ListOfEmptyDictionariesRelatedApplicationsSectionIsMalformed) {
  ExpectUnableToParseWebAppManifest("{\"related_applications\": [{}]}");
}

TEST(PaymentManifestParserTest, NoPlayPlatformIsMalformed) {
  ExpectUnableToParseWebAppManifest(
      "{"
      "  \"related_applications\": [{"
      "    \"id\": \"com.bobpay.app\", "
      "    \"min_version\": \"1\", "
      "    \"fingerprints\": [{"
      "      \"type\": \"sha256_cert\", "
      "      \"value\": \"00:01:02:03:04:05:06:07:08:09:A0:A1:A2:A3:A4:A5:A6:A7"
      ":A8:A9:B0:B1:B2:B3:B4:B5:B6:B7:B8:B9:C0:C1\""
      "    }]"
      "  }]"
      "}");
}

TEST(PaymentManifestParserTest, NoPackageNameIsMalformed) {
  ExpectUnableToParseWebAppManifest(
      "{"
      "  \"related_applications\": [{"
      "    \"platform\": \"play\", "
      "    \"min_version\": \"1\", "
      "    \"fingerprints\": [{"
      "      \"type\": \"sha256_cert\", "
      "      \"value\": \"00:01:02:03:04:05:06:07:08:09:A0:A1:A2:A3:A4:A5:A6:A7"
      ":A8:A9:B0:B1:B2:B3:B4:B5:B6:B7:B8:B9:C0:C1\""
      "    }]"
      "  }]"
      "}");
}

TEST(PaymentManifestParserTest, NoVersionIsMalformed) {
  ExpectUnableToParseWebAppManifest(
      "{"
      "  \"related_applications\": [{"
      "    \"platform\": \"play\", "
      "    \"id\": \"com.bobpay.app\", "
      "    \"fingerprints\": [{"
      "      \"type\": \"sha256_cert\", "
      "      \"value\": \"00:01:02:03:04:05:06:07:08:09:A0:A1:A2:A3:A4:A5:A6:A7"
      ":A8:A9:B0:B1:B2:B3:B4:B5:B6:B7:B8:B9:C0:C1\""
      "    }]"
      "  }]"
      "}");
}

TEST(PaymentManifestParserTest, NoFingerprintIsMalformed) {
  ExpectUnableToParseWebAppManifest(
      "{"
      "  \"related_applications\": [{"
      "    \"platform\": \"play\", "
      "    \"id\": \"com.bobpay.app\", "
      "    \"min_version\": \"1\""
      "  }]"
      "}");
}

TEST(PaymentManifestParserTest, EmptyFingerprintsIsMalformed) {
  ExpectUnableToParseWebAppManifest(
      "{"
      "  \"related_applications\": [{"
      "    \"platform\": \"play\", "
      "    \"id\": \"com.bobpay.app\", "
      "    \"min_version\": \"1\", "
      "    \"fingerprints\": []"
      "  }]"
      "}");
}

TEST(PaymentManifestParserTest, EmptyFingerprintsDictionaryIsMalformed) {
  ExpectUnableToParseWebAppManifest(
      "{"
      "  \"related_applications\": [{"
      "    \"platform\": \"play\", "
      "    \"id\": \"com.bobpay.app\", "
      "    \"min_version\": \"1\", "
      "    \"fingerprints\": [{}]"
      "  }]"
      "}");
}

TEST(PaymentManifestParserTest, NoFingerprintTypeIsMalformed) {
  ExpectUnableToParseWebAppManifest(
      "{"
      "  \"related_applications\": [{"
      "    \"platform\": \"play\", "
      "    \"id\": \"com.bobpay.app\", "
      "    \"min_version\": \"1\", "
      "    \"fingerprints\": [{"
      "      \"value\": \"00:01:02:03:04:05:06:07:08:09:A0:A1:A2:A3:A4:A5:A6:A7"
      ":A8:A9:B0:B1:B2:B3:B4:B5:B6:B7:B8:B9:C0:C1\""
      "    }]"
      "  }]"
      "}");
}

TEST(PaymentManifestParserTest, NoFingerprintValueIsMalformed) {
  ExpectUnableToParseWebAppManifest(
      "{"
      "  \"related_applications\": [{"
      "    \"platform\": \"play\", "
      "    \"id\": \"com.bobpay.app\", "
      "    \"min_version\": \"1\", "
      "    \"fingerprints\": [{"
      "      \"type\": \"sha256_cert\""
      "    }]"
      "  }]"
      "}");
}

TEST(PaymentManifestParserTest, PlatformShouldNotHaveNullCharacters) {
  ExpectUnableToParseWebAppManifest(
      "{"
      "  \"related_applications\": [{"
      "    \"platform\": \"pl\0ay\", "
      "    \"id\": \"com.bobpay.app\", "
      "    \"min_version\": \"1\", "
      "    \"fingerprints\": [{"
      "      \"type\": \"sha256_cert\", "
      "      \"value\": \"00:01:02:03:04:05:06:07:08:09:A0:A1:A2:A3:A4:A5:A6:A7"
      ":A8:A9:B0:B1:B2:B3:B4:B5:B6:B7:B8:B9:C0:C1\""
      "    }]"
      "  }]"
      "}");
}

TEST(PaymentManifestParserTest, PackageNameShouldNotHaveNullCharacters) {
  ExpectUnableToParseWebAppManifest(
      "{"
      "  \"related_applications\": [{"
      "    \"platform\": \"play\", "
      "    \"id\": \"com.bob\0pay.app\", "
      "    \"min_version\": \"1\", "
      "    \"fingerprints\": [{"
      "      \"type\": \"sha256_cert\", "
      "      \"value\": \"00:01:02:03:04:05:06:07:08:09:A0:A1:A2:A3:A4:A5:A6:A7"
      ":A8:A9:B0:B1:B2:B3:B4:B5:B6:B7:B8:B9:C0:C1\""
      "    }]"
      "  }]"
      "}");
}

TEST(PaymentManifestParserTest, VersionShouldNotHaveNullCharacters) {
  ExpectUnableToParseWebAppManifest(
      "{"
      "  \"related_applications\": [{"
      "    \"platform\": \"play\", "
      "    \"id\": \"com.bobpay.app\", "
      "    \"min_version\": \"1\01\", "
      "    \"fingerprints\": [{"
      "      \"type\": \"sha256_cert\", "
      "      \"value\": \"00:01:02:03:04:05:06:07:08:09:A0:A1:A2:A3:A4:A5:A6:A7"
      ":A8:A9:B0:B1:B2:B3:B4:B5:B6:B7:B8:B9:C0:C1\""
      "    }]"
      "  }]"
      "}");
}

TEST(PaymentManifestParserTest, FingerprintTypeShouldNotHaveNullCharacters) {
  ExpectUnableToParseWebAppManifest(
      "{"
      "  \"related_applications\": [{"
      "    \"platform\": \"play\", "
      "    \"id\": \"com.bobpay.app\", "
      "    \"min_version\": \"1\", "
      "    \"fingerprints\": [{"
      "      \"type\": \"sha256_c\0ert\", "
      "      \"value\": \"00:01:02:03:04:05:06:07:08:09:A0:A1:A2:A3:A4:A5:A6:A7"
      ":A8:A9:B0:B1:B2:B3:B4:B5:B6:B7:B8:B9:C0:C1\""
      "    }]"
      "  }]"
      "}");
}

TEST(PaymentManifestParserTest, FingerprintValueShouldNotHaveNullCharacters) {
  ExpectUnableToParseWebAppManifest(
      "{"
      "  \"related_applications\": [{"
      "    \"platform\": \"play\", "
      "    \"id\": \"com.bobpay.app\", "
      "    \"min_version\": \"1\", "
      "    \"fingerprints\": [{"
      "      \"type\": \"sha256_cert\", "
      "      \"value\": "
      "\"00:01:02:0\0:04:05:06:07:08:09:A0:A1:A2:A3:A4:A5:A6:A7"
      ":A8:A9:B0:B1:B2:B3:B4:B5:B6:B7:B8:B9:C0:C1\""
      "    }]"
      "  }]"
      "}");
}

TEST(PaymentManifestParserTest, KeysShouldBeLowerCase) {
  ExpectUnableToParseWebAppManifest(
      "{"
      "  \"Related_applications\": [{"
      "    \"Platform\": \"play\", "
      "    \"Id\": \"com.bobpay.app\", "
      "    \"Min_version\": \"1\", "
      "    \"Fingerprints\": [{"
      "      \"Type\": \"sha256_cert\", "
      "      \"Value\": \"00:01:02:03:04:05:06:07:08:09:A0:A1:A2:A3:A4:A5:A6:A7"
      ":A8:A9:B0:B1:B2:B3:B4:B5:B6:B7:B8:B9:C0:C1\""
      "    }]"
      "  }]"
      "}");
}

TEST(PaymentManifestParserTest, FingerprintsShouldBeSha256) {
  ExpectUnableToParseWebAppManifest(
      "{"
      "  \"related_applications\": [{"
      "    \"platform\": \"play\", "
      "    \"id\": \"com.bobpay.app\", "
      "    \"min_version\": \"1\", "
      "    \"fingerprints\": [{"
      "      \"type\": \"sha1_cert\", "
      "      \"value\": \"00:01:02:03:04:05:06:07:08:09:A0:A1:A2:A3:A4:A5:A6:A7"
      ":A8:A9\""
      "    }]"
      "  }]"
      "}");
}

TEST(PaymentManifestParserTest, FingerprintBytesShouldBeColonSeparated) {
  ExpectUnableToParseWebAppManifest(
      "{"
      "  \"related_applications\": [{"
      "    \"platform\": \"play\", "
      "    \"id\": \"com.bobpay.app\", "
      "    \"min_version\": \"1\", "
      "    \"fingerprints\": [{"
      "      \"type\": \"sha256_cert\", "
      "      \"value\": \"000010020030040050060070080090A00A10A20A30A40A50A60A7"
      "0A80A90B00B10B20B30B40B50B60B70B80B90C00C1\""
      "    }]"
      "  }]"
      "}");
}

TEST(PaymentManifestParserTest, FingerprintBytesShouldBeUpperCase) {
  ExpectUnableToParseWebAppManifest(
      "{"
      "  \"related_applications\": [{"
      "    \"platform\": \"play\", "
      "    \"id\": \"com.bobpay.app\", "
      "    \"min_version\": \"1\", "
      "    \"fingerprints\": [{"
      "      \"type\": \"sha256_cert\", "
      "      \"value\": \"00:01:02:03:04:05:06:07:08:09:a0:a1:a2:a3:a4:a5:a6:a7"
      ":a8:a9:b0:b1:b2:b3:b4:b5:b6:b7:b8:b9:c0:c1\""
      "    }]"
      "  }]"
      "}");
}

TEST(PaymentManifestParserTest, FingerprintsShouldContainsThirtyTwoBytes) {
  ExpectUnableToParseWebAppManifest(
      "{"
      "  \"related_applications\": [{"
      "    \"platform\": \"play\", "
      "    \"id\": \"com.bobpay.app\", "
      "    \"min_version\": \"1\", "
      "    \"fingerprints\": [{"
      "      \"type\": \"sha256_cert\", "
      "      \"value\": \"00:01:02:03:04:05:06:07:08:09:A0:A1:A2:A3:A4:A5:A6:A7"
      ":A8:A9:B0:B1:B2:B3:B4:B5:B6:B7:B8:B9:C0:C1:C2\""
      "    }]"
      "  }]"
      "}");
  ExpectUnableToParseWebAppManifest(
      "{"
      "  \"related_applications\": [{"
      "    \"platform\": \"play\", "
      "    \"id\": \"com.bobpay.app\", "
      "    \"min_version\": \"1\", "
      "    \"fingerprints\": [{"
      "      \"type\": \"sha256_cert\", "
      "      \"value\": \"00:01:02:03:04:05:06:07:08:09:A0:A1:A2:A3:A4:A5:A6:A7"
      ":A8:A9:B0:B1:B2:B3:B4:B5:B6:B7:B8:B9:C0\""
      "    }]"
      "  }]"
      "}");
}

TEST(PaymentManifestParserTest, WellFormed) {
  ExpectParsedWebAppManifest(
      "{"
      "  \"related_applications\": [{"
      "    \"platform\": \"play\", "
      "    \"id\": \"com.bobpay.app\", "
      "    \"min_version\": \"1\", "
      "    \"fingerprints\": [{"
      "      \"type\": \"sha256_cert\", "
      "      \"value\": \"00:01:02:03:04:05:06:07:08:09:A0:A1:A2:A3:A4:A5:A6:A7"
      ":A8:A9:B0:B1:B2:B3:B4:B5:B6:B7:B8:B9:C0:C1\""
      "    }]"
      "  }]"
      "}",
      "com.bobpay.app", 1,
      {{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xA0,
        0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xB0, 0xB1,
        0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xC0, 0xC1}});
}

TEST(PaymentManifestParserTest, DuplicateSignaturesWellFormed) {
  ExpectParsedWebAppManifest(
      "{"
      "  \"related_applications\": [{"
      "    \"platform\": \"play\", "
      "    \"id\": \"com.bobpay.app\", "
      "    \"min_version\": \"1\", "
      "    \"fingerprints\": [{"
      "      \"type\": \"sha256_cert\", "
      "      \"value\": \"00:01:02:03:04:05:06:07:08:09:A0:A1:A2:A3:A4:A5:A6:A7"
      ":A8:A9:B0:B1:B2:B3:B4:B5:B6:B7:B8:B9:C0:C1\""
      "    }, {"
      "      \"type\": \"sha256_cert\", "
      "      \"value\": \"00:01:02:03:04:05:06:07:08:09:A0:A1:A2:A3:A4:A5:A6:A7"
      ":A8:A9:B0:B1:B2:B3:B4:B5:B6:B7:B8:B9:C0:C1\""
      "    }]"
      "  }]"
      "}",
      "com.bobpay.app", 1,
      {{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xA0,
        0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xB0, 0xB1,
        0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xC0, 0xC1},
       {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xA0,
        0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xB0, 0xB1,
        0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xC0, 0xC1}});
}

TEST(PaymentManifestParserTest, TwoDifferentSignaturesWellFormed) {
  ExpectParsedWebAppManifest(
      "{"
      "  \"related_applications\": [{"
      "    \"platform\": \"play\", "
      "    \"id\": \"com.bobpay.app\", "
      "    \"min_version\": \"1\", "
      "    \"fingerprints\": [{"
      "      \"type\": \"sha256_cert\", "
      "      \"value\": \"00:01:02:03:04:05:06:07:08:09:A0:A1:A2:A3:A4:A5:A6:A7"
      ":A8:A9:B0:B1:B2:B3:B4:B5:B6:B7:B8:B9:C0:C1\""
      "    }, {"
      "      \"type\": \"sha256_cert\", "
      "      \"value\": \"AA:01:02:03:04:05:06:07:08:09:A0:A1:A2:A3:A4:A5:A6:A7"
      ":A8:A9:B0:B1:B2:B3:B4:B5:B6:B7:B8:B9:C0:C1\""
      "    }]"
      "  }]"
      "}",
      "com.bobpay.app", 1,
      {{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xA0,
        0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xB0, 0xB1,
        0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xC0, 0xC1},
       {0xAA, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xA0,
        0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xB0, 0xB1,
        0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xC0, 0xC1}});
}

TEST(PaymentManifestParserTest, TwoRelatedApplicationsWellFormed) {
  std::vector<mojom::WebAppManifestSectionPtr> actual_output =
      PaymentManifestParser::ParseWebAppManifestIntoVector(
          "{"
          "  \"related_applications\": [{"
          "    \"platform\": \"play\", "
          "    \"id\": \"com.bobpay.app.dev\", "
          "    \"min_version\": \"2\", "
          "    \"fingerprints\": [{"
          "      \"type\": \"sha256_cert\", "
          "      \"value\": "
          "\"00:01:02:03:04:05:06:07:08:09:A0:A1:A2:A3:A4:A5:A6:A7"
          ":A8:A9:B0:B1:B2:B3:B4:B5:B6:B7:B8:B9:C0:C1\""
          "    }]"
          "  }, {"
          "    \"platform\": \"play\", "
          "    \"id\": \"com.bobpay.app.prod\", "
          "    \"min_version\": \"1\", "
          "    \"fingerprints\": [{"
          "      \"type\": \"sha256_cert\", "
          "      \"value\": "
          "\"AA:01:02:03:04:05:06:07:08:09:A0:A1:A2:A3:A4:A5:A6:A7"
          ":A8:A9:B0:B1:B2:B3:B4:B5:B6:B7:B8:B9:C0:C1\""
          "    }]"
          "  }]"
          "}");

  ASSERT_EQ(2U, actual_output.size());

  EXPECT_EQ("com.bobpay.app.dev", actual_output.front()->id);
  EXPECT_EQ(2, actual_output.front()->min_version);
  EXPECT_EQ(
      std::vector<std::vector<uint8_t>>(
          {{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xA0,
            0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xB0, 0xB1,
            0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xC0, 0xC1}}),
      actual_output.front()->fingerprints);

  EXPECT_EQ("com.bobpay.app.prod", actual_output.back()->id);
  EXPECT_EQ(1, actual_output.back()->min_version);
  EXPECT_EQ(
      std::vector<std::vector<uint8_t>>(
          {{0xAA, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xA0,
            0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xB0, 0xB1,
            0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xC0, 0xC1}}),
      actual_output.back()->fingerprints);
}

}  // namespace
}  // namespace payments
