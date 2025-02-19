// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include <algorithm>
#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/files/memory_mapped_file.h"
#include "base/optional.h"
#include "base/path_service.h"
#include "chrome/installer/zucchini/buffer_view.h"
#include "chrome/installer/zucchini/patch_reader.h"
#include "chrome/installer/zucchini/patch_writer.h"
#include "chrome/installer/zucchini/zucchini.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace zucchini {

base::FilePath MakeTestPath(const std::string& filename) {
  base::FilePath path;
  DCHECK(PathService::Get(base::DIR_SOURCE_ROOT, &path));
  return path.AppendASCII("chrome")
      .AppendASCII("installer")
      .AppendASCII("zucchini")
      .AppendASCII("testdata")
      .AppendASCII(filename);
}

void TestGenApply(const std::string& old_filename,
                  const std::string& new_filename,
                  bool raw) {
  base::FilePath old_path = MakeTestPath(old_filename);
  base::FilePath new_path = MakeTestPath(new_filename);

  base::MemoryMappedFile old_file;
  ASSERT_TRUE(old_file.Initialize(old_path));

  base::MemoryMappedFile new_file;
  ASSERT_TRUE(new_file.Initialize(new_path));

  ConstBufferView old_region(old_file.data(), old_file.length());
  ConstBufferView new_region(new_file.data(), new_file.length());

  EnsemblePatchWriter patch_writer(old_region, new_region);

  // Generate patch from "old" to "new".
  ASSERT_EQ(status::kStatusSuccess,
            raw ? GenerateRaw(old_region, new_region, &patch_writer)
                : GenerateEnsemble(old_region, new_region, &patch_writer));

  size_t patch_size = patch_writer.SerializedSize();
  EXPECT_GE(patch_size, 80U);  // Minimum size is empty patch.
  // TODO(etiennep): Add check on maximum expected size.

  std::vector<uint8_t> patch_buffer(patch_writer.SerializedSize());
  patch_writer.SerializeInto({patch_buffer.data(), patch_buffer.size()});

  // Read back generated patch.
  base::Optional<EnsemblePatchReader> patch_reader =
      EnsemblePatchReader::Create({patch_buffer.data(), patch_buffer.size()});
  ASSERT_TRUE(patch_reader.has_value());

  // Check basic properties.
  EXPECT_TRUE(patch_reader->CheckOldFile(old_region));
  EXPECT_TRUE(patch_reader->CheckNewFile(new_region));
  EXPECT_EQ(old_file.length(), patch_reader->header().old_size);
  // If new_size doesn't match expectation, the function is aborted.
  ASSERT_EQ(new_file.length(), patch_reader->header().new_size);

  // Apply patch to "old" to get "patched new", ensure it's identical to "new".
  std::vector<uint8_t> patched_new_buffer(new_region.size());
  ASSERT_EQ(status::kStatusSuccess,
            Apply(old_region, *patch_reader,
                  {patched_new_buffer.data(), patched_new_buffer.size()}));

  // Note that |new_region| and |patched_new_buffer| are the same size.
  EXPECT_TRUE(std::equal(new_region.begin(), new_region.end(),
                         patched_new_buffer.begin()));
}

TEST(EndToEndTest, GenApplyRaw) {
  TestGenApply("setup1.exe", "setup2.exe", true);
  TestGenApply("chrome64_1.exe", "chrome64_2.exe", true);
}

TEST(EndToEndTest, GenApplyIdentity) {
  TestGenApply("setup1.exe", "setup1.exe", false);
}

TEST(EndToEndTest, GenApplySimple) {
  TestGenApply("setup1.exe", "setup2.exe", false);
  TestGenApply("setup2.exe", "setup1.exe", false);
  TestGenApply("chrome64_1.exe", "chrome64_2.exe", false);
}

TEST(EndToEndTest, GenApplyCross) {
  TestGenApply("setup1.exe", "chrome64_1.exe", false);
}

}  // namespace zucchini
