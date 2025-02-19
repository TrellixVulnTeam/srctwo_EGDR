// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_EDK_TEST_TEST_SUPPORT_IMPL_H_
#define MOJO_EDK_TEST_TEST_SUPPORT_IMPL_H_

#include <stdio.h>

#include "base/macros.h"
#include "mojo/public/tests/test_support_private.h"

namespace mojo {
namespace edk {
namespace test {

class TestSupportImpl : public mojo::test::TestSupport {
 public:
  TestSupportImpl();
  ~TestSupportImpl() override;

  void LogPerfResult(const char* test_name,
                     const char* sub_test_name,
                     double value,
                     const char* units) override;
  FILE* OpenSourceRootRelativeFile(const char* relative_path) override;
  char** EnumerateSourceRootRelativeDirectory(
      const char* relative_path) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(TestSupportImpl);
};

}  // namespace test
}  // namespace edk
}  // namespace mojo

#endif  // MOJO_EDK_TEST_TEST_SUPPORT_IMPL_H_
