// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/bind.h"
#include "base/test/launcher/unit_test_launcher.h"
#include "base/test/test_suite.h"
#include "mojo/edk/embedder/embedder.h"
#include "ui/compositor/test/test_suite.h"

int main(int argc, char** argv) {
  mojo::edk::Init();

#if defined(USE_AURA)
  ui::test::CompositorTestSuite test_suite(argc, argv);

  return base::LaunchUnitTests(
      argc, argv, base::Bind(&ui::test::CompositorTestSuite::Run,
                             base::Unretained(&test_suite)));
#else
  base::TestSuite test_suite(argc, argv);
  return base::LaunchUnitTests(
      argc, argv,
      base::Bind(&base::TestSuite::Run, base::Unretained(&test_suite)));
#endif
}
