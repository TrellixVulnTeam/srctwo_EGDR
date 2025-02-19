// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "build/build_config.h"
#include "headless/public/headless_shell.h"

#if defined(OS_WIN)
#include "content/public/app/sandbox_helper_win.h"
#include "sandbox/win/src/sandbox_types.h"
#endif

int main(int argc, const char** argv) {
#if defined(OS_WIN)
  sandbox::SandboxInterfaceInfo sandbox_info = {0};
  content::InitializeSandboxInfo(&sandbox_info);
  return headless::HeadlessShellMain(0, &sandbox_info);
#else
  return headless::HeadlessShellMain(argc, argv);
#endif  // defined(OS_WIN)
}
