// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/safe_browsing/test_safe_browsing_database_helper.h"

#include <utility>

#include "base/bind.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/location.h"
#include "base/memory/ptr_util.h"
#include "base/memory/ref_counted.h"
#include "base/strings/stringprintf.h"
#include "chrome/browser/safe_browsing/safe_browsing_service.h"
#include "chrome/browser/safe_browsing/test_safe_browsing_service.h"
#include "chrome/browser/safe_browsing/v4_test_utils.h"
#include "components/safe_browsing_db/v4_database.h"
#include "components/safe_browsing_db/v4_protocol_manager_util.h"
#include "components/security_interstitials/content/unsafe_resource.h"

namespace {

// UI manager that never actually shows any interstitials, but emulates as if
// the user chose to proceed through them.
class FakeSafeBrowsingUIManager
    : public safe_browsing::TestSafeBrowsingUIManager {
 public:
  FakeSafeBrowsingUIManager() {}

 protected:
  ~FakeSafeBrowsingUIManager() override {}

  void DisplayBlockingPage(const UnsafeResource& resource) override {
    resource.callback_thread->PostTask(
        FROM_HERE, base::Bind(resource.callback, true /* proceed */));
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(FakeSafeBrowsingUIManager);
};

}  // namespace

// This class automatically inserts lists into the store map when initializing
// the test database.
class InsertingDatabaseFactory : public safe_browsing::TestV4DatabaseFactory {
 public:
  explicit InsertingDatabaseFactory(
      safe_browsing::TestV4StoreFactory* store_factory,
      const std::vector<safe_browsing::ListIdentifier>& lists_to_insert)
      : lists_to_insert_(lists_to_insert), store_factory_(store_factory) {}

  std::unique_ptr<safe_browsing::V4Database> Create(
      const scoped_refptr<base::SequencedTaskRunner>& db_task_runner,
      std::unique_ptr<safe_browsing::StoreMap> store_map) override {
    const base::FilePath base_store_path(FILE_PATH_LITERAL("UrlDb.store"));
    for (const auto& id : lists_to_insert_) {
      if (!base::ContainsKey(*store_map, id)) {
        const base::FilePath store_path =
            base_store_path.InsertBeforeExtensionASCII(base::StringPrintf(
                " (%d)", base::GetUniquePathNumber(
                             base_store_path, base::FilePath::StringType())));
        (*store_map)[id] =
            store_factory_->CreateV4Store(db_task_runner, store_path);
      }
    }

    for (const auto& it : *store_map)
      lists_.push_back(it.first);
    return safe_browsing::TestV4DatabaseFactory::Create(db_task_runner,
                                                        std::move(store_map));
  }

  const std::vector<safe_browsing::ListIdentifier> lists() { return lists_; }

 private:
  std::vector<safe_browsing::ListIdentifier> lists_to_insert_;
  std::vector<safe_browsing::ListIdentifier> lists_;
  safe_browsing::TestV4StoreFactory* store_factory_;
};

TestSafeBrowsingDatabaseHelper::TestSafeBrowsingDatabaseHelper()
    : TestSafeBrowsingDatabaseHelper(
          std::vector<safe_browsing::ListIdentifier>()) {}

TestSafeBrowsingDatabaseHelper::TestSafeBrowsingDatabaseHelper(
    std::vector<safe_browsing::ListIdentifier> lists_to_insert) {
  sb_factory_ = base::MakeUnique<safe_browsing::TestSafeBrowsingServiceFactory>(
      safe_browsing::V4FeatureList::V4UsageStatus::V4_ONLY);
  sb_factory_->SetTestUIManager(new FakeSafeBrowsingUIManager());
  safe_browsing::SafeBrowsingService::RegisterFactory(sb_factory_.get());

  auto store_factory = base::MakeUnique<safe_browsing::TestV4StoreFactory>();
  auto v4_db_factory = base::MakeUnique<InsertingDatabaseFactory>(
      store_factory.get(), lists_to_insert);
  auto v4_get_hash_factory =
      base::MakeUnique<safe_browsing::TestV4GetHashProtocolManagerFactory>();

  v4_db_factory_ = v4_db_factory.get();
  v4_get_hash_factory_ = v4_get_hash_factory.get();

  safe_browsing::V4Database::RegisterStoreFactoryForTest(
      std::move(store_factory));
  safe_browsing::V4Database::RegisterDatabaseFactoryForTest(
      std::move(v4_db_factory));
  safe_browsing::V4GetHashProtocolManager::RegisterFactory(
      std::move(v4_get_hash_factory));
}

TestSafeBrowsingDatabaseHelper::~TestSafeBrowsingDatabaseHelper() {
  safe_browsing::V4GetHashProtocolManager::RegisterFactory(nullptr);
  safe_browsing::V4Database::RegisterDatabaseFactoryForTest(nullptr);
  safe_browsing::V4Database::RegisterStoreFactoryForTest(nullptr);
  safe_browsing::SafeBrowsingService::RegisterFactory(nullptr);
}

void TestSafeBrowsingDatabaseHelper::MarkUrlAsMatchingListWithId(
    const GURL& bad_url,
    const safe_browsing::ListIdentifier& list_id,
    safe_browsing::ThreatPatternType threat_pattern_type) {
  safe_browsing::FullHashInfo full_hash_info =
      GetFullHashInfoWithMetadata(bad_url, list_id, threat_pattern_type);
  v4_db_factory_->MarkPrefixAsBad(list_id, full_hash_info.full_hash);
  v4_get_hash_factory_->AddToFullHashCache(full_hash_info);
}

bool TestSafeBrowsingDatabaseHelper::HasListSynced(
    const safe_browsing::ListIdentifier& list_id) {
  return base::ContainsValue(v4_db_factory_->lists(), list_id);
}
