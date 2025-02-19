// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>

#include <string>

#include "base/logging.h"
#include "base/macros.h"
#include "storage/browser/fileapi/file_system_url.h"
#include "storage/browser/fileapi/isolated_context.h"
#include "testing/gtest/include/gtest/gtest.h"

#define FPL(x) FILE_PATH_LITERAL(x)

#if defined(FILE_PATH_USES_DRIVE_LETTERS)
#define DRIVE FPL("C:")
#else
#define DRIVE
#endif

using storage::FileSystemMountOption;
using storage::FileSystemURL;
using storage::IsolatedContext;
using storage::kFileSystemTypeDragged;
using storage::kFileSystemTypeIsolated;
using storage::kFileSystemTypeNativeLocal;

namespace content {

typedef IsolatedContext::MountPointInfo FileInfo;

namespace {

const base::FilePath kTestPaths[] = {
  base::FilePath(DRIVE FPL("/a/b.txt")),
  base::FilePath(DRIVE FPL("/c/d/e")),
  base::FilePath(DRIVE FPL("/h/")),
  base::FilePath(DRIVE FPL("/")),
#if defined(FILE_PATH_USES_WIN_SEPARATORS)
  base::FilePath(DRIVE FPL("\\foo\\bar")),
  base::FilePath(DRIVE FPL("\\")),
#endif
  // For duplicated base name test.
  base::FilePath(DRIVE FPL("/")),
  base::FilePath(DRIVE FPL("/f/e")),
  base::FilePath(DRIVE FPL("/f/b.txt")),
};

}  // namespace

class IsolatedContextTest : public testing::Test {
 public:
  IsolatedContextTest() {
    for (size_t i = 0; i < arraysize(kTestPaths); ++i)
      fileset_.insert(kTestPaths[i].NormalizePathSeparators());
  }

  void SetUp() override {
    IsolatedContext::FileInfoSet files;
    for (size_t i = 0; i < arraysize(kTestPaths); ++i) {
      std::string name;
      ASSERT_TRUE(
          files.AddPath(kTestPaths[i].NormalizePathSeparators(), &name));
      names_.push_back(name);
    }
    id_ = IsolatedContext::GetInstance()->RegisterDraggedFileSystem(files);
    IsolatedContext::GetInstance()->AddReference(id_);
    ASSERT_FALSE(id_.empty());
  }

  void TearDown() override {
    IsolatedContext::GetInstance()->RemoveReference(id_);
  }

  IsolatedContext* isolated_context() const {
    return IsolatedContext::GetInstance();
  }

 protected:
  std::string id_;
  std::multiset<base::FilePath> fileset_;
  std::vector<std::string> names_;

 private:
  DISALLOW_COPY_AND_ASSIGN(IsolatedContextTest);
};

TEST_F(IsolatedContextTest, RegisterAndRevokeTest) {
  // See if the returned top-level entries match with what we registered.
  std::vector<FileInfo> toplevels;
  ASSERT_TRUE(isolated_context()->GetDraggedFileInfo(id_, &toplevels));
  ASSERT_EQ(fileset_.size(), toplevels.size());
  for (size_t i = 0; i < toplevels.size(); ++i) {
    ASSERT_TRUE(fileset_.find(toplevels[i].path) != fileset_.end());
  }

  // See if the name of each registered kTestPaths (that is what we
  // register in SetUp() by RegisterDraggedFileSystem) is properly cracked as
  // a valid virtual path in the isolated filesystem.
  for (size_t i = 0; i < arraysize(kTestPaths); ++i) {
    base::FilePath virtual_path = isolated_context()->CreateVirtualRootPath(id_)
        .AppendASCII(names_[i]);
    std::string cracked_id;
    base::FilePath cracked_path;
    std::string cracked_inner_id;
    storage::FileSystemType cracked_type;
    FileSystemMountOption cracked_option;
    ASSERT_TRUE(isolated_context()->CrackVirtualPath(
        virtual_path, &cracked_id, &cracked_type, &cracked_inner_id,
        &cracked_path, &cracked_option));
    ASSERT_EQ(kTestPaths[i].NormalizePathSeparators().value(),
              cracked_path.value());
    ASSERT_EQ(id_, cracked_id);
    ASSERT_EQ(kFileSystemTypeDragged, cracked_type);
    EXPECT_TRUE(cracked_inner_id.empty());
  }

  // Make sure GetRegisteredPath returns false for id_ since it is
  // registered for dragged files.
  base::FilePath path;
  ASSERT_FALSE(isolated_context()->GetRegisteredPath(id_, &path));

  // Deref the current one and registering a new one.
  isolated_context()->RemoveReference(id_);

  std::string id2 = isolated_context()->RegisterFileSystemForPath(
      kFileSystemTypeNativeLocal, std::string(),
      base::FilePath(DRIVE FPL("/foo")), NULL);

  // Make sure the GetDraggedFileInfo returns false for both ones.
  ASSERT_FALSE(isolated_context()->GetDraggedFileInfo(id2, &toplevels));
  ASSERT_FALSE(isolated_context()->GetDraggedFileInfo(id_, &toplevels));

  // Make sure the GetRegisteredPath returns true only for the new one.
  ASSERT_FALSE(isolated_context()->GetRegisteredPath(id_, &path));
  ASSERT_TRUE(isolated_context()->GetRegisteredPath(id2, &path));

  // Try registering three more file systems for the same path as id2.
  std::string id3 = isolated_context()->RegisterFileSystemForPath(
      kFileSystemTypeNativeLocal, std::string(), path, NULL);
  std::string id4 = isolated_context()->RegisterFileSystemForPath(
      kFileSystemTypeNativeLocal, std::string(), path, NULL);
  std::string id5 = isolated_context()->RegisterFileSystemForPath(
      kFileSystemTypeNativeLocal, std::string(), path, NULL);

  // Remove file system for id4.
  isolated_context()->AddReference(id4);
  isolated_context()->RemoveReference(id4);

  // Only id4 should become invalid now.
  ASSERT_TRUE(isolated_context()->GetRegisteredPath(id2, &path));
  ASSERT_TRUE(isolated_context()->GetRegisteredPath(id3, &path));
  ASSERT_FALSE(isolated_context()->GetRegisteredPath(id4, &path));
  ASSERT_TRUE(isolated_context()->GetRegisteredPath(id5, &path));

  // Revoke file system id5, after adding multiple references.
  isolated_context()->AddReference(id5);
  isolated_context()->AddReference(id5);
  isolated_context()->AddReference(id5);
  isolated_context()->RevokeFileSystem(id5);

  // No matter how many references we add id5 must be invalid now.
  ASSERT_TRUE(isolated_context()->GetRegisteredPath(id2, &path));
  ASSERT_TRUE(isolated_context()->GetRegisteredPath(id3, &path));
  ASSERT_FALSE(isolated_context()->GetRegisteredPath(id4, &path));
  ASSERT_FALSE(isolated_context()->GetRegisteredPath(id5, &path));

  // Revoke the file systems by path.
  isolated_context()->RevokeFileSystemByPath(path);

  // Now all the file systems associated to the path must be invalid.
  ASSERT_FALSE(isolated_context()->GetRegisteredPath(id2, &path));
  ASSERT_FALSE(isolated_context()->GetRegisteredPath(id3, &path));
  ASSERT_FALSE(isolated_context()->GetRegisteredPath(id4, &path));
  ASSERT_FALSE(isolated_context()->GetRegisteredPath(id5, &path));
}

TEST_F(IsolatedContextTest, CrackWithRelativePaths) {
  const struct {
    base::FilePath::StringType path;
    bool valid;
  } relatives[] = {
    { FPL("foo"), true },
    { FPL("foo/bar"), true },
    { FPL(".."), false },
    { FPL("foo/.."), false },
    { FPL("foo/../bar"), false },
#if defined(FILE_PATH_USES_WIN_SEPARATORS)
# define SHOULD_FAIL_WITH_WIN_SEPARATORS false
#else
# define SHOULD_FAIL_WITH_WIN_SEPARATORS true
#endif
    { FPL("foo\\..\\baz"), SHOULD_FAIL_WITH_WIN_SEPARATORS },
    { FPL("foo/..\\baz"), SHOULD_FAIL_WITH_WIN_SEPARATORS },
  };

  for (size_t i = 0; i < arraysize(kTestPaths); ++i) {
    for (size_t j = 0; j < arraysize(relatives); ++j) {
      SCOPED_TRACE(testing::Message() << "Testing "
                   << kTestPaths[i].value() << " " << relatives[j].path);
      base::FilePath virtual_path =
          isolated_context()->CreateVirtualRootPath(id_).AppendASCII(
              names_[i]).Append(relatives[j].path);
      std::string cracked_id;
      base::FilePath cracked_path;
      storage::FileSystemType cracked_type;
      std::string cracked_inner_id;
      FileSystemMountOption cracked_option;
      if (!relatives[j].valid) {
        ASSERT_FALSE(isolated_context()->CrackVirtualPath(
            virtual_path, &cracked_id, &cracked_type, &cracked_inner_id,
            &cracked_path, &cracked_option));
        continue;
      }
      ASSERT_TRUE(isolated_context()->CrackVirtualPath(
          virtual_path, &cracked_id, &cracked_type, &cracked_inner_id,
          &cracked_path, &cracked_option));
      ASSERT_EQ(kTestPaths[i].Append(relatives[j].path)
                    .NormalizePathSeparators().value(),
                cracked_path.value());
      ASSERT_EQ(id_, cracked_id);
      ASSERT_EQ(kFileSystemTypeDragged, cracked_type);
      EXPECT_TRUE(cracked_inner_id.empty());
    }
  }
}

TEST_F(IsolatedContextTest, CrackURLWithRelativePaths) {
  const struct {
    base::FilePath::StringType path;
    bool valid;
  } relatives[] = {
    { FPL("foo"), true },
    { FPL("foo/bar"), true },
    { FPL(".."), false },
    { FPL("foo/.."), false },
    { FPL("foo/../bar"), false },
#if defined(FILE_PATH_USES_WIN_SEPARATORS)
# define SHOULD_FAIL_WITH_WIN_SEPARATORS false
#else
# define SHOULD_FAIL_WITH_WIN_SEPARATORS true
#endif
    { FPL("foo\\..\\baz"), SHOULD_FAIL_WITH_WIN_SEPARATORS },
    { FPL("foo/..\\baz"), SHOULD_FAIL_WITH_WIN_SEPARATORS },
  };

  for (size_t i = 0; i < arraysize(kTestPaths); ++i) {
    for (size_t j = 0; j < arraysize(relatives); ++j) {
      SCOPED_TRACE(testing::Message() << "Testing "
                   << kTestPaths[i].value() << " " << relatives[j].path);
      base::FilePath virtual_path =
          isolated_context()->CreateVirtualRootPath(id_).AppendASCII(
              names_[i]).Append(relatives[j].path);

      FileSystemURL cracked = isolated_context()->CreateCrackedFileSystemURL(
          GURL("http://chromium.org"), kFileSystemTypeIsolated, virtual_path);

      ASSERT_EQ(relatives[j].valid, cracked.is_valid());

      if (!relatives[j].valid)
        continue;
      ASSERT_EQ(GURL("http://chromium.org"), cracked.origin());
      ASSERT_EQ(kTestPaths[i].Append(relatives[j].path)
                    .NormalizePathSeparators().value(),
                cracked.path().value());
      ASSERT_EQ(virtual_path.NormalizePathSeparators(), cracked.virtual_path());
      ASSERT_EQ(id_, cracked.filesystem_id());
      ASSERT_EQ(kFileSystemTypeDragged, cracked.type());
      ASSERT_EQ(kFileSystemTypeIsolated, cracked.mount_type());
    }
  }
}

TEST_F(IsolatedContextTest, TestWithVirtualRoot) {
  std::string cracked_id;
  base::FilePath cracked_path;
  FileSystemMountOption cracked_option;

  // Trying to crack virtual root "/" returns true but with empty cracked path
  // as "/" of the isolated filesystem is a pure virtual directory
  // that has no corresponding platform directory.
  base::FilePath virtual_path = isolated_context()->CreateVirtualRootPath(id_);
  ASSERT_TRUE(isolated_context()->CrackVirtualPath(
      virtual_path, &cracked_id, NULL, NULL, &cracked_path, &cracked_option));
  ASSERT_EQ(FPL(""), cracked_path.value());
  ASSERT_EQ(id_, cracked_id);

  // Trying to crack "/foo" should fail (because "foo" is not the one
  // included in the kTestPaths).
  virtual_path = isolated_context()->CreateVirtualRootPath(
      id_).AppendASCII("foo");
  ASSERT_FALSE(isolated_context()->CrackVirtualPath(
      virtual_path, &cracked_id, NULL, NULL, &cracked_path, &cracked_option));
}

TEST_F(IsolatedContextTest, CanHandleURL) {
  const GURL test_origin("http://chromium.org");
  const base::FilePath test_path(FPL("/mount"));

  // Should handle isolated file system.
  EXPECT_TRUE(isolated_context()->HandlesFileSystemMountType(
      storage::kFileSystemTypeIsolated));

  // Shouldn't handle the rest.
  EXPECT_FALSE(isolated_context()->HandlesFileSystemMountType(
      storage::kFileSystemTypeExternal));
  EXPECT_FALSE(isolated_context()->HandlesFileSystemMountType(
      storage::kFileSystemTypeTemporary));
  EXPECT_FALSE(isolated_context()->HandlesFileSystemMountType(
      storage::kFileSystemTypePersistent));
  EXPECT_FALSE(isolated_context()->HandlesFileSystemMountType(
      storage::kFileSystemTypeTest));
  // Not even if it's isolated subtype.
  EXPECT_FALSE(isolated_context()->HandlesFileSystemMountType(
      storage::kFileSystemTypeNativeLocal));
  EXPECT_FALSE(isolated_context()->HandlesFileSystemMountType(
      storage::kFileSystemTypeDragged));
  EXPECT_FALSE(isolated_context()->HandlesFileSystemMountType(
      storage::kFileSystemTypeNativeMedia));
  EXPECT_FALSE(isolated_context()->HandlesFileSystemMountType(
      storage::kFileSystemTypeDeviceMedia));
}

TEST_F(IsolatedContextTest, VirtualFileSystemTests) {
  // Should be able to register empty and non-absolute paths
  std::string empty_fsid = isolated_context()->RegisterFileSystemForVirtualPath(
      storage::kFileSystemTypeIsolated, "_", base::FilePath());
  std::string relative_fsid =
      isolated_context()->RegisterFileSystemForVirtualPath(
          storage::kFileSystemTypeIsolated,
          "_",
          base::FilePath(FPL("relpath")));
  ASSERT_FALSE(empty_fsid.empty());
  ASSERT_FALSE(relative_fsid.empty());

  // Make sure that filesystem root is not prepended to cracked virtual paths.
  base::FilePath database_root = base::FilePath(DRIVE FPL("/database_path"));
  std::string database_fsid =
      isolated_context()->RegisterFileSystemForVirtualPath(
          storage::kFileSystemTypeIsolated, "_", database_root);

  base::FilePath test_virtual_path =
      base::FilePath().AppendASCII("virtualdir").AppendASCII("virtualfile.txt");

  base::FilePath whole_virtual_path =
      isolated_context()->CreateVirtualRootPath(database_fsid)
          .AppendASCII("_").Append(test_virtual_path);

  std::string cracked_id;
  base::FilePath cracked_path;
  std::string cracked_inner_id;
  FileSystemMountOption cracked_option;
  ASSERT_TRUE(isolated_context()->CrackVirtualPath(
      whole_virtual_path, &cracked_id, NULL, &cracked_inner_id,
      &cracked_path, &cracked_option));
  ASSERT_EQ(database_fsid, cracked_id);
  ASSERT_EQ(test_virtual_path, cracked_path);
  EXPECT_TRUE(cracked_inner_id.empty());
}

}  // namespace content
