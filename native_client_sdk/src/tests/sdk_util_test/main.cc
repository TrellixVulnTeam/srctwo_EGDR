// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "gtest/gtest.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/var.h"
#include "ppapi_simple/ps_main.h"

#if defined(WIN32)
#include <Windows.h>
#undef PostMessage
#endif

class GTestEventListener : public ::testing::EmptyTestEventListener {
 public:
  // TestEventListener overrides.
  virtual void OnTestStart(const ::testing::TestInfo& test_info) {
    std::stringstream msg;
    msg << "start:" << test_info.test_case_name() << "." << test_info.name();
    pp::Instance(PSGetInstanceId()).PostMessage(msg.str());
  }

  virtual void OnTestPartResult(
      const ::testing::TestPartResult& test_part_result) {
    if (test_part_result.failed()) {
      std::stringstream msg;
      msg << "fail:" << test_part_result.file_name() << ","
          << test_part_result.line_number() << ","
          << test_part_result.summary();
      pp::Instance(PSGetInstanceId()).PostMessage(msg.str());
    }
  }

  virtual void OnTestEnd(const ::testing::TestInfo& test_info) {
    std::stringstream msg;
    msg << "end:" << test_info.test_case_name() << "." << test_info.name()
        << "," << (test_info.result()->Failed() ? "failed" : "ok");
    pp::Instance(PSGetInstanceId()).PostMessage(msg.str());
  }
};

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  if (PSGetInstanceId() != 0) {
    ::testing::UnitTest::GetInstance()->listeners()
        .Append(new GTestEventListener());
  }
  return RUN_ALL_TESTS();
}
