// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Internal header file for the Linux breakpad implementation. This file is
// shared between crash_handler_host_linux.cc and breakpad_linux.cc.

#ifndef COMPONENTS_CRASH_CONTENT_APP_BREAKPAD_LINUX_IMPL_H_
#define COMPONENTS_CRASH_CONTENT_APP_BREAKPAD_LINUX_IMPL_H_

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#include "breakpad/src/common/simple_string_dictionary.h"
#include "components/crash/content/app/breakpad_linux.h"

namespace breakpad {

typedef google_breakpad::NonAllocatingMap<256, 256, 64> CrashKeyStorage;

#if defined(ADDRESS_SANITIZER)
static const size_t kMaxAsanReportSize = 1 << 16;
#endif
// Define a preferred limit on minidump sizes, because Crash Server currently
// throws away any larger than 1.2MB (1.2 * 1024 * 1024).  A value of -1 means
// no limit.
static const off_t kMaxMinidumpFileSize = 1258291;

// The size of the iovec used to transfer crash data from a child back to the
// browser.
#if !defined(ADDRESS_SANITIZER)
const size_t kCrashIovSize = 6;
#else
// Additional field to pass the AddressSanitizer log to the crash handler.
const size_t kCrashIovSize = 7;
#endif

// BreakpadInfo describes a crash report.
// The minidump information can either be contained in a file descriptor (fd) or
// in a file (whose path is in filename).
struct BreakpadInfo {
  int fd;                          // File descriptor to the Breakpad dump data.
  const char* filename;            // Path to the Breakpad dump data.
#if defined(ADDRESS_SANITIZER)
  const char* log_filename;        // Path to the ASan log file.
  const char* asan_report_str;     // ASan report.
  unsigned asan_report_length;     // Length of |asan_report_length|.
#endif
  const char* process_type;        // Process type, e.g. "renderer".
  unsigned process_type_length;    // Length of |process_type|.
  const char* distro;              // Linux distro string.
  unsigned distro_length;          // Length of |distro|.
  bool upload;                     // Whether to upload or save crash dump.
  uint64_t process_start_time;     // Uptime of the crashing process.
  size_t oom_size;                 // Amount of memory requested if OOM.
  uint64_t pid;                    // PID where applicable.
  CrashKeyStorage* crash_keys;
};

extern void HandleCrashDump(const BreakpadInfo& info);

}  // namespace breakpad

#endif  // COMPONENTS_CRASH_CONTENT_APP_BREAKPAD_LINUX_IMPL_H_
