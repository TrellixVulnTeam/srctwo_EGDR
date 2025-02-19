// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/win/taskbar_icon_finder.h"

#include "base/test/scoped_task_environment.h"
#include "testing/gtest/include/gtest/gtest.h"

// The most simple test possible to ensure that the finder doesn't leak or
// cause crashes.
TEST(TaskbarIconFinder, Simple) {
  base::test::ScopedTaskEnvironment task_environment;
  FindTaskbarIconModal();
}
