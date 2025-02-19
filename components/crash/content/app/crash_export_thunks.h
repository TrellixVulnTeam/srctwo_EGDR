// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_CRASH_CONTENT_APP_CRASH_EXPORT_THUNKS_H_
#define COMPONENTS_CRASH_CONTENT_APP_CRASH_EXPORT_THUNKS_H_

#include <stddef.h>
#include <windows.h>

#include "build/build_config.h"

namespace crash_reporter {
struct Report;
}

extern "C" {

// All the functions in this file are named with a suffix of _ExportThunk to
// make sure their names cannot easily collide with random other functions.
// The Microsoft Visual Studio linker has a misfeature where it searches rather
// aggressively for functions to match exports in .DEF files.
// See https://crbug.com/760385#c11 for how this can be bad.

// This function may be invoked across module boundaries to request a single
// crash report upload. See CrashUploadListCrashpad.
void RequestSingleCrashUpload_ExportThunk(const char* local_id);

// This function may be invoked across module boundaries to retrieve the crash
// list. It copies up to |report_count| reports into |reports| and returns the
// number of reports available. If the return value is less than or equal to
// |reports_size|, then |reports| contains all the available reports.
size_t GetCrashReports_ExportThunk(crash_reporter::Report* reports,
                                   size_t reports_size);

// Crashes the process after generating a dump for the provided exception. Note
// that the crash reporter should be initialized before calling this function
// for it to do anything.
// NOTE: This function is used by SyzyASAN to invoke a crash. If you change the
// the name or signature of this function you will break SyzyASAN instrumented
// releases of Chrome. Please contact syzygy-team@chromium.org before doing so!
int CrashForException_ExportThunk(EXCEPTION_POINTERS* info);

// This function is used in chrome_metrics_services_manager_client.cc to trigger
// changes to the upload-enabled state. This is done when the metrics services
// are initialized, and when the user changes their consent for uploads. See
// crash_reporter::SetUploadConsent for effects. The given consent value should
// be consistent with
// crash_reporter::GetCrashReporterClient()->GetCollectStatsConsent(), but it's
// not enforced to avoid blocking startup code on synchronizing them.
void SetUploadConsent_ExportThunk(bool consent);

// NOTE: This function is used by SyzyASAN to annotate crash reports. If you
// change the name or signature of this function you will break SyzyASAN
// instrumented releases of Chrome. Please contact syzygy-team@chromium.org
// before doing so! See also http://crbug.com/567781.
void SetCrashKeyValue_ExportThunk(const wchar_t* key, const wchar_t* value);

void ClearCrashKeyValue_ExportThunk(const wchar_t* key);

void SetCrashKeyValueEx_ExportThunk(const char* key,
                                    size_t key_len,
                                    const char* value,
                                    size_t value_len);

void ClearCrashKeyValueEx_ExportThunk(const char* key, size_t key_len);

// Injects a thread into a remote process to dump state when there is no crash.
// |serialized_crash_keys| is a nul terminated string in the address space of
// |process| that represents serialized crash keys sent from the browser.
// Keys and values are separated by ':', and key/value pairs are separated by
// ','. All keys should be previously registered as crash keys.
// This method is used solely to classify hung input.
HANDLE InjectDumpForHungInput_ExportThunk(HANDLE process,
                                          void* serialized_crash_keys);

// Injects a thread into a remote process to dump state when there is no crash.
// This method provides |reason| which will interpreted as an integer and logged
// as a crash key.
HANDLE InjectDumpForHungInputNoCrashKeys_ExportThunk(HANDLE process,
                                                     int reason);

#if defined(ARCH_CPU_X86_64)
// V8 support functions.
void RegisterNonABICompliantCodeRange_ExportThunk(void* start,
                                                  size_t size_in_bytes);
void UnregisterNonABICompliantCodeRange_ExportThunk(void* start);
#endif  // defined(ARCH_CPU_X86_64)

}  // extern "C"

#endif  // COMPONENTS_CRASH_CONTENT_APP_CRASH_EXPORT_THUNKS_H_
