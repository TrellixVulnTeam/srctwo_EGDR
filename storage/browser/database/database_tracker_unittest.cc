// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>
#include <stdint.h>

#include <memory>

#include "base/files/file.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/memory/ptr_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/test/scoped_task_environment.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "base/time/time.h"
#include "net/base/net_errors.h"
#include "net/base/test_completion_callback.h"
#include "storage/browser/database/database_tracker.h"
#include "storage/browser/quota/quota_manager_proxy.h"
#include "storage/browser/test/mock_special_storage_policy.h"
#include "storage/common/database/database_identifier.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/sqlite/sqlite3.h"

using base::ASCIIToUTF16;
using storage::DatabaseConnections;
using storage::DatabaseTracker;
using storage::OriginInfo;

namespace {

const char kOrigin1Url[] = "http://origin1";
const char kOrigin2Url[] = "http://protected_origin2";

class TestObserver : public storage::DatabaseTracker::Observer {
 public:
  TestObserver()
      : new_notification_received_(false),
        observe_size_changes_(true),
        observe_scheduled_deletions_(true) {}
  TestObserver(bool observe_size_changes, bool observe_scheduled_deletions)
      : new_notification_received_(false),
        observe_size_changes_(observe_size_changes),
        observe_scheduled_deletions_(observe_scheduled_deletions) {}

  ~TestObserver() override {}
  void OnDatabaseSizeChanged(const std::string& origin_identifier,
                             const base::string16& database_name,
                             int64_t database_size) override {
    if (!observe_size_changes_)
      return;
    new_notification_received_ = true;
    origin_identifier_ = origin_identifier;
    database_name_ = database_name;
    database_size_ = database_size;
  }
  void OnDatabaseScheduledForDeletion(
      const std::string& origin_identifier,
      const base::string16& database_name) override {
    if (!observe_scheduled_deletions_)
      return;
    new_notification_received_ = true;
    origin_identifier_ = origin_identifier;
    database_name_ = database_name;
  }
  bool DidReceiveNewNotification() {
    bool temp_new_notification_received = new_notification_received_;
    new_notification_received_ = false;
    return temp_new_notification_received;
  }
  std::string GetNotificationOriginIdentifier() { return origin_identifier_; }
  base::string16 GetNotificationDatabaseName() { return database_name_; }
  int64_t GetNotificationDatabaseSize() { return database_size_; }

 private:
  bool new_notification_received_;
  bool observe_size_changes_;
  bool observe_scheduled_deletions_;
  std::string origin_identifier_;
  base::string16 database_name_;
  int64_t database_size_;
};

void CheckNotificationReceived(TestObserver* observer,
                               const std::string& expected_origin_identifier,
                               const base::string16& expected_database_name,
                               int64_t expected_database_size) {
  EXPECT_TRUE(observer->DidReceiveNewNotification());
  EXPECT_EQ(expected_origin_identifier,
            observer->GetNotificationOriginIdentifier());
  EXPECT_EQ(expected_database_name, observer->GetNotificationDatabaseName());
  EXPECT_EQ(expected_database_size, observer->GetNotificationDatabaseSize());
}

class TestQuotaManagerProxy : public storage::QuotaManagerProxy {
 public:
  TestQuotaManagerProxy()
      : QuotaManagerProxy(nullptr, nullptr), registered_client_(nullptr) {}

  void RegisterClient(storage::QuotaClient* client) override {
    EXPECT_FALSE(registered_client_);
    registered_client_ = client;
  }

  void NotifyStorageAccessed(storage::QuotaClient::ID client_id,
                             const GURL& origin,
                             storage::StorageType type) override {
    EXPECT_EQ(storage::QuotaClient::kDatabase, client_id);
    EXPECT_EQ(storage::kStorageTypeTemporary, type);
    accesses_[origin] += 1;
  }

  void NotifyStorageModified(storage::QuotaClient::ID client_id,
                             const GURL& origin,
                             storage::StorageType type,
                             int64_t delta) override {
    EXPECT_EQ(storage::QuotaClient::kDatabase, client_id);
    EXPECT_EQ(storage::kStorageTypeTemporary, type);
    modifications_[origin].first += 1;
    modifications_[origin].second += delta;
  }

  // Not needed for our tests.
  void NotifyOriginInUse(const GURL& origin) override {}
  void NotifyOriginNoLongerInUse(const GURL& origin) override {}
  void SetUsageCacheEnabled(storage::QuotaClient::ID client_id,
                            const GURL& origin,
                            storage::StorageType type,
                            bool enabled) override {}
  void GetUsageAndQuota(base::SequencedTaskRunner* original_task_runner,
                        const GURL& origin,
                        storage::StorageType type,
                        const UsageAndQuotaCallback& callback) override {}

  void SimulateQuotaManagerDestroyed() {
    if (registered_client_) {
      registered_client_->OnQuotaManagerDestroyed();
      registered_client_ = nullptr;
    }
  }

  bool WasAccessNotified(const GURL& origin) { return accesses_[origin] != 0; }

  bool WasModificationNotified(const GURL& origin, int64_t amount) {
    return modifications_[origin].first != 0 &&
           modifications_[origin].second == amount;
  }

  void reset() {
    accesses_.clear();
    modifications_.clear();
  }

  storage::QuotaClient* registered_client_;

  // Map from origin to count of access notifications.
  std::map<GURL, int> accesses_;

  // Map from origin to <count, sum of deltas>
  std::map<GURL, std::pair<int, int64_t>> modifications_;

 protected:
  ~TestQuotaManagerProxy() override { EXPECT_FALSE(registered_client_); }
};

bool EnsureFileOfSize(const base::FilePath& file_path, int64_t length) {
  base::File file(file_path,
                  base::File::FLAG_OPEN_ALWAYS | base::File::FLAG_WRITE);
  if (!file.IsValid())
    return false;
  return file.SetLength(length);
}

}  // namespace

namespace content {

// We declare a helper class, and make it a friend of DatabaseTracker using
// the FORWARD_DECLARE_TEST macro, and we implement all tests we want to run as
// static methods of this class. Then we make our TEST() targets call these
// static functions. This allows us to run each test in normal mode and
// incognito mode without writing the same code twice.
class DatabaseTracker_TestHelper_Test {
 public:
  static void TestDeleteOpenDatabase(bool incognito_mode) {
    // Initialize the tracker database.
    base::test::ScopedTaskEnvironment scoped_task_environment;
    base::ScopedTempDir temp_dir;
    ASSERT_TRUE(temp_dir.CreateUniqueTempDir());
    scoped_refptr<MockSpecialStoragePolicy> special_storage_policy =
        new MockSpecialStoragePolicy;
    special_storage_policy->AddProtected(GURL(kOrigin2Url));
    scoped_refptr<DatabaseTracker> tracker(
        base::MakeRefCounted<DatabaseTracker>(
            temp_dir.GetPath(), incognito_mode, special_storage_policy.get(),
            nullptr));
    tracker->set_task_runner_for_testing(
        base::SequencedTaskRunnerHandle::Get());

    // Create and open three databases.
    int64_t database_size = 0;
    const std::string kOrigin1 =
        storage::GetIdentifierFromOrigin(GURL(kOrigin1Url));
    const std::string kOrigin2 =
        storage::GetIdentifierFromOrigin(GURL(kOrigin2Url));
    const base::string16 kDB1 = ASCIIToUTF16("db1");
    const base::string16 kDB2 = ASCIIToUTF16("db2");
    const base::string16 kDB3 = ASCIIToUTF16("db3");
    const base::string16 kDescription = ASCIIToUTF16("database_description");

    tracker->DatabaseOpened(kOrigin1, kDB1, kDescription, 0, &database_size);
    tracker->DatabaseOpened(kOrigin2, kDB2, kDescription, 0, &database_size);
    tracker->DatabaseOpened(kOrigin2, kDB3, kDescription, 0, &database_size);

    EXPECT_TRUE(base::CreateDirectory(
        tracker->DatabaseDirectory().Append(base::FilePath::FromUTF16Unsafe(
            tracker->GetOriginDirectory(kOrigin1)))));
    EXPECT_TRUE(base::CreateDirectory(
        tracker->DatabaseDirectory().Append(base::FilePath::FromUTF16Unsafe(
            tracker->GetOriginDirectory(kOrigin2)))));
    EXPECT_EQ(
        1, base::WriteFile(tracker->GetFullDBFilePath(kOrigin1, kDB1), "a", 1));
    EXPECT_EQ(2, base::WriteFile(tracker->GetFullDBFilePath(kOrigin2, kDB2),
                                 "aa", 2));
    EXPECT_EQ(3, base::WriteFile(tracker->GetFullDBFilePath(kOrigin2, kDB3),
                                 "aaa", 3));
    tracker->DatabaseModified(kOrigin1, kDB1);
    tracker->DatabaseModified(kOrigin2, kDB2);
    tracker->DatabaseModified(kOrigin2, kDB3);

    // Delete db1. Should also delete origin1.
    TestObserver observer;
    tracker->AddObserver(&observer);
    net::TestCompletionCallback callback;
    int result = tracker->DeleteDatabase(kOrigin1, kDB1, callback.callback());
    EXPECT_EQ(net::ERR_IO_PENDING, result);
    ASSERT_FALSE(callback.have_result());
    EXPECT_TRUE(observer.DidReceiveNewNotification());
    EXPECT_EQ(kOrigin1, observer.GetNotificationOriginIdentifier());
    EXPECT_EQ(kDB1, observer.GetNotificationDatabaseName());
    tracker->DatabaseClosed(kOrigin1, kDB1);
    result = callback.GetResult(result);
    EXPECT_EQ(net::OK, result);
    EXPECT_FALSE(
        base::PathExists(tracker->DatabaseDirectory().AppendASCII(kOrigin1)));

    // Recreate db1.
    tracker->DatabaseOpened(kOrigin1, kDB1, kDescription, 0, &database_size);
    EXPECT_TRUE(base::CreateDirectory(
        tracker->DatabaseDirectory().Append(base::FilePath::FromUTF16Unsafe(
            tracker->GetOriginDirectory(kOrigin1)))));
    EXPECT_EQ(
        1, base::WriteFile(tracker->GetFullDBFilePath(kOrigin1, kDB1), "a", 1));
    tracker->DatabaseModified(kOrigin1, kDB1);

    // Setup file modification times.  db1 and db2 are modified now, db3 three
    // days ago.
    base::Time now = base::Time::Now();
    EXPECT_TRUE(
        base::TouchFile(tracker->GetFullDBFilePath(kOrigin1, kDB1), now, now));
    EXPECT_TRUE(
        base::TouchFile(tracker->GetFullDBFilePath(kOrigin2, kDB2), now, now));
    base::Time three_days_ago = now - base::TimeDelta::FromDays(3);
    EXPECT_TRUE(base::TouchFile(tracker->GetFullDBFilePath(kOrigin2, kDB3),
                                three_days_ago, three_days_ago));

    // Delete databases modified since yesterday. db2 is whitelisted.
    base::Time yesterday = base::Time::Now();
    yesterday -= base::TimeDelta::FromDays(1);
    result = tracker->DeleteDataModifiedSince(yesterday, callback.callback());
    EXPECT_EQ(net::ERR_IO_PENDING, result);
    ASSERT_FALSE(callback.have_result());
    EXPECT_TRUE(observer.DidReceiveNewNotification());
    tracker->DatabaseClosed(kOrigin1, kDB1);
    tracker->DatabaseClosed(kOrigin2, kDB2);
    result = callback.GetResult(result);
    EXPECT_EQ(net::OK, result);
    EXPECT_FALSE(
        base::PathExists(tracker->DatabaseDirectory().AppendASCII(kOrigin1)));
    EXPECT_TRUE(base::PathExists(tracker->GetFullDBFilePath(kOrigin2, kDB2)));
    EXPECT_TRUE(base::PathExists(tracker->GetFullDBFilePath(kOrigin2, kDB3)));

    tracker->DatabaseClosed(kOrigin2, kDB3);
    tracker->RemoveObserver(&observer);
  }

  static void TestDatabaseTracker(bool incognito_mode) {
    // Initialize the tracker database.
    base::test::ScopedTaskEnvironment scoped_task_environment;
    base::ScopedTempDir temp_dir;
    ASSERT_TRUE(temp_dir.CreateUniqueTempDir());
    scoped_refptr<MockSpecialStoragePolicy> special_storage_policy =
        new MockSpecialStoragePolicy;
    special_storage_policy->AddProtected(GURL(kOrigin2Url));
    scoped_refptr<DatabaseTracker> tracker(
        base::MakeRefCounted<DatabaseTracker>(
            temp_dir.GetPath(), incognito_mode, special_storage_policy.get(),
            nullptr));
    tracker->set_task_runner_for_testing(
        base::SequencedTaskRunnerHandle::Get());

    // Add two observers.
    TestObserver observer1;
    TestObserver observer2;
    tracker->AddObserver(&observer1);
    tracker->AddObserver(&observer2);

    // Open three new databases.
    int64_t database_size = 0;
    const std::string kOrigin1 =
        storage::GetIdentifierFromOrigin(GURL(kOrigin1Url));
    const std::string kOrigin2 =
        storage::GetIdentifierFromOrigin(GURL(kOrigin2Url));
    const base::string16 kDB1 = ASCIIToUTF16("db1");
    const base::string16 kDB2 = ASCIIToUTF16("db2");
    const base::string16 kDB3 = ASCIIToUTF16("db3");
    const base::string16 kDescription = ASCIIToUTF16("database_description");

    // Get the info for kOrigin1 and kOrigin2
    DatabaseTracker::CachedOriginInfo* origin1_info =
        tracker->GetCachedOriginInfo(kOrigin1);
    DatabaseTracker::CachedOriginInfo* origin2_info =
        tracker->GetCachedOriginInfo(kOrigin1);
    EXPECT_TRUE(origin1_info);
    EXPECT_TRUE(origin2_info);

    tracker->DatabaseOpened(kOrigin1, kDB1, kDescription, 0, &database_size);
    EXPECT_EQ(0, database_size);
    tracker->DatabaseOpened(kOrigin2, kDB2, kDescription, 0, &database_size);
    EXPECT_EQ(0, database_size);
    tracker->DatabaseOpened(kOrigin1, kDB3, kDescription, 0, &database_size);
    EXPECT_EQ(0, database_size);

    // Write some data to each file and check that the listeners are
    // called with the appropriate values.
    EXPECT_TRUE(base::CreateDirectory(
        tracker->DatabaseDirectory().Append(base::FilePath::FromUTF16Unsafe(
            tracker->GetOriginDirectory(kOrigin1)))));
    EXPECT_TRUE(base::CreateDirectory(
        tracker->DatabaseDirectory().Append(base::FilePath::FromUTF16Unsafe(
            tracker->GetOriginDirectory(kOrigin2)))));
    EXPECT_EQ(
        1, base::WriteFile(tracker->GetFullDBFilePath(kOrigin1, kDB1), "a", 1));
    EXPECT_EQ(2, base::WriteFile(tracker->GetFullDBFilePath(kOrigin2, kDB2),
                                 "aa", 2));
    EXPECT_EQ(4, base::WriteFile(tracker->GetFullDBFilePath(kOrigin1, kDB3),
                                 "aaaa", 4));
    tracker->DatabaseModified(kOrigin1, kDB1);
    CheckNotificationReceived(&observer1, kOrigin1, kDB1, 1);
    CheckNotificationReceived(&observer2, kOrigin1, kDB1, 1);
    tracker->DatabaseModified(kOrigin2, kDB2);
    CheckNotificationReceived(&observer1, kOrigin2, kDB2, 2);
    CheckNotificationReceived(&observer2, kOrigin2, kDB2, 2);
    tracker->DatabaseModified(kOrigin1, kDB3);
    CheckNotificationReceived(&observer1, kOrigin1, kDB3, 4);
    CheckNotificationReceived(&observer2, kOrigin1, kDB3, 4);

    // Close all databases
    tracker->DatabaseClosed(kOrigin1, kDB1);
    tracker->DatabaseClosed(kOrigin2, kDB2);
    tracker->DatabaseClosed(kOrigin1, kDB3);

    // Open an existing database and check the reported size
    tracker->DatabaseOpened(kOrigin1, kDB1, kDescription, 0, &database_size);
    EXPECT_EQ(1, database_size);
    tracker->DatabaseClosed(kOrigin1, kDB1);

    // Remove an observer; this should clear all caches.
    tracker->RemoveObserver(&observer2);

    // Close the tracker database and clear all caches.
    // Then make sure that DatabaseOpened() still returns the correct result.
    tracker->CloseTrackerDatabaseAndClearCaches();
    tracker->DatabaseOpened(kOrigin1, kDB1, kDescription, 0, &database_size);
    EXPECT_EQ(1, database_size);
    tracker->DatabaseClosed(kOrigin1, kDB1);

    // Remove all observers.
    tracker->RemoveObserver(&observer1);

    // Trying to delete a database in use should fail
    tracker->DatabaseOpened(kOrigin1, kDB3, kDescription, 0, &database_size);
    EXPECT_FALSE(tracker->DeleteClosedDatabase(kOrigin1, kDB3));
    origin1_info = tracker->GetCachedOriginInfo(kOrigin1);
    EXPECT_TRUE(origin1_info);
    EXPECT_EQ(4, origin1_info->GetDatabaseSize(kDB3));
    tracker->DatabaseClosed(kOrigin1, kDB3);

    // Delete a database and make sure the space used by that origin is updated
    EXPECT_TRUE(tracker->DeleteClosedDatabase(kOrigin1, kDB3));
    origin1_info = tracker->GetCachedOriginInfo(kOrigin1);
    EXPECT_TRUE(origin1_info);
    EXPECT_EQ(1, origin1_info->GetDatabaseSize(kDB1));
    EXPECT_EQ(0, origin1_info->GetDatabaseSize(kDB3));

    // Get all data for all origins
    std::vector<OriginInfo> origins_info;
    EXPECT_TRUE(tracker->GetAllOriginsInfo(&origins_info));
    EXPECT_EQ(size_t(2), origins_info.size());
    EXPECT_EQ(kOrigin1, origins_info[0].GetOriginIdentifier());
    EXPECT_EQ(1, origins_info[0].TotalSize());
    EXPECT_EQ(1, origins_info[0].GetDatabaseSize(kDB1));
    EXPECT_EQ(0, origins_info[0].GetDatabaseSize(kDB3));

    EXPECT_EQ(kOrigin2, origins_info[1].GetOriginIdentifier());
    EXPECT_EQ(2, origins_info[1].TotalSize());

    // Trying to delete an origin with databases in use should fail
    tracker->DatabaseOpened(kOrigin1, kDB1, kDescription, 0, &database_size);
    EXPECT_FALSE(tracker->DeleteOrigin(kOrigin1, false));
    origin1_info = tracker->GetCachedOriginInfo(kOrigin1);
    EXPECT_TRUE(origin1_info);
    EXPECT_EQ(1, origin1_info->GetDatabaseSize(kDB1));
    tracker->DatabaseClosed(kOrigin1, kDB1);

    // Delete an origin that doesn't have any database in use
    EXPECT_TRUE(tracker->DeleteOrigin(kOrigin1, false));
    origins_info.clear();
    EXPECT_TRUE(tracker->GetAllOriginsInfo(&origins_info));
    EXPECT_EQ(size_t(1), origins_info.size());
    EXPECT_EQ(kOrigin2, origins_info[0].GetOriginIdentifier());

    origin1_info = tracker->GetCachedOriginInfo(kOrigin1);
    EXPECT_TRUE(origin1_info);
    EXPECT_EQ(0, origin1_info->TotalSize());
  }

  static void DatabaseTrackerQuotaIntegration() {
    const GURL kOrigin(kOrigin1Url);
    const std::string kOriginId = storage::GetIdentifierFromOrigin(kOrigin);
    const base::string16 kName = ASCIIToUTF16("name");
    const base::string16 kDescription = ASCIIToUTF16("description");

    base::test::ScopedTaskEnvironment scoped_task_environment;
    base::ScopedTempDir temp_dir;
    ASSERT_TRUE(temp_dir.CreateUniqueTempDir());

    // Initialize the tracker with a QuotaManagerProxy
    scoped_refptr<TestQuotaManagerProxy> test_quota_proxy(
        new TestQuotaManagerProxy);
    scoped_refptr<DatabaseTracker> tracker(
        base::MakeRefCounted<DatabaseTracker>(temp_dir.GetPath(),
                                              false /* incognito */, nullptr,
                                              test_quota_proxy.get()));
    tracker->set_task_runner_for_testing(
        base::SequencedTaskRunnerHandle::Get());

    EXPECT_TRUE(test_quota_proxy->registered_client_);

    // Create a database and modify it a couple of times, close it,
    // then delete it. Observe the tracker notifies accordingly.

    int64_t database_size = 0;
    tracker->DatabaseOpened(kOriginId, kName, kDescription, 0, &database_size);
    EXPECT_TRUE(test_quota_proxy->WasAccessNotified(kOrigin));
    test_quota_proxy->reset();

    base::FilePath db_file(tracker->GetFullDBFilePath(kOriginId, kName));
    EXPECT_TRUE(base::CreateDirectory(db_file.DirName()));
    EXPECT_TRUE(EnsureFileOfSize(db_file, 10));
    tracker->DatabaseModified(kOriginId, kName);
    EXPECT_TRUE(test_quota_proxy->WasModificationNotified(kOrigin, 10));
    test_quota_proxy->reset();

    EXPECT_TRUE(EnsureFileOfSize(db_file, 100));
    tracker->DatabaseModified(kOriginId, kName);
    EXPECT_TRUE(test_quota_proxy->WasModificationNotified(kOrigin, 90));
    test_quota_proxy->reset();

    tracker->DatabaseClosed(kOriginId, kName);
    EXPECT_TRUE(test_quota_proxy->WasAccessNotified(kOrigin));
    EXPECT_EQ(net::OK, tracker->DeleteDatabase(kOriginId, kName,
                                               net::CompletionCallback()));
    EXPECT_TRUE(test_quota_proxy->WasModificationNotified(kOrigin, -100));
    test_quota_proxy->reset();

    // Create a database and modify it, try to delete it while open,
    // then close it (at which time deletion will actually occur).
    // Observe the tracker notifies accordingly.

    tracker->DatabaseOpened(kOriginId, kName, kDescription, 0, &database_size);
    EXPECT_TRUE(test_quota_proxy->WasAccessNotified(kOrigin));
    test_quota_proxy->reset();

    db_file = tracker->GetFullDBFilePath(kOriginId, kName);
    EXPECT_TRUE(base::CreateDirectory(db_file.DirName()));
    EXPECT_TRUE(EnsureFileOfSize(db_file, 100));
    tracker->DatabaseModified(kOriginId, kName);
    EXPECT_TRUE(test_quota_proxy->WasModificationNotified(kOrigin, 100));
    test_quota_proxy->reset();

    EXPECT_EQ(
        net::ERR_IO_PENDING,
        tracker->DeleteDatabase(kOriginId, kName, net::CompletionCallback()));
    EXPECT_FALSE(test_quota_proxy->WasModificationNotified(kOrigin, -100));

    tracker->DatabaseClosed(kOriginId, kName);
    EXPECT_TRUE(test_quota_proxy->WasAccessNotified(kOrigin));
    EXPECT_TRUE(test_quota_proxy->WasModificationNotified(kOrigin, -100));
    test_quota_proxy->reset();

    // Create a database and up the file size without telling
    // the tracker about the modification, than simulate a
    // a renderer crash.
    // Observe the tracker notifies accordingly.

    tracker->DatabaseOpened(kOriginId, kName, kDescription, 0, &database_size);
    EXPECT_TRUE(test_quota_proxy->WasAccessNotified(kOrigin));
    test_quota_proxy->reset();
    db_file = tracker->GetFullDBFilePath(kOriginId, kName);
    EXPECT_TRUE(base::CreateDirectory(db_file.DirName()));
    EXPECT_TRUE(EnsureFileOfSize(db_file, 100));
    DatabaseConnections crashed_renderer_connections;
    crashed_renderer_connections.AddConnection(kOriginId, kName);
    EXPECT_FALSE(test_quota_proxy->WasModificationNotified(kOrigin, 100));
    tracker->CloseDatabases(crashed_renderer_connections);
    EXPECT_TRUE(test_quota_proxy->WasModificationNotified(kOrigin, 100));

    // Cleanup.
    crashed_renderer_connections.RemoveAllConnections();
    test_quota_proxy->SimulateQuotaManagerDestroyed();
  }

  static void DatabaseTrackerClearSessionOnlyDatabasesOnExit() {
    int64_t database_size = 0;
    const std::string kOrigin1 =
        storage::GetIdentifierFromOrigin(GURL(kOrigin1Url));
    const std::string kOrigin2 =
        storage::GetIdentifierFromOrigin(GURL(kOrigin2Url));
    const base::string16 kDB1 = ASCIIToUTF16("db1");
    const base::string16 kDB2 = ASCIIToUTF16("db2");
    const base::string16 kDescription = ASCIIToUTF16("database_description");

    // Initialize the tracker database.
    base::test::ScopedTaskEnvironment scoped_task_environment;
    base::ScopedTempDir temp_dir;
    ASSERT_TRUE(temp_dir.CreateUniqueTempDir());
    base::FilePath origin1_db_dir;
    base::FilePath origin2_db_dir;
    {
      scoped_refptr<MockSpecialStoragePolicy> special_storage_policy =
          new MockSpecialStoragePolicy;
      special_storage_policy->AddSessionOnly(GURL(kOrigin2Url));
      scoped_refptr<DatabaseTracker> tracker(
          base::MakeRefCounted<DatabaseTracker>(temp_dir.GetPath(), false,
                                                special_storage_policy.get(),
                                                nullptr));
      tracker->set_task_runner_for_testing(
          base::SequencedTaskRunnerHandle::Get());

      // Open two new databases.
      tracker->DatabaseOpened(kOrigin1, kDB1, kDescription, 0, &database_size);
      EXPECT_EQ(0, database_size);
      tracker->DatabaseOpened(kOrigin2, kDB2, kDescription, 0, &database_size);
      EXPECT_EQ(0, database_size);

      // Write some data to each file.
      base::FilePath db_file;
      db_file = tracker->GetFullDBFilePath(kOrigin1, kDB1);
      EXPECT_TRUE(base::CreateDirectory(db_file.DirName()));
      EXPECT_TRUE(EnsureFileOfSize(db_file, 1));

      db_file = tracker->GetFullDBFilePath(kOrigin2, kDB2);
      EXPECT_TRUE(base::CreateDirectory(db_file.DirName()));
      EXPECT_TRUE(EnsureFileOfSize(db_file, 2));

      // Store the origin database directories as long as they still exist.
      origin1_db_dir = tracker->GetFullDBFilePath(kOrigin1, kDB1).DirName();
      origin2_db_dir = tracker->GetFullDBFilePath(kOrigin2, kDB2).DirName();

      tracker->DatabaseModified(kOrigin1, kDB1);
      tracker->DatabaseModified(kOrigin2, kDB2);

      // Close all databases.
      tracker->DatabaseClosed(kOrigin1, kDB1);
      tracker->DatabaseClosed(kOrigin2, kDB2);

      tracker->Shutdown();
    }

    // At this point, the database tracker should be gone. Create a new one.
    scoped_refptr<DatabaseTracker> tracker(
        base::MakeRefCounted<DatabaseTracker>(temp_dir.GetPath(), false,
                                              nullptr, nullptr));
    tracker->set_task_runner_for_testing(
        base::SequencedTaskRunnerHandle::Get());

    // Get all data for all origins.
    std::vector<OriginInfo> origins_info;
    EXPECT_TRUE(tracker->GetAllOriginsInfo(&origins_info));
    // kOrigin1 was not session-only, so it survived. kOrigin2 was session-only
    // and it got deleted.
    EXPECT_EQ(size_t(1), origins_info.size());
    EXPECT_EQ(kOrigin1, origins_info[0].GetOriginIdentifier());
    EXPECT_TRUE(base::PathExists(tracker->GetFullDBFilePath(kOrigin1, kDB1)));
    EXPECT_EQ(base::FilePath(), tracker->GetFullDBFilePath(kOrigin2, kDB2));

    // The origin directory of kOrigin1 remains, but the origin directory of
    // kOrigin2 is deleted.
    EXPECT_TRUE(base::PathExists(origin1_db_dir));
    EXPECT_FALSE(base::PathExists(origin2_db_dir));
  }

  static void DatabaseTrackerSetForceKeepSessionState() {
    int64_t database_size = 0;
    const std::string kOrigin1 =
        storage::GetIdentifierFromOrigin(GURL(kOrigin1Url));
    const std::string kOrigin2 =
        storage::GetIdentifierFromOrigin(GURL(kOrigin2Url));
    const base::string16 kDB1 = ASCIIToUTF16("db1");
    const base::string16 kDB2 = ASCIIToUTF16("db2");
    const base::string16 kDescription = ASCIIToUTF16("database_description");

    // Initialize the tracker database.
    base::test::ScopedTaskEnvironment scoped_task_environment;
    base::ScopedTempDir temp_dir;
    ASSERT_TRUE(temp_dir.CreateUniqueTempDir());
    base::FilePath origin1_db_dir;
    base::FilePath origin2_db_dir;
    {
      scoped_refptr<MockSpecialStoragePolicy> special_storage_policy =
          new MockSpecialStoragePolicy;
      special_storage_policy->AddSessionOnly(GURL(kOrigin2Url));
      scoped_refptr<DatabaseTracker> tracker(
          base::MakeRefCounted<DatabaseTracker>(temp_dir.GetPath(), false,
                                                special_storage_policy.get(),
                                                nullptr));
      tracker->set_task_runner_for_testing(
          base::SequencedTaskRunnerHandle::Get());
      tracker->SetForceKeepSessionState();

      // Open two new databases.
      tracker->DatabaseOpened(kOrigin1, kDB1, kDescription, 0, &database_size);
      EXPECT_EQ(0, database_size);
      tracker->DatabaseOpened(kOrigin2, kDB2, kDescription, 0, &database_size);
      EXPECT_EQ(0, database_size);

      // Write some data to each file.
      base::FilePath db_file;
      db_file = tracker->GetFullDBFilePath(kOrigin1, kDB1);
      EXPECT_TRUE(base::CreateDirectory(db_file.DirName()));
      EXPECT_TRUE(EnsureFileOfSize(db_file, 1));

      db_file = tracker->GetFullDBFilePath(kOrigin2, kDB2);
      EXPECT_TRUE(base::CreateDirectory(db_file.DirName()));
      EXPECT_TRUE(EnsureFileOfSize(db_file, 2));

      // Store the origin database directories as long as they still exist.
      origin1_db_dir = tracker->GetFullDBFilePath(kOrigin1, kDB1).DirName();
      origin2_db_dir = tracker->GetFullDBFilePath(kOrigin2, kDB2).DirName();

      tracker->DatabaseModified(kOrigin1, kDB1);
      tracker->DatabaseModified(kOrigin2, kDB2);

      // Close all databases.
      tracker->DatabaseClosed(kOrigin1, kDB1);
      tracker->DatabaseClosed(kOrigin2, kDB2);

      tracker->Shutdown();
    }

    // At this point, the database tracker should be gone. Create a new one.
    scoped_refptr<DatabaseTracker> tracker(
        base::MakeRefCounted<DatabaseTracker>(temp_dir.GetPath(), false,
                                              nullptr, nullptr));
    tracker->set_task_runner_for_testing(
        base::SequencedTaskRunnerHandle::Get());

    // Get all data for all origins.
    std::vector<OriginInfo> origins_info;
    EXPECT_TRUE(tracker->GetAllOriginsInfo(&origins_info));
    // No origins were deleted.
    EXPECT_EQ(size_t(2), origins_info.size());
    EXPECT_TRUE(base::PathExists(tracker->GetFullDBFilePath(kOrigin1, kDB1)));
    EXPECT_TRUE(base::PathExists(tracker->GetFullDBFilePath(kOrigin2, kDB2)));

    EXPECT_TRUE(base::PathExists(origin1_db_dir));
    EXPECT_TRUE(base::PathExists(origin2_db_dir));
  }

  static void EmptyDatabaseNameIsValid() {
    const GURL kOrigin(kOrigin1Url);
    const std::string kOriginId = storage::GetIdentifierFromOrigin(kOrigin);
    const base::string16 kEmptyName;
    const base::string16 kDescription(ASCIIToUTF16("description"));
    const base::string16 kChangedDescription(
        ASCIIToUTF16("changed_description"));

    // Initialize a tracker database, no need to put it on disk.
    const bool kUseInMemoryTrackerDatabase = true;
    base::test::ScopedTaskEnvironment scoped_task_environment;
    base::ScopedTempDir temp_dir;
    ASSERT_TRUE(temp_dir.CreateUniqueTempDir());
    scoped_refptr<DatabaseTracker> tracker(
        base::MakeRefCounted<DatabaseTracker>(
            temp_dir.GetPath(), kUseInMemoryTrackerDatabase, nullptr, nullptr));
    tracker->set_task_runner_for_testing(
        base::SequencedTaskRunnerHandle::Get());

    // Starts off with no databases.
    std::vector<OriginInfo> infos;
    EXPECT_TRUE(tracker->GetAllOriginsInfo(&infos));
    EXPECT_TRUE(infos.empty());

    // Create a db with an empty name.
    int64_t database_size = -1;
    tracker->DatabaseOpened(kOriginId, kEmptyName, kDescription, 0,
                            &database_size);
    EXPECT_EQ(0, database_size);
    tracker->DatabaseModified(kOriginId, kEmptyName);
    EXPECT_TRUE(tracker->GetAllOriginsInfo(&infos));
    EXPECT_EQ(1u, infos.size());
    EXPECT_EQ(kDescription, infos[0].GetDatabaseDescription(kEmptyName));
    EXPECT_FALSE(tracker->GetFullDBFilePath(kOriginId, kEmptyName).empty());
    tracker->DatabaseOpened(kOriginId, kEmptyName, kChangedDescription, 0,
                            &database_size);
    infos.clear();
    EXPECT_TRUE(tracker->GetAllOriginsInfo(&infos));
    EXPECT_EQ(1u, infos.size());
    EXPECT_EQ(kChangedDescription, infos[0].GetDatabaseDescription(kEmptyName));
    tracker->DatabaseClosed(kOriginId, kEmptyName);
    tracker->DatabaseClosed(kOriginId, kEmptyName);

    // Deleting it should return to the initial state.
    EXPECT_EQ(net::OK, tracker->DeleteDatabase(kOriginId, kEmptyName,
                                               net::CompletionCallback()));
    infos.clear();
    EXPECT_TRUE(tracker->GetAllOriginsInfo(&infos));
    EXPECT_TRUE(infos.empty());
  }

  static void HandleSqliteError() {
    const GURL kOrigin(kOrigin1Url);
    const std::string kOriginId = storage::GetIdentifierFromOrigin(kOrigin);
    const base::string16 kName(ASCIIToUTF16("name"));
    const base::string16 kDescription(ASCIIToUTF16("description"));

    // Initialize a tracker database, no need to put it on disk.
    const bool kUseInMemoryTrackerDatabase = true;
    base::test::ScopedTaskEnvironment scoped_task_environment;
    base::ScopedTempDir temp_dir;
    ASSERT_TRUE(temp_dir.CreateUniqueTempDir());
    scoped_refptr<DatabaseTracker> tracker(
        base::MakeRefCounted<DatabaseTracker>(
            temp_dir.GetPath(), kUseInMemoryTrackerDatabase, nullptr, nullptr));
    tracker->set_task_runner_for_testing(
        base::SequencedTaskRunnerHandle::Get());

    // Setup to observe OnScheduledForDelete notifications.
    TestObserver observer(false, true);
    tracker->AddObserver(&observer);

    // Verify does no harm when there is no such database.
    tracker->HandleSqliteError(kOriginId, kName, SQLITE_CORRUPT);
    EXPECT_FALSE(tracker->IsDatabaseScheduledForDeletion(kOriginId, kName));
    EXPECT_FALSE(observer.DidReceiveNewNotification());

    // --------------------------------------------------------
    // Create a record of a database in the tracker db and create
    // a spoof_db_file on disk in the expected location.
    int64_t database_size = 0;
    tracker->DatabaseOpened(kOriginId, kName, kDescription, 0, &database_size);
    base::FilePath spoof_db_file = tracker->GetFullDBFilePath(kOriginId, kName);
    EXPECT_FALSE(tracker->GetFullDBFilePath(kOriginId, kName).empty());
    EXPECT_TRUE(base::CreateDirectory(spoof_db_file.DirName()));
    EXPECT_TRUE(EnsureFileOfSize(spoof_db_file, 1));

    // Verify does no harm with a non-error is reported.
    tracker->HandleSqliteError(kOriginId, kName, SQLITE_OK);
    EXPECT_FALSE(tracker->IsDatabaseScheduledForDeletion(kOriginId, kName));
    EXPECT_FALSE(observer.DidReceiveNewNotification());

    // Verify that with a connection open, the db is scheduled for deletion,
    // but that the file still exists.
    tracker->HandleSqliteError(kOriginId, kName, SQLITE_CORRUPT);
    EXPECT_TRUE(tracker->IsDatabaseScheduledForDeletion(kOriginId, kName));
    EXPECT_TRUE(observer.DidReceiveNewNotification());
    EXPECT_TRUE(base::PathExists(spoof_db_file));

    // Verify that once closed, the file is deleted and the record in the
    // tracker db is removed.
    tracker->DatabaseClosed(kOriginId, kName);
    EXPECT_FALSE(base::PathExists(spoof_db_file));
    EXPECT_TRUE(tracker->GetFullDBFilePath(kOriginId, kName).empty());

    // --------------------------------------------------------
    // Create another record of a database in the tracker db and create
    // a spoof_db_file on disk in the expected location.
    tracker->DatabaseOpened(kOriginId, kName, kDescription, 0, &database_size);
    base::FilePath spoof_db_file2 =
        tracker->GetFullDBFilePath(kOriginId, kName);
    EXPECT_FALSE(tracker->GetFullDBFilePath(kOriginId, kName).empty());
    EXPECT_NE(spoof_db_file, spoof_db_file2);
    EXPECT_TRUE(base::CreateDirectory(spoof_db_file2.DirName()));
    EXPECT_TRUE(EnsureFileOfSize(spoof_db_file2, 1));

    // Verify that with no connection open, the db is deleted immediately.
    tracker->DatabaseClosed(kOriginId, kName);
    tracker->HandleSqliteError(kOriginId, kName, SQLITE_CORRUPT);
    EXPECT_FALSE(tracker->IsDatabaseScheduledForDeletion(kOriginId, kName));
    EXPECT_FALSE(observer.DidReceiveNewNotification());
    EXPECT_TRUE(tracker->GetFullDBFilePath(kOriginId, kName).empty());
    EXPECT_FALSE(base::PathExists(spoof_db_file2));

    tracker->RemoveObserver(&observer);
  }
};

TEST(DatabaseTrackerTest, DeleteOpenDatabase) {
  DatabaseTracker_TestHelper_Test::TestDeleteOpenDatabase(false);
}

TEST(DatabaseTrackerTest, DeleteOpenDatabaseIncognitoMode) {
  DatabaseTracker_TestHelper_Test::TestDeleteOpenDatabase(true);
}

TEST(DatabaseTrackerTest, DatabaseTracker) {
  DatabaseTracker_TestHelper_Test::TestDatabaseTracker(false);
}

TEST(DatabaseTrackerTest, DatabaseTrackerIncognitoMode) {
  DatabaseTracker_TestHelper_Test::TestDatabaseTracker(true);
}

TEST(DatabaseTrackerTest, DatabaseTrackerQuotaIntegration) {
  // There is no difference in behavior between incognito and not.
  DatabaseTracker_TestHelper_Test::DatabaseTrackerQuotaIntegration();
}

TEST(DatabaseTrackerTest, DatabaseTrackerClearSessionOnlyDatabasesOnExit) {
  // Only works for regular mode.
  DatabaseTracker_TestHelper_Test::
      DatabaseTrackerClearSessionOnlyDatabasesOnExit();
}

TEST(DatabaseTrackerTest, DatabaseTrackerSetForceKeepSessionState) {
  // Only works for regular mode.
  DatabaseTracker_TestHelper_Test::DatabaseTrackerSetForceKeepSessionState();
}

TEST(DatabaseTrackerTest, EmptyDatabaseNameIsValid) {
  DatabaseTracker_TestHelper_Test::EmptyDatabaseNameIsValid();
}

TEST(DatabaseTrackerTest, HandleSqliteError) {
  DatabaseTracker_TestHelper_Test::HandleSqliteError();
}

}  // namespace content
