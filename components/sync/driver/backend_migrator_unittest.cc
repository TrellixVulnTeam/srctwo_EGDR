// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/sync/driver/backend_migrator.h"

#include <memory>

#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "base/tracked_objects.h"
#include "components/sync/base/model_type_test_util.h"
#include "components/sync/driver/data_type_manager_mock.h"
#include "components/sync/driver/fake_sync_service.h"
#include "components/sync/protocol/sync.pb.h"
#include "components/sync/syncable/directory.h"
#include "components/sync/syncable/test_user_share.h"
#include "components/sync/syncable/write_transaction.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using ::testing::_;
using ::testing::Eq;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::Return;

namespace syncer {

class SyncBackendMigratorTest : public testing::Test {
 public:
  SyncBackendMigratorTest() {}
  virtual ~SyncBackendMigratorTest() {}

  virtual void SetUp() {
    test_user_share_.SetUp();
    Mock::VerifyAndClear(manager());
    preferred_types_.Put(BOOKMARKS);
    preferred_types_.Put(PREFERENCES);
    preferred_types_.Put(AUTOFILL);

    migrator_ = std::make_unique<BackendMigrator>(
        "Profile0", test_user_share_.user_share(), service(), manager(),
        base::Closure());
    SetUnsyncedTypes(ModelTypeSet());
  }

  virtual void TearDown() {
    migrator_.reset();
    test_user_share_.TearDown();
  }

  // Marks all types in |unsynced_types| as unsynced  and all other
  // types as synced.
  void SetUnsyncedTypes(ModelTypeSet unsynced_types) {
    WriteTransaction trans(FROM_HERE, test_user_share_.user_share());
    for (int i = FIRST_REAL_MODEL_TYPE; i < MODEL_TYPE_COUNT; ++i) {
      ModelType type = ModelTypeFromInt(i);
      sync_pb::DataTypeProgressMarker progress_marker;
      if (!unsynced_types.Has(type)) {
        progress_marker.set_token("dummy");
      }
      trans.GetDirectory()->SetDownloadProgress(type, progress_marker);
    }
  }

  void SendConfigureDone(DataTypeManager::ConfigureStatus status,
                         ModelTypeSet requested_types) {
    if (status == DataTypeManager::OK) {
      DataTypeManager::ConfigureResult result(status, requested_types);
      migrator_->OnConfigureDone(result);
    } else {
      DataTypeManager::ConfigureResult result(status, requested_types);
      migrator_->OnConfigureDone(result);
    }
    base::RunLoop run_loop;
    run_loop.RunUntilIdle();
  }

  SyncService* service() { return &service_; }
  DataTypeManagerMock* manager() { return &manager_; }
  ModelTypeSet preferred_types() { return preferred_types_; }
  BackendMigrator* migrator() { return migrator_.get(); }
  void RemovePreferredType(ModelType type) { preferred_types_.Remove(type); }

 private:
  base::MessageLoop message_loop_;
  ModelTypeSet preferred_types_;
  FakeSyncService service_;
  NiceMock<DataTypeManagerMock> manager_;
  TestUserShare test_user_share_;
  std::unique_ptr<BackendMigrator> migrator_;
};

class MockMigrationObserver : public MigrationObserver {
 public:
  virtual ~MockMigrationObserver() {}

  MOCK_METHOD0(OnMigrationStateChange, void());
};

// Test that in the normal case a migration does transition through each state
// and wind up back in IDLE.
TEST_F(SyncBackendMigratorTest, Sanity) {
  MockMigrationObserver migration_observer;
  migrator()->AddMigrationObserver(&migration_observer);
  EXPECT_CALL(migration_observer, OnMigrationStateChange()).Times(4);

  ModelTypeSet to_migrate, difference;
  to_migrate.Put(PREFERENCES);
  difference.Put(AUTOFILL);
  difference.Put(BOOKMARKS);

  EXPECT_CALL(*manager(), state())
      .WillOnce(Return(DataTypeManager::CONFIGURED));
  EXPECT_CALL(*manager(), PurgeForMigration(_, CONFIGURE_REASON_MIGRATION))
      .Times(1);
  EXPECT_CALL(*manager(), Configure(_, CONFIGURE_REASON_MIGRATION)).Times(1);

  migrator()->MigrateTypes(to_migrate);
  EXPECT_EQ(BackendMigrator::DISABLING_TYPES, migrator()->state());

  SetUnsyncedTypes(to_migrate);
  SendConfigureDone(DataTypeManager::OK, difference);
  EXPECT_EQ(BackendMigrator::REENABLING_TYPES, migrator()->state());

  SetUnsyncedTypes(ModelTypeSet());
  SendConfigureDone(DataTypeManager::OK, preferred_types());
  EXPECT_EQ(BackendMigrator::IDLE, migrator()->state());

  migrator()->RemoveMigrationObserver(&migration_observer);
}

// Test that in the normal case with Nigori a migration transitions through
// each state and wind up back in IDLE.
TEST_F(SyncBackendMigratorTest, MigrateNigori) {
  ModelTypeSet to_migrate, difference;
  to_migrate.Put(NIGORI);
  difference.Put(AUTOFILL);
  difference.Put(BOOKMARKS);

  EXPECT_CALL(*manager(), state())
      .WillOnce(Return(DataTypeManager::CONFIGURED));

  EXPECT_CALL(*manager(), PurgeForMigration(_, CONFIGURE_REASON_MIGRATION));

  migrator()->MigrateTypes(to_migrate);
  EXPECT_EQ(BackendMigrator::DISABLING_TYPES, migrator()->state());

  SetUnsyncedTypes(to_migrate);
  SendConfigureDone(DataTypeManager::OK, difference);
  EXPECT_EQ(BackendMigrator::REENABLING_TYPES, migrator()->state());

  SetUnsyncedTypes(ModelTypeSet());
  SendConfigureDone(DataTypeManager::OK, preferred_types());
  EXPECT_EQ(BackendMigrator::IDLE, migrator()->state());
}

// Test that the migrator waits for the data type manager to be idle before
// starting a migration.
TEST_F(SyncBackendMigratorTest, WaitToStart) {
  ModelTypeSet to_migrate;
  to_migrate.Put(PREFERENCES);

  EXPECT_CALL(*manager(), state())
      .WillOnce(Return(DataTypeManager::CONFIGURING));
  EXPECT_CALL(*manager(), Configure(_, _)).Times(0);
  migrator()->MigrateTypes(to_migrate);
  EXPECT_EQ(BackendMigrator::WAITING_TO_START, migrator()->state());

  Mock::VerifyAndClearExpectations(manager());
  EXPECT_CALL(*manager(), state())
      .WillOnce(Return(DataTypeManager::CONFIGURED));
  EXPECT_CALL(*manager(), PurgeForMigration(_, CONFIGURE_REASON_MIGRATION));
  SetUnsyncedTypes(ModelTypeSet());
  SendConfigureDone(DataTypeManager::OK, ModelTypeSet());

  EXPECT_EQ(BackendMigrator::DISABLING_TYPES, migrator()->state());
}

// Test that the migrator can cope with a migration request while a migration
// is in progress.
TEST_F(SyncBackendMigratorTest, RestartMigration) {
  ModelTypeSet to_migrate1, to_migrate2, to_migrate_union, bookmarks;
  to_migrate1.Put(PREFERENCES);
  to_migrate2.Put(AUTOFILL);
  to_migrate_union.Put(PREFERENCES);
  to_migrate_union.Put(AUTOFILL);
  bookmarks.Put(BOOKMARKS);

  EXPECT_CALL(*manager(), state())
      .WillOnce(Return(DataTypeManager::CONFIGURED));
  EXPECT_CALL(*manager(), PurgeForMigration(_, CONFIGURE_REASON_MIGRATION))
      .Times(2);
  migrator()->MigrateTypes(to_migrate1);

  EXPECT_EQ(BackendMigrator::DISABLING_TYPES, migrator()->state());
  migrator()->MigrateTypes(to_migrate2);

  const ModelTypeSet difference1 = Difference(preferred_types(), to_migrate1);

  Mock::VerifyAndClearExpectations(manager());
  EXPECT_CALL(*manager(), PurgeForMigration(_, CONFIGURE_REASON_MIGRATION))
      .Times(1);
  EXPECT_CALL(*manager(), Configure(_, CONFIGURE_REASON_MIGRATION)).Times(1);
  SetUnsyncedTypes(to_migrate1);
  SendConfigureDone(DataTypeManager::OK, difference1);
  EXPECT_EQ(BackendMigrator::DISABLING_TYPES, migrator()->state());

  SetUnsyncedTypes(to_migrate_union);
  SendConfigureDone(DataTypeManager::OK, bookmarks);
  EXPECT_EQ(BackendMigrator::REENABLING_TYPES, migrator()->state());
}

// Test that an external invocation of Configure(...) during a migration results
// in a migration reattempt.
TEST_F(SyncBackendMigratorTest, InterruptedWhileDisablingTypes) {
  ModelTypeSet to_migrate;
  ModelTypeSet difference;
  to_migrate.Put(PREFERENCES);
  difference.Put(AUTOFILL);
  difference.Put(BOOKMARKS);

  EXPECT_CALL(*manager(), state())
      .WillOnce(Return(DataTypeManager::CONFIGURED));
  EXPECT_CALL(*manager(), PurgeForMigration(HasModelTypes(to_migrate),
                                            CONFIGURE_REASON_MIGRATION));
  migrator()->MigrateTypes(to_migrate);
  EXPECT_EQ(BackendMigrator::DISABLING_TYPES, migrator()->state());

  Mock::VerifyAndClearExpectations(manager());
  EXPECT_CALL(*manager(), PurgeForMigration(HasModelTypes(to_migrate),
                                            CONFIGURE_REASON_MIGRATION));
  SetUnsyncedTypes(ModelTypeSet());
  SendConfigureDone(DataTypeManager::OK, preferred_types());

  EXPECT_EQ(BackendMigrator::DISABLING_TYPES, migrator()->state());
}

// Test that spurious OnConfigureDone events don't confuse the
// migrator while it's waiting for disabled types to have been purged
// from the sync db.
TEST_F(SyncBackendMigratorTest, WaitingForPurge) {
  ModelTypeSet to_migrate, difference;
  to_migrate.Put(PREFERENCES);
  to_migrate.Put(AUTOFILL);
  difference.Put(BOOKMARKS);

  EXPECT_CALL(*manager(), state())
      .WillOnce(Return(DataTypeManager::CONFIGURED));
  EXPECT_CALL(*manager(), PurgeForMigration(_, CONFIGURE_REASON_MIGRATION))
      .Times(1);
  EXPECT_CALL(*manager(), Configure(_, CONFIGURE_REASON_MIGRATION)).Times(1);

  migrator()->MigrateTypes(to_migrate);
  EXPECT_EQ(BackendMigrator::DISABLING_TYPES, migrator()->state());

  SendConfigureDone(DataTypeManager::OK, difference);
  EXPECT_EQ(BackendMigrator::DISABLING_TYPES, migrator()->state());

  ModelTypeSet prefs;
  prefs.Put(PREFERENCES);
  SetUnsyncedTypes(prefs);
  SendConfigureDone(DataTypeManager::OK, difference);
  EXPECT_EQ(BackendMigrator::DISABLING_TYPES, migrator()->state());

  SetUnsyncedTypes(to_migrate);
  SendConfigureDone(DataTypeManager::OK, difference);
  EXPECT_EQ(BackendMigrator::REENABLING_TYPES, migrator()->state());
}

TEST_F(SyncBackendMigratorTest, MigratedTypeDisabledByUserDuringMigration) {
  ModelTypeSet to_migrate;
  to_migrate.Put(PREFERENCES);

  EXPECT_CALL(*manager(), state())
      .WillOnce(Return(DataTypeManager::CONFIGURED));
  EXPECT_CALL(*manager(), PurgeForMigration(_, CONFIGURE_REASON_MIGRATION))
      .Times(1);
  EXPECT_CALL(*manager(), Configure(_, CONFIGURE_REASON_MIGRATION)).Times(1);
  migrator()->MigrateTypes(to_migrate);

  RemovePreferredType(PREFERENCES);
  SetUnsyncedTypes(to_migrate);
  SendConfigureDone(DataTypeManager::OK, preferred_types());
  EXPECT_EQ(BackendMigrator::REENABLING_TYPES, migrator()->state());
  SetUnsyncedTypes(ModelTypeSet());
  SendConfigureDone(DataTypeManager::OK, preferred_types());
  EXPECT_EQ(BackendMigrator::IDLE, migrator()->state());
}

TEST_F(SyncBackendMigratorTest, ConfigureFailure) {
  ModelTypeSet to_migrate;
  to_migrate.Put(PREFERENCES);

  EXPECT_CALL(*manager(), state())
      .WillOnce(Return(DataTypeManager::CONFIGURED));
  EXPECT_CALL(*manager(), PurgeForMigration(_, CONFIGURE_REASON_MIGRATION))
      .Times(1);
  migrator()->MigrateTypes(to_migrate);
  SetUnsyncedTypes(ModelTypeSet());
  SendConfigureDone(DataTypeManager::ABORTED, ModelTypeSet());
  EXPECT_EQ(BackendMigrator::IDLE, migrator()->state());
}

};  // namespace syncer
