// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/utility/safe_browsing/mac/hfs.h"

#include <stddef.h>
#include <stdint.h>

#include "base/files/file.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/utility/safe_browsing/mac/dmg_test_utils.h"
#include "chrome/utility/safe_browsing/mac/read_stream.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace safe_browsing {
namespace dmg {
namespace {

class HFSIteratorTest : public testing::Test {
 public:
  void GetTargetFiles(bool case_sensitive,
                      std::set<base::string16>* files,
                      std::set<base::string16>* dirs) {
    const char* kBaseFiles[] = {
      "first/second/third/fourth/fifth/random",
      "first/second/third/fourth/Hello World",
      "first/second/third/symlink-random",
      "first/second/goat-output.txt",
      "first/unicode_name",
      "README.txt",
      ".metadata_never_index",
    };

    const char* kBaseDirs[] = {
      "first/second/third/fourth/fifth",
      "first/second/third/fourth",
      "first/second/third",
      "first/second",
      "first",
      ".Trashes",
    };

    const base::string16 dmg_name = base::ASCIIToUTF16("SafeBrowsingDMG/");

    for (size_t i = 0; i < arraysize(kBaseFiles); ++i)
      files->insert(dmg_name + base::ASCIIToUTF16(kBaseFiles[i]));

    files->insert(dmg_name + base::ASCIIToUTF16("first/second/") +
                  base::UTF8ToUTF16("Te\xCC\x86st\xCC\x88 \xF0\x9F\x90\x90 "));

    dirs->insert(dmg_name.substr(0, dmg_name.size() - 1));
    for (size_t i = 0; i < arraysize(kBaseDirs); ++i)
      dirs->insert(dmg_name + base::ASCIIToUTF16(kBaseDirs[i]));

    if (case_sensitive) {
      files->insert(base::ASCIIToUTF16(
          "SafeBrowsingDMG/first/second/third/fourth/hEllo wOrld"));
    }
  }

  void TestTargetFiles(safe_browsing::dmg::HFSIterator* hfs_reader,
                       bool case_sensitive) {
    std::set<base::string16> files, dirs;
    GetTargetFiles(case_sensitive, &files, &dirs);

    ASSERT_TRUE(hfs_reader->Open());
    while (hfs_reader->Next()) {
      base::string16 path = hfs_reader->GetPath();
      // Skip over .fseventsd files.
      if (path.find(base::ASCIIToUTF16("SafeBrowsingDMG/.fseventsd")) !=
              base::string16::npos) {
        continue;
      }
      if (hfs_reader->IsDirectory())
        EXPECT_TRUE(dirs.erase(path)) << path;
      else
        EXPECT_TRUE(files.erase(path)) << path;
    }

    EXPECT_EQ(0u, files.size());
    for (const auto& file : files) {
      ADD_FAILURE() << "Unexpected missing file " << file;
    }
  }
};

TEST_F(HFSIteratorTest, HFSPlus) {
  base::File file;
  ASSERT_NO_FATAL_FAILURE(test::GetTestFile("hfs_plus.img", &file));

  FileReadStream stream(file.GetPlatformFile());
  HFSIterator hfs_reader(&stream);
  TestTargetFiles(&hfs_reader, false);
}

TEST_F(HFSIteratorTest, HFSXCaseSensitive) {
  base::File file;
  ASSERT_NO_FATAL_FAILURE(test::GetTestFile("hfsx_case_sensitive.img", &file));

  FileReadStream stream(file.GetPlatformFile());
  HFSIterator hfs_reader(&stream);
  TestTargetFiles(&hfs_reader, true);
}

class HFSFileReadTest : public testing::TestWithParam<const char*> {
 protected:
  void SetUp() override {
    ASSERT_NO_FATAL_FAILURE(test::GetTestFile(GetParam(), &hfs_file_));

    hfs_stream_.reset(new FileReadStream(hfs_file_.GetPlatformFile()));
    hfs_reader_.reset(new HFSIterator(hfs_stream_.get()));
    ASSERT_TRUE(hfs_reader_->Open());
  }

  bool GoToFile(const char* name) {
    while (hfs_reader_->Next()) {
      if (EndsWith(hfs_reader_->GetPath(), base::ASCIIToUTF16(name),
                   base::CompareCase::SENSITIVE)) {
        return true;
      }
    }
    return false;
  }

  HFSIterator* hfs_reader() { return hfs_reader_.get(); }

 private:
  base::File hfs_file_;
  std::unique_ptr<FileReadStream> hfs_stream_;
  std::unique_ptr<HFSIterator> hfs_reader_;
};

TEST_P(HFSFileReadTest, ReadReadme) {
  ASSERT_TRUE(GoToFile("README.txt"));

  std::unique_ptr<ReadStream> stream = hfs_reader()->GetReadStream();
  ASSERT_TRUE(stream.get());

  EXPECT_FALSE(hfs_reader()->IsSymbolicLink());
  EXPECT_FALSE(hfs_reader()->IsHardLink());
  EXPECT_FALSE(hfs_reader()->IsDecmpfsCompressed());

  std::vector<uint8_t> buffer(4, 0);

  // Read the first four bytes.
  EXPECT_TRUE(stream->ReadExact(&buffer[0], buffer.size()));
  const uint8_t expected[] = { 'T', 'h', 'i', 's' };
  EXPECT_EQ(0, memcmp(expected, &buffer[0], sizeof(expected)));
  buffer.clear();

  // Rewind back to the start.
  EXPECT_EQ(0, stream->Seek(0, SEEK_SET));

  // Read the entire file now.
  EXPECT_TRUE(test::ReadEntireStream(stream.get(), &buffer));
  EXPECT_EQ("This is a test HFS+ filesystem generated by "
            "chrome/test/data/safe_browsing/dmg/make_hfs.sh.\n",
            base::StringPiece(reinterpret_cast<const char*>(&buffer[0]),
                              buffer.size()));
  EXPECT_EQ(92u, buffer.size());
}

TEST_P(HFSFileReadTest, ReadRandom) {
  ASSERT_TRUE(GoToFile("fifth/random"));

  std::unique_ptr<ReadStream> stream = hfs_reader()->GetReadStream();
  ASSERT_TRUE(stream.get());

  EXPECT_FALSE(hfs_reader()->IsSymbolicLink());
  EXPECT_FALSE(hfs_reader()->IsHardLink());
  EXPECT_FALSE(hfs_reader()->IsDecmpfsCompressed());

  std::vector<uint8_t> buffer;
  EXPECT_TRUE(test::ReadEntireStream(stream.get(), &buffer));
  EXPECT_EQ(768u, buffer.size());
}

TEST_P(HFSFileReadTest, Symlink) {
  ASSERT_TRUE(GoToFile("symlink-random"));

  std::unique_ptr<ReadStream> stream = hfs_reader()->GetReadStream();
  ASSERT_TRUE(stream.get());

  EXPECT_TRUE(hfs_reader()->IsSymbolicLink());
  EXPECT_FALSE(hfs_reader()->IsHardLink());
  EXPECT_FALSE(hfs_reader()->IsDecmpfsCompressed());

  std::vector<uint8_t> buffer;
  EXPECT_TRUE(test::ReadEntireStream(stream.get(), &buffer));

  EXPECT_EQ("fourth/fifth/random",
            base::StringPiece(reinterpret_cast<const char*>(&buffer[0]),
                              buffer.size()));
}

TEST_P(HFSFileReadTest, HardLink) {
  ASSERT_TRUE(GoToFile("unicode_name"));

  EXPECT_FALSE(hfs_reader()->IsSymbolicLink());
  EXPECT_TRUE(hfs_reader()->IsHardLink());
  EXPECT_FALSE(hfs_reader()->IsDecmpfsCompressed());
}

TEST_P(HFSFileReadTest, DecmpfsFile) {
  ASSERT_TRUE(GoToFile("first/second/goat-output.txt"));

  std::unique_ptr<ReadStream> stream = hfs_reader()->GetReadStream();
  ASSERT_TRUE(stream.get());

  EXPECT_FALSE(hfs_reader()->IsSymbolicLink());
  EXPECT_FALSE(hfs_reader()->IsHardLink());
  EXPECT_TRUE(hfs_reader()->IsDecmpfsCompressed());

  std::vector<uint8_t> buffer;
  EXPECT_TRUE(test::ReadEntireStream(stream.get(), &buffer));
  EXPECT_EQ(0u, buffer.size());
}

INSTANTIATE_TEST_CASE_P(HFSIteratorTest,
                        HFSFileReadTest,
                        testing::Values(
                            "hfs_plus.img",
                            "hfsx_case_sensitive.img"));

}  // namespace
}  // namespace dmg
}  // namespace safe_browsing
