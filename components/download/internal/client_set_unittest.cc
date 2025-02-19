// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/download/internal/client_set.h"

#include <algorithm>

#include "base/memory/ptr_util.h"
#include "components/download/internal/test/empty_client.h"
#include "components/download/public/clients.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace download {

TEST(DownloadServiceClientSetTest, TestGetClient) {
  auto client = base::MakeUnique<test::EmptyClient>();
  Client* raw_client = client.get();

  auto client_map = base::MakeUnique<DownloadClientMap>();
  client_map->insert(std::make_pair(DownloadClient::TEST, std::move(client)));
  ClientSet clients(std::move(client_map));

  EXPECT_EQ(raw_client, clients.GetClient(DownloadClient::TEST));
  EXPECT_EQ(nullptr, clients.GetClient(DownloadClient::INVALID));
}

TEST(DownloadServiceClientSetTest, TestGetRegisteredClients) {
  auto client = base::MakeUnique<test::EmptyClient>();

  auto client_map = base::MakeUnique<DownloadClientMap>();
  client_map->insert(std::make_pair(DownloadClient::TEST, std::move(client)));
  ClientSet clients(std::move(client_map));

  std::set<DownloadClient> expected_set = {DownloadClient::TEST};
  std::set<DownloadClient> actual_set = clients.GetRegisteredClients();

  EXPECT_EQ(expected_set.size(), actual_set.size());
  EXPECT_TRUE(
      std::equal(expected_set.begin(), expected_set.end(), actual_set.begin()));
}

}  // namespace download
