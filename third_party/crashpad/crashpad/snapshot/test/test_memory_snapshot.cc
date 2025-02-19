// Copyright 2014 The Crashpad Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "snapshot/test/test_memory_snapshot.h"

#include <string>

namespace crashpad {
namespace test {

TestMemorySnapshot::TestMemorySnapshot()
    : address_(0), size_(0), value_('\0') {
}

TestMemorySnapshot::~TestMemorySnapshot() {
}

uint64_t TestMemorySnapshot::Address() const {
  return address_;
}

size_t TestMemorySnapshot::Size() const {
  return size_;
}

bool TestMemorySnapshot::Read(Delegate* delegate) const {
  if (size_ == 0) {
    return delegate->MemorySnapshotDelegateRead(nullptr, size_);
  }

  std::string buffer(size_, value_);
  return delegate->MemorySnapshotDelegateRead(&buffer[0], size_);
}

}  // namespace test
}  // namespace crashpad
