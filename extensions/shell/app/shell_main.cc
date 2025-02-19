// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "build/build_config.h"
#include "content/public/app/content_main.h"
#include "extensions/shell/app/shell_main_delegate.h"

#if defined(OS_WIN)
#include "content/public/app/sandbox_helper_win.h"
#include "sandbox/win/src/sandbox_types.h"
#endif

#if defined(OS_MACOSX)
#include "extensions/shell/app/shell_main_mac.h"
#endif

#if defined(OS_MACOSX)
int main(int argc, const char** argv) {
  // Do the delegate work in shell_main_mac to avoid having to export the
  // delegate types.
  return ::ContentMain(argc, argv);
}
#elif defined(OS_WIN)
int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, wchar_t*, int) {
  extensions::ShellMainDelegate delegate;
  content::ContentMainParams params(&delegate);

  sandbox::SandboxInterfaceInfo sandbox_info = {0};
  content::InitializeSandboxInfo(&sandbox_info);
  params.instance = instance;
  params.sandbox_info = &sandbox_info;

  return content::ContentMain(params);
}
#else  // non-Mac POSIX
int main(int argc, const char** argv) {
  extensions::ShellMainDelegate delegate;
  content::ContentMainParams params(&delegate);

  params.argc = argc;
  params.argv = argv;

  return content::ContentMain(params);
}
#endif
