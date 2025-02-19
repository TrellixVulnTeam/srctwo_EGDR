// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/browser/value_store/value_store_unittest.h"

#include <stddef.h>

#include "base/files/file_enumerator.h"
#include "base/files/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/memory/ref_counted.h"
#include "base/message_loop/message_loop.h"
#include "base/values.h"
#include "content/public/test/test_browser_thread_bundle.h"
#include "extensions/browser/value_store/leveldb_value_store.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/leveldatabase/src/include/leveldb/db.h"
#include "third_party/leveldatabase/src/include/leveldb/write_batch.h"

namespace {

const char kDatabaseUMAClientName[] = "Test";

ValueStore* Param(const base::FilePath& file_path) {
  return new LeveldbValueStore(kDatabaseUMAClientName, file_path);
}

}  // namespace

INSTANTIATE_TEST_CASE_P(
    LeveldbValueStore,
    ValueStoreTest,
    testing::Values(&Param));

class LeveldbValueStoreUnitTest : public testing::Test {
 public:
  LeveldbValueStoreUnitTest() {}
  ~LeveldbValueStoreUnitTest() override {}

 protected:
  void SetUp() override {
    ASSERT_TRUE(database_dir_.CreateUniqueTempDir());
    CreateStore();
    ASSERT_TRUE(store_->Get()->status().ok());
  }

  void TearDown() override {
    if (!store_)
      return;
    store_->Clear();
    CloseStore();
  }

  void CloseStore() { store_.reset(); }

  void CreateStore() {
    store_.reset(
        new LeveldbValueStore(kDatabaseUMAClientName, database_path()));
  }

  LeveldbValueStore* store() { return store_.get(); }
  const base::FilePath& database_path() { return database_dir_.GetPath(); }

 private:
  std::unique_ptr<LeveldbValueStore> store_;
  base::ScopedTempDir database_dir_;

  content::TestBrowserThreadBundle thread_bundle_;
};

// Check that we can restore a single corrupted key in the LeveldbValueStore.
TEST_F(LeveldbValueStoreUnitTest, RestoreKeyTest) {
  const char kNotCorruptKey[] = "not-corrupt";
  const char kValue[] = "value";

  // Insert a valid pair.
  std::unique_ptr<base::Value> value(new base::Value(kValue));
  ASSERT_TRUE(store()
                  ->Set(ValueStore::DEFAULTS, kNotCorruptKey, *value)
                  ->status().ok());

  // Insert a corrupt pair.
  const char kCorruptKey[] = "corrupt";
  leveldb::WriteBatch batch;
  batch.Put(kCorruptKey, "[{(.*+\"\'\\");
  ASSERT_TRUE(store()->WriteToDbForTest(&batch));

  // Verify corruption (the first Get will return corruption).
  ValueStore::ReadResult result = store()->Get(kCorruptKey);
  ASSERT_FALSE(result->status().ok());
  ASSERT_EQ(ValueStore::CORRUPTION, result->status().code);

  // Verify restored (was deleted in the first Get).
  result = store()->Get(kCorruptKey);
  EXPECT_TRUE(result->status().ok()) << "Get result not OK: "
                                     << result->status().message;
  EXPECT_TRUE(result->settings().empty());

  // Verify that the valid pair is still present.
  result = store()->Get(kNotCorruptKey);
  EXPECT_TRUE(result->status().ok());
  EXPECT_TRUE(result->settings().HasKey(kNotCorruptKey));
  std::string value_string;
  EXPECT_TRUE(result->settings().GetString(kNotCorruptKey, &value_string));
  EXPECT_EQ(kValue, value_string);
}

// Test that the Restore() method does not just delete the entire database
// (unless absolutely necessary), and instead only removes corrupted keys.
TEST_F(LeveldbValueStoreUnitTest, RestoreDoesMinimumNecessary) {
  const char* kNotCorruptKeys[] = {"a", "n", "z"};
  const size_t kNotCorruptKeysSize = 3u;
  const char kCorruptKey1[] = "f";
  const char kCorruptKey2[] = "s";
  const char kValue[] = "value";
  const char kCorruptValue[] = "[{(.*+\"\'\\";

  // Insert a collection of non-corrupted pairs.
  std::unique_ptr<base::Value> value(new base::Value(kValue));
  for (size_t i = 0; i < kNotCorruptKeysSize; ++i) {
    ASSERT_TRUE(store()
                    ->Set(ValueStore::DEFAULTS, kNotCorruptKeys[i], *value)
                    ->status().ok());
  }

  // Insert a few corrupted pairs.
  leveldb::WriteBatch batch;
  batch.Put(kCorruptKey1, kCorruptValue);
  batch.Put(kCorruptKey2, kCorruptValue);
  ASSERT_TRUE(store()->WriteToDbForTest(&batch));

  // Verify that we broke it and that it was repaired by the value store.
  ValueStore::ReadResult result = store()->Get();
  ASSERT_FALSE(result->status().ok());
  ASSERT_EQ(ValueStore::CORRUPTION, result->status().code);
  ASSERT_EQ(ValueStore::VALUE_RESTORE_DELETE_SUCCESS,
            result->status().restore_status);

  // We should still have all valid pairs present in the database.
  std::string value_string;
  for (size_t i = 0; i < kNotCorruptKeysSize; ++i) {
    result = store()->Get(kNotCorruptKeys[i]);
    EXPECT_TRUE(result->status().ok());
    ASSERT_EQ(ValueStore::RESTORE_NONE, result->status().restore_status);
    EXPECT_TRUE(result->settings().HasKey(kNotCorruptKeys[i]));
    EXPECT_TRUE(
        result->settings().GetString(kNotCorruptKeys[i], &value_string));
    EXPECT_EQ(kValue, value_string);
  }
}

// Test that the LeveldbValueStore can recover in the case of a CATastrophic
// failure and we have total corruption. In this case, the database is plagued
// by LolCats.
// Full corruption has been known to happen occasionally in strange edge cases,
// such as after users use Windows Restore. We can't prevent it, but we need to
// be able to handle it smoothly.
TEST_F(LeveldbValueStoreUnitTest, RestoreFullDatabase) {
  const std::string kLolCats("I can haz leveldb filez?");
  const char* kNotCorruptKeys[] = {"a", "n", "z"};
  const size_t kNotCorruptKeysSize = 3u;
  const char kValue[] = "value";

  // Generate a database.
  std::unique_ptr<base::Value> value(new base::Value(kValue));
  for (size_t i = 0; i < kNotCorruptKeysSize; ++i) {
    ASSERT_TRUE(store()
                    ->Set(ValueStore::DEFAULTS, kNotCorruptKeys[i], *value)
                    ->status().ok());
  }

  // Close it (so we remove the lock), and replace all files with LolCats.
  CloseStore();
  base::FileEnumerator enumerator(
      database_path(), true /* recursive */, base::FileEnumerator::FILES);
  for (base::FilePath file = enumerator.Next(); !file.empty();
       file = enumerator.Next()) {
    // WriteFile() failure is a result of -1.
    ASSERT_NE(base::WriteFile(file, kLolCats.c_str(), kLolCats.length()), -1);
  }
  CreateStore();

  // We couldn't recover anything, but we should be in a sane state again.
  ValueStore::ReadResult result = store()->Get();
  ASSERT_EQ(ValueStore::DB_RESTORE_REPAIR_SUCCESS,
            result->status().restore_status);
  EXPECT_TRUE(result->status().ok());
  EXPECT_EQ(0u, result->settings().size());
}
