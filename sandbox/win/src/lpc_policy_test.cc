// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// These tests have been added to specifically tests issues arising from (A)LPC
// lock down.

#include <algorithm>
#include <cctype>

#include <windows.h>
#include <winioctl.h>

#include "base/win/windows_version.h"
#include "sandbox/win/src/heap_helper.h"
#include "sandbox/win/src/sandbox.h"
#include "sandbox/win/src/sandbox_factory.h"
#include "sandbox/win/src/sandbox_policy.h"
#include "sandbox/win/tests/common/controller.h"
#include "sandbox/win/tests/common/test_utils.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace sandbox {

namespace {

bool CsrssDisconnectSupported() {
  // This functionality has not been verified on versions before Win10.
  if (base::win::GetVersion() < base::win::VERSION_WIN10)
    return false;

  // Does not work on 32-bit on x64 (ie Wow64).
  return (base::win::OSInfo::GetInstance()->wow64_status() !=
          base::win::OSInfo::WOW64_ENABLED);
}

}  // namespace
// Converts LCID to std::wstring for passing to sbox tests.
std::wstring LcidToWString(LCID lcid) {
  wchar_t buff[10] = {0};
  int res = swprintf_s(buff, sizeof(buff) / sizeof(buff[0]), L"%08x", lcid);
  if (-1 != res) {
    return std::wstring(buff);
  }
  return std::wstring();
}

// Converts LANGID to std::wstring for passing to sbox tests.
std::wstring LangidToWString(LANGID langid) {
  wchar_t buff[10] = {0};
  int res = swprintf_s(buff, sizeof(buff) / sizeof(buff[0]), L"%04x", langid);
  if (-1 != res) {
    return std::wstring(buff);
  }
  return std::wstring();
}

SBOX_TESTS_COMMAND int Lpc_GetUserDefaultLangID(int argc, wchar_t** argv) {
  if (argc != 1)
    return SBOX_TEST_FAILED_TO_EXECUTE_COMMAND;
  std::wstring expected_langid_string(argv[0]);

  // This will cause an exception if not warmed up suitably.
  LANGID langid = ::GetUserDefaultLangID();

  std::wstring langid_string = LangidToWString(langid);
  if (0 == wcsncmp(langid_string.c_str(), expected_langid_string.c_str(), 4)) {
    return SBOX_TEST_SUCCEEDED;
  }
  return SBOX_TEST_FAILED;
}

TEST(LpcPolicyTest, GetUserDefaultLangID) {
  LANGID langid = ::GetUserDefaultLangID();
  std::wstring cmd = L"Lpc_GetUserDefaultLangID " + LangidToWString(langid);
  TestRunner runner;
  EXPECT_EQ(SBOX_TEST_SUCCEEDED, runner.RunTest(cmd.c_str()));
}

SBOX_TESTS_COMMAND int Lpc_GetUserDefaultLCID(int argc, wchar_t** argv) {
  if (argc != 1)
    return SBOX_TEST_FAILED_TO_EXECUTE_COMMAND;
  std::wstring expected_lcid_string(argv[0]);

  // This will cause an exception if not warmed up suitably.
  LCID lcid = ::GetUserDefaultLCID();

  std::wstring lcid_string = LcidToWString(lcid);
  if (0 == wcsncmp(lcid_string.c_str(), expected_lcid_string.c_str(), 8)) {
    return SBOX_TEST_SUCCEEDED;
  }
  return SBOX_TEST_FAILED;
}

TEST(LpcPolicyTest, GetUserDefaultLCID) {
  LCID lcid = ::GetUserDefaultLCID();
  std::wstring cmd = L"Lpc_GetUserDefaultLCID " + LcidToWString(lcid);
  TestRunner runner;
  EXPECT_EQ(SBOX_TEST_SUCCEEDED, runner.RunTest(cmd.c_str()));
}

// GetUserDefaultLocaleName is not available on WIN XP.  So we'll
// load it on-the-fly.
const wchar_t kKernel32DllName[] = L"kernel32.dll";
typedef int(WINAPI* GetUserDefaultLocaleNameFunction)(LPWSTR lpLocaleName,
                                                      int cchLocaleName);

SBOX_TESTS_COMMAND int Lpc_GetUserDefaultLocaleName(int argc, wchar_t** argv) {
  if (argc != 1)
    return SBOX_TEST_FAILED_TO_EXECUTE_COMMAND;
  std::wstring expected_locale_name(argv[0]);
  static GetUserDefaultLocaleNameFunction GetUserDefaultLocaleName_func = NULL;
  if (!GetUserDefaultLocaleName_func) {
    // GetUserDefaultLocaleName is not available on WIN XP.  So we'll
    // load it on-the-fly.
    HMODULE kernel32_dll = ::GetModuleHandle(kKernel32DllName);
    if (!kernel32_dll) {
      return SBOX_TEST_FAILED;
    }
    GetUserDefaultLocaleName_func =
        reinterpret_cast<GetUserDefaultLocaleNameFunction>(
            GetProcAddress(kernel32_dll, "GetUserDefaultLocaleName"));
    if (!GetUserDefaultLocaleName_func) {
      return SBOX_TEST_FAILED;
    }
  }
  wchar_t locale_name[LOCALE_NAME_MAX_LENGTH] = {0};
  // This will cause an exception if not warmed up suitably.
  int ret = GetUserDefaultLocaleName_func(
      locale_name, LOCALE_NAME_MAX_LENGTH * sizeof(wchar_t));
  if (!ret) {
    return SBOX_TEST_FAILED;
  }
  if (!wcsnlen(locale_name, LOCALE_NAME_MAX_LENGTH)) {
    return SBOX_TEST_FAILED;
  }
  if (0 == wcsncmp(locale_name, expected_locale_name.c_str(),
                   LOCALE_NAME_MAX_LENGTH)) {
    return SBOX_TEST_SUCCEEDED;
  }
  return SBOX_TEST_FAILED;
}

TEST(LpcPolicyTest, GetUserDefaultLocaleName) {
  static GetUserDefaultLocaleNameFunction GetUserDefaultLocaleName_func = NULL;
  if (!GetUserDefaultLocaleName_func) {
    // GetUserDefaultLocaleName is not available on WIN XP.  So we'll
    // load it on-the-fly.
    HMODULE kernel32_dll = ::GetModuleHandle(kKernel32DllName);
    EXPECT_NE(nullptr, kernel32_dll);
    GetUserDefaultLocaleName_func =
        reinterpret_cast<GetUserDefaultLocaleNameFunction>(
            GetProcAddress(kernel32_dll, "GetUserDefaultLocaleName"));
    EXPECT_NE(nullptr, GetUserDefaultLocaleName_func);
  }
  wchar_t locale_name[LOCALE_NAME_MAX_LENGTH] = {0};
  EXPECT_NE(0, GetUserDefaultLocaleName_func(
                   locale_name, LOCALE_NAME_MAX_LENGTH * sizeof(wchar_t)));
  EXPECT_NE(0U, wcsnlen(locale_name, LOCALE_NAME_MAX_LENGTH));
  std::wstring cmd =
      L"Lpc_GetUserDefaultLocaleName " + std::wstring(locale_name);
  TestRunner runner;
  EXPECT_EQ(SBOX_TEST_SUCCEEDED, runner.RunTest(cmd.c_str()));
}

// Closing ALPC port can invalidate its heap.
// Test that all heaps are valid.
SBOX_TESTS_COMMAND int Lpc_TestValidProcessHeaps(int argc, wchar_t** argv) {
  if (argc != 0)
    return SBOX_TEST_FAILED_TO_EXECUTE_COMMAND;
  // Retrieves the number of heaps in the current process.
  DWORD number_of_heaps = ::GetProcessHeaps(0, NULL);
  // Try to retrieve a handle to all the heaps owned by this process. Returns
  // false if the number of heaps has changed.
  //
  // This is inherently racy as is, but it's not something that we observe a lot
  // in Chrome, the heaps tend to be created at startup only.
  std::unique_ptr<HANDLE[]> all_heaps(new HANDLE[number_of_heaps]);
  if (::GetProcessHeaps(number_of_heaps, all_heaps.get()) != number_of_heaps)
    return SBOX_TEST_FIRST_ERROR;

  for (size_t i = 0; i < number_of_heaps; ++i) {
    HANDLE handle = all_heaps[i];
    ULONG HeapInformation;
    BOOL result =
        HeapQueryInformation(handle, HeapCompatibilityInformation,
                             &HeapInformation, sizeof(HeapInformation), NULL);
    if (!result)
      return SBOX_TEST_SECOND_ERROR;
  }
  return SBOX_TEST_SUCCEEDED;
}

TEST(LpcPolicyTest, TestValidProcessHeaps) {
  TestRunner runner;
  EXPECT_EQ(SBOX_TEST_SUCCEEDED, runner.RunTest(L"Lpc_TestValidProcessHeaps"));
}

// All processes should have a shared heap with csrss.exe. This test ensures
// that this heap can be found.
TEST(LpcPolicyTest, TestCanFindCsrPortHeap) {
  if (!CsrssDisconnectSupported()) {
    return;
  }
  HANDLE csr_port_handle = sandbox::FindCsrPortHeap();
  EXPECT_NE(nullptr, csr_port_handle);
}

TEST(LpcPolicyTest, TestHeapFlags) {
  if (!CsrssDisconnectSupported()) {
    // This functionality has not been verified on versions before Win10.
    return;
  }
  // Windows does not support callers supplying arbritary flag values. So we
  // write some non-trivial value to reduce the chance we match this in random
  // data.
  DWORD flags = 0x41007;
  HANDLE heap = HeapCreate(flags, 0, 0);
  EXPECT_NE(nullptr, heap);
  DWORD actual_flags = 0;
  EXPECT_TRUE(sandbox::HeapFlags(heap, &actual_flags));
  EXPECT_EQ(flags, actual_flags);
  EXPECT_TRUE(HeapDestroy(heap));
}

}  // namespace sandbox
