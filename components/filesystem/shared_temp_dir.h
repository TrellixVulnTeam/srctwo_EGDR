// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_FILESYSTEM_SHARED_TEMP_DIR_H_
#define COMPONENTS_FILESYSTEM_SHARED_TEMP_DIR_H_

#include "base/memory/ref_counted.h"

namespace base {
class ScopedTempDir;
}

namespace filesystem {

// A class to allow multiple mojom::Directory objects to hold a reference to a
// temporary directory.
class SharedTempDir : public base::RefCounted<SharedTempDir> {
 public:
  SharedTempDir(std::unique_ptr<base::ScopedTempDir> temp_dir);

 private:
  friend class base::RefCounted<SharedTempDir>;
  ~SharedTempDir();

  std::unique_ptr<base::ScopedTempDir> temp_dir_;

  DISALLOW_COPY_AND_ASSIGN(SharedTempDir);
};

}  // namespace filesystem

#endif  // COMPONENTS_FILESYSTEM_SHARED_TEMP_DIR_H_
