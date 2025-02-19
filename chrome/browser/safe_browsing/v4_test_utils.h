// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SAFE_BROWSING_V4_TEST_UTILS_H_
#define CHROME_BROWSER_SAFE_BROWSING_V4_TEST_UTILS_H_

#include <memory>

#include "components/safe_browsing_db/v4_database.h"
#include "components/safe_browsing_db/v4_get_hash_protocol_manager.h"

namespace safe_browsing {

class TestV4Store : public V4Store {
 public:
  TestV4Store(const scoped_refptr<base::SequencedTaskRunner>& task_runner,
              const base::FilePath& store_path);
  ~TestV4Store() override;

  bool HasValidData() const override;

  void MarkPrefixAsBad(HashPrefix prefix);
};

class TestV4StoreFactory : public V4StoreFactory {
 public:
  TestV4StoreFactory();
  ~TestV4StoreFactory() override;

  std::unique_ptr<V4Store> CreateV4Store(
      const scoped_refptr<base::SequencedTaskRunner>& task_runner,
      const base::FilePath& store_path) override;
};

class TestV4Database : public V4Database {
 public:
  TestV4Database(const scoped_refptr<base::SequencedTaskRunner>& db_task_runner,
                 std::unique_ptr<StoreMap> store_map);

  void MarkPrefixAsBad(ListIdentifier list_id, HashPrefix prefix);
};

class TestV4DatabaseFactory : public V4DatabaseFactory {
 public:
  TestV4DatabaseFactory();
  ~TestV4DatabaseFactory() override;

  std::unique_ptr<V4Database> Create(
      const scoped_refptr<base::SequencedTaskRunner>& db_task_runner,
      std::unique_ptr<StoreMap> store_map) override;

  void MarkPrefixAsBad(ListIdentifier list_id, HashPrefix prefix);

 private:
  // Owned by V4LocalDatabaseManager. The following usage is expected: each
  // test in the test fixture instantiates a new SafebrowsingService instance,
  // which instantiates a new V4LocalDatabaseManager, which instantiates a new
  // V4Database using this method so use-after-free isn't possible.
  TestV4Database* v4_db_ = nullptr;
};

class TestV4GetHashProtocolManager : public V4GetHashProtocolManager {
 public:
  TestV4GetHashProtocolManager(
      net::URLRequestContextGetter* request_context_getter,
      const StoresToCheck& stores_to_check,
      const V4ProtocolConfig& config);

  void AddToFullHashCache(FullHashInfo fhi);
};

class TestV4GetHashProtocolManagerFactory
    : public V4GetHashProtocolManagerFactory {
 public:
  TestV4GetHashProtocolManagerFactory();
  ~TestV4GetHashProtocolManagerFactory() override;

  std::unique_ptr<V4GetHashProtocolManager> CreateProtocolManager(
      net::URLRequestContextGetter* request_context_getter,
      const StoresToCheck& stores_to_check,
      const V4ProtocolConfig& config) override;

  void AddToFullHashCache(FullHashInfo fhi) { pm_->AddToFullHashCache(fhi); }

 private:
  // Owned by the SafeBrowsingService.
  TestV4GetHashProtocolManager* pm_ = nullptr;
};

// Returns a FullHash for the basic host+path pattern for a given URL after
// canonicalization.
FullHash GetFullHash(const GURL& url);

// Returns FullHashInfo object for the basic host+path pattern for a given URL
// after canonicalization.
FullHashInfo GetFullHashInfo(const GURL& url, const ListIdentifier& list_id);

// Returns a FullHashInfo info for the basic host+path pattern for a given URL
// after canonicalization. Also adds metadata information to the FullHashInfo
// object.
FullHashInfo GetFullHashInfoWithMetadata(const GURL& url,
                                         const ListIdentifier& list_id,
                                         ThreatPatternType threat_pattern_type);

}  // namespace safe_browsing

#endif  // CHROME_BROWSER_SAFE_BROWSING_V4_TEST_UTILS_H_
