// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <set>

#include "base/bind.h"
#include "base/files/file_util.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "chrome/browser/devtools/devtools_file_system_indexer.h"
#include "chrome/common/chrome_paths.h"
#include "content/public/test/test_browser_thread_bundle.h"
#include "testing/gtest/include/gtest/gtest.h"

class DevToolsFileSystemIndexerTest : public testing::Test {
 public:
  void SetDone() {
    indexing_done_ = true;
    base::RunLoop::QuitCurrentWhenIdleDeprecated();
  }

  void SearchCallback(const std::vector<std::string>& results) {
    search_results_.clear();
    for (const std::string& result : results) {
      search_results_.insert(
          base::FilePath::FromUTF8Unsafe(result).BaseName().AsUTF8Unsafe());
    }
    base::RunLoop::QuitCurrentWhenIdleDeprecated();
  }

 protected:
  void SetUp() override {
    indexer_ = new DevToolsFileSystemIndexer();
    indexing_done_ = false;
  }

  content::TestBrowserThreadBundle thread_bundle_;
  scoped_refptr<DevToolsFileSystemIndexer> indexer_;
  std::set<std::string> search_results_;
  bool indexing_done_;
};

TEST_F(DevToolsFileSystemIndexerTest, BasicUsage) {
  base::FilePath base_test_path;
  PathService::Get(chrome::DIR_TEST_DATA, &base_test_path);
  base::FilePath index_path =
      base_test_path.Append(FILE_PATH_LITERAL("devtools"))
          .Append(FILE_PATH_LITERAL("indexer"));

  scoped_refptr<DevToolsFileSystemIndexer::FileSystemIndexingJob> job =
      indexer_->IndexPath(index_path.AsUTF8Unsafe(), base::Bind([](int) {}),
                          base::Bind([](int) {}),
                          base::Bind(&DevToolsFileSystemIndexerTest::SetDone,
                                     base::Unretained(this)));

  base::RunLoop().Run();
  ASSERT_TRUE(indexing_done_);

  indexer_->SearchInPath(
      index_path.AsUTF8Unsafe(), "Hello",
      base::Bind(&DevToolsFileSystemIndexerTest::SearchCallback,
                 base::Unretained(this)));
  base::RunLoop().Run();

  ASSERT_EQ(3lu, search_results_.size());
  ASSERT_EQ(1lu, search_results_.count("hello_world.c"));
  ASSERT_EQ(1lu, search_results_.count("hello_world.html"));
  ASSERT_EQ(1lu, search_results_.count("hello_world.js"));

  indexer_->SearchInPath(
      index_path.AsUTF8Unsafe(), "FUNCTION",
      base::Bind(&DevToolsFileSystemIndexerTest::SearchCallback,
                 base::Unretained(this)));
  base::RunLoop().Run();

  ASSERT_EQ(1lu, search_results_.size());
  ASSERT_EQ(1lu, search_results_.count("hello_world.js"));
}
