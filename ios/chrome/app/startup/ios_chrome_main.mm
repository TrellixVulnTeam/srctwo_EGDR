// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/chrome/app/startup/ios_chrome_main.h"

#import <UIKit/UIKit.h>

#include <vector>

#include "base/bind.h"
#include "base/logging.h"
#include "base/strings/sys_string_conversions.h"
#include "base/time/time.h"
#include "components/task_scheduler_util/browser/initialization.h"
#include "ios/web/public/app/web_main_runner.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

namespace {
base::Time* g_start_time;
}  // namespace

IOSChromeMain::IOSChromeMain() {
  web_main_runner_.reset(web::WebMainRunner::Create());

  web::WebMainParams main_params(&main_delegate_);
// Copy NSProcessInfo arguments into WebMainParams in debug only, since
// command line should be meaningless outside of developer builds.
#if !defined(NDEBUG)
  NSArray* arguments = [[NSProcessInfo processInfo] arguments];
  main_params.argc = [arguments count];
  const char* argv[main_params.argc];
  std::vector<std::string> argv_store;

  // Avoid using std::vector::push_back (or any other method that could cause
  // the vector to grow) as this will cause the std::string to be copied or
  // moved (depends on the C++ implementation) which may invalidates the pointer
  // returned by std::string::c_str(). Even if the strings are moved, this may
  // cause garbage if std::string uses optimisation for small strings (by
  // returning pointer to the object internals in that case).
  argv_store.resize([arguments count]);
  for (NSUInteger i = 0; i < [arguments count]; i++) {
    argv_store[i] = base::SysNSStringToUTF8([arguments objectAtIndex:i]);
    argv[i] = argv_store[i].c_str();
  }
  main_params.argv = argv;
#endif

  main_params.get_task_scheduler_init_params_callback = base::Bind(
      &task_scheduler_util::GetBrowserTaskSchedulerInitParamsFromVariations);
  // Chrome registers an AtExitManager in main in order to initialize breakpad
  // early, so prevent a second registration by WebMainRunner.
  main_params.register_exit_manager = false;
  web_main_runner_->Initialize(std::move(main_params));
}

IOSChromeMain::~IOSChromeMain() {
  web_main_runner_->ShutDown();
}

// static
void IOSChromeMain::InitStartTime() {
  DCHECK(!g_start_time);
  g_start_time = new base::Time(base::Time::Now());
}

// static
const base::Time& IOSChromeMain::StartTime() {
  CHECK(g_start_time);
  return *g_start_time;
}
