// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <utility>

#include "base/guid.h"
#include "base/memory/ptr_util.h"
#include "components/download/internal/entry.h"
#include "components/download/internal/proto_conversions.h"
#include "components/download/internal/test/entry_utils.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {
std::string TEST_URL = "https://google.com";

}  // namespace

namespace download {

class ProtoConversionsTest : public testing::Test, public ProtoConversions {
 public:
  ~ProtoConversionsTest() override {}
};

TEST_F(ProtoConversionsTest, StateConversion) {
  Entry::State states[] = {Entry::State::NEW, Entry::State::AVAILABLE,
                           Entry::State::ACTIVE, Entry::State::PAUSED,
                           Entry::State::COMPLETE};
  for (auto state : states) {
    ASSERT_EQ(state, RequestStateFromProto(RequestStateToProto(state)));
  }
}

TEST_F(ProtoConversionsTest, DownloadClientConversion) {
  DownloadClient clients[] = {DownloadClient::INVALID, DownloadClient::TEST,
                              DownloadClient::TEST_2, DownloadClient::BOUNDARY};
  for (auto client : clients) {
    ASSERT_EQ(client, DownloadClientFromProto(DownloadClientToProto(client)));
  }
}

TEST_F(ProtoConversionsTest, NetworkRequirementsConversion) {
  SchedulingParams::NetworkRequirements values[] = {
      SchedulingParams::NetworkRequirements::NONE,
      SchedulingParams::NetworkRequirements::OPTIMISTIC,
      SchedulingParams::NetworkRequirements::UNMETERED};
  for (auto value : values) {
    ASSERT_EQ(value,
              NetworkRequirementsFromProto(NetworkRequirementsToProto(value)));
  }
}

TEST_F(ProtoConversionsTest, BatteryRequirementsConversion) {
  SchedulingParams::BatteryRequirements values[] = {
      SchedulingParams::BatteryRequirements::BATTERY_INSENSITIVE,
      SchedulingParams::BatteryRequirements::BATTERY_SENSITIVE};
  for (auto value : values) {
    ASSERT_EQ(value,
              BatteryRequirementsFromProto(BatteryRequirementsToProto(value)));
  }
}

TEST_F(ProtoConversionsTest, SchedulingPriorityConversion) {
  SchedulingParams::Priority values[] = {
      SchedulingParams::Priority::LOW, SchedulingParams::Priority::NORMAL,
      SchedulingParams::Priority::HIGH, SchedulingParams::Priority::UI,
      SchedulingParams::Priority::DEFAULT};
  for (auto value : values) {
    ASSERT_EQ(value,
              SchedulingPriorityFromProto(SchedulingPriorityToProto(value)));
  }
}

TEST_F(ProtoConversionsTest, SchedulingParamsConversion) {
  SchedulingParams expected;
  expected.cancel_time = base::Time::Now();
  expected.priority = SchedulingParams::Priority::DEFAULT;
  expected.network_requirements =
      SchedulingParams::NetworkRequirements::OPTIMISTIC;
  expected.battery_requirements =
      SchedulingParams::BatteryRequirements::BATTERY_SENSITIVE;

  protodb::SchedulingParams proto;
  SchedulingParamsToProto(expected, &proto);
  SchedulingParams actual = SchedulingParamsFromProto(proto);
  EXPECT_EQ(expected.cancel_time, actual.cancel_time);
  EXPECT_EQ(SchedulingParams::Priority::DEFAULT, actual.priority);
  EXPECT_EQ(SchedulingParams::NetworkRequirements::OPTIMISTIC,
            actual.network_requirements);
  EXPECT_EQ(SchedulingParams::BatteryRequirements::BATTERY_SENSITIVE,
            actual.battery_requirements);
}

TEST_F(ProtoConversionsTest, RequestParamsWithHeadersConversion) {
  RequestParams expected;
  expected.url = GURL(TEST_URL);
  expected.method = "GET";
  expected.request_headers.SetHeader("key1", "value1");
  expected.request_headers.SetHeader("key2", "value2");

  protodb::RequestParams proto;
  RequestParamsToProto(expected, &proto);
  RequestParams actual = RequestParamsFromProto(proto);

  EXPECT_EQ(expected.url, actual.url);
  EXPECT_EQ(expected.method, actual.method);

  std::string out;
  actual.request_headers.GetHeader("key1", &out);
  EXPECT_EQ("value1", out);
  actual.request_headers.GetHeader("key2", &out);
  EXPECT_EQ("value2", out);
  EXPECT_EQ(expected.request_headers.ToString(),
            actual.request_headers.ToString());
}

TEST_F(ProtoConversionsTest, EntryConversion) {
  Entry expected = test::BuildEntry(DownloadClient::TEST, base::GenerateGUID());
  Entry actual = EntryFromProto(EntryToProto(expected));
  EXPECT_TRUE(test::CompareEntry(&expected, &actual));

  expected = test::BuildEntry(
      DownloadClient::TEST, base::GenerateGUID(), base::Time::Now(),
      SchedulingParams::NetworkRequirements::OPTIMISTIC,
      SchedulingParams::BatteryRequirements::BATTERY_SENSITIVE,
      SchedulingParams::Priority::HIGH, GURL(TEST_URL), "GET",
      Entry::State::ACTIVE, base::FilePath(FILE_PATH_LITERAL("/test/xyz")),
      base::Time::Now(), base::Time::Now(), base::Time::Now(), 1024u, 3, 5);
  actual = EntryFromProto(EntryToProto(expected));
  EXPECT_TRUE(test::CompareEntry(&expected, &actual));
}

TEST_F(ProtoConversionsTest, EntryVectorConversion) {
  std::vector<Entry> expected;
  expected.push_back(
      test::BuildEntry(DownloadClient::TEST, base::GenerateGUID()));
  expected.push_back(
      test::BuildEntry(DownloadClient::TEST_2, base::GenerateGUID()));
  expected.push_back(test::BuildEntry(
      DownloadClient::TEST, base::GenerateGUID(), base::Time::Now(),
      SchedulingParams::NetworkRequirements::OPTIMISTIC,
      SchedulingParams::BatteryRequirements::BATTERY_SENSITIVE,
      SchedulingParams::Priority::HIGH, GURL(TEST_URL), "GET",
      Entry::State::ACTIVE, base::FilePath(FILE_PATH_LITERAL("/test/xyz")),
      base::Time::Now(), base::Time::Now(), base::Time::Now(), 1024u, 2, 5));

  auto actual = EntryVectorFromProto(
      EntryVectorToProto(base::MakeUnique<std::vector<Entry>>(expected)));
  EXPECT_TRUE(test::CompareEntryList(expected, *actual));
}

}  // namespace download
