// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/common/conflicts/module_watcher_win.h"

#include <windows.h>
#include <tlhelp32.h>
#include <winternl.h>  // For UNICODE_STRING.

#include <string>
#include <utility>

#include "base/lazy_instance.h"
#include "base/memory/ptr_util.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "base/synchronization/lock.h"
#include "base/win/scoped_handle.h"

// These structures and functions are documented in MSDN, see
// http://msdn.microsoft.com/en-us/library/gg547638(v=vs.85).aspx
// there are however no headers or import libraries available in the
// Platform SDK. They are declared outside of the anonymous namespace to
// allow them to be forward declared in the header file.
enum {
  // The DLL was loaded. The NotificationData parameter points to a
  // LDR_DLL_LOADED_NOTIFICATION_DATA structure.
  LDR_DLL_NOTIFICATION_REASON_LOADED = 1,
  // The DLL was unloaded. The NotificationData parameter points to a
  // LDR_DLL_UNLOADED_NOTIFICATION_DATA structure.
  LDR_DLL_NOTIFICATION_REASON_UNLOADED = 2,
};

// Structure that is used for module load notifications.
struct LDR_DLL_LOADED_NOTIFICATION_DATA {
  // Reserved.
  ULONG Flags;
  // The full path name of the DLL module.
  PCUNICODE_STRING FullDllName;
  // The base file name of the DLL module.
  PCUNICODE_STRING BaseDllName;
  // A pointer to the base address for the DLL in memory.
  PVOID DllBase;
  // The size of the DLL image, in bytes.
  ULONG SizeOfImage;
};
using PLDR_DLL_LOADED_NOTIFICATION_DATA = LDR_DLL_LOADED_NOTIFICATION_DATA*;

// Structure that is used for module unload notifications.
struct LDR_DLL_UNLOADED_NOTIFICATION_DATA {
  // Reserved.
  ULONG Flags;
  // The full path name of the DLL module.
  PCUNICODE_STRING FullDllName;
  // The base file name of the DLL module.
  PCUNICODE_STRING BaseDllName;
  // A pointer to the base address for the DLL in memory.
  PVOID DllBase;
  // The size of the DLL image, in bytes.
  ULONG SizeOfImage;
};
using PLDR_DLL_UNLOADED_NOTIFICATION_DATA = LDR_DLL_UNLOADED_NOTIFICATION_DATA*;

// Union that is used for notifications.
union LDR_DLL_NOTIFICATION_DATA {
  LDR_DLL_LOADED_NOTIFICATION_DATA Loaded;
  LDR_DLL_UNLOADED_NOTIFICATION_DATA Unloaded;
};
using PLDR_DLL_NOTIFICATION_DATA = LDR_DLL_NOTIFICATION_DATA*;

// Signature of the notification callback function.
using PLDR_DLL_NOTIFICATION_FUNCTION =
    VOID(CALLBACK*)(ULONG notification_reason,
                    const LDR_DLL_NOTIFICATION_DATA* notification_data,
                    PVOID context);

// Signatures of the functions used for registering DLL notification callbacks.
using LdrRegisterDllNotificationFunc =
    NTSTATUS(NTAPI*)(ULONG flags,
                     PLDR_DLL_NOTIFICATION_FUNCTION notification_function,
                     PVOID context,
                     PVOID* cookie);
using LdrUnregisterDllNotificationFunc = NTSTATUS(NTAPI*)(PVOID cookie);

namespace {

// Global lock for ensuring synchronization of destruction and notifications.
base::LazyInstance<base::Lock>::Leaky g_module_watcher_lock =
    LAZY_INSTANCE_INITIALIZER;
// Global pointer to the singleton ModuleWatcher, if one exists. Under
// |module_watcher_lock|.
ModuleWatcher* g_module_watcher_instance = nullptr;

// Names of the DLL notification registration functions. These are exported by
// ntdll.
constexpr wchar_t kNtDll[] = L"ntdll.dll";
constexpr char kLdrRegisterDllNotification[] = "LdrRegisterDllNotification";
constexpr char kLdrUnregisterDllNotification[] = "LdrUnregisterDllNotification";

// Helper function for converting a UNICODE_STRING to a FilePath.
base::FilePath ToFilePath(const UNICODE_STRING* str) {
  return base::FilePath(
      base::StringPiece16(str->Buffer, str->Length / sizeof(wchar_t)));
}

template <typename NotificationDataType>
void OnModuleEvent(mojom::ModuleEventType event_type,
                   const NotificationDataType& notification_data,
                   const ModuleWatcher::OnModuleEventCallback& callback) {
  ModuleWatcher::ModuleEvent event(
      event_type, ToFilePath(notification_data.FullDllName),
      notification_data.DllBase, notification_data.SizeOfImage);
  callback.Run(event);
}

}  // namespace

// static
std::unique_ptr<ModuleWatcher> ModuleWatcher::Create(
    OnModuleEventCallback callback) {
  {
    base::AutoLock lock(g_module_watcher_lock.Get());
    // If a ModuleWatcher already exists then bail out.
    if (g_module_watcher_instance)
      return nullptr;
    g_module_watcher_instance = new ModuleWatcher();
  }
  g_module_watcher_instance->Initialize(std::move(callback));
  return base::WrapUnique(g_module_watcher_instance);
}

ModuleWatcher::~ModuleWatcher() {
  // As soon as |g_module_watcher_instance| is null any dispatched callbacks
  // will be silently absorbed by LoaderNotificationCallback.
  base::AutoLock lock(g_module_watcher_lock.Get());
  DCHECK_EQ(g_module_watcher_instance, this);
  g_module_watcher_instance = nullptr;
  UnregisterDllNotificationCallback();
}

// Initializes the ModuleWatcher instance.
void ModuleWatcher::Initialize(OnModuleEventCallback callback) {
  callback_ = std::move(callback);
  RegisterDllNotificationCallback();
  EnumerateAlreadyLoadedModules();
}

void ModuleWatcher::RegisterDllNotificationCallback() {
  // It's safe to pass the return value of ::GetModuleHandle() directly to
  // ::GetProcAddress() because ntdll is guaranteed to be loaded.
  LdrRegisterDllNotificationFunc reg_fn =
      reinterpret_cast<LdrRegisterDllNotificationFunc>(::GetProcAddress(
          ::GetModuleHandle(kNtDll), kLdrRegisterDllNotification));
  if (reg_fn)
    reg_fn(0, &LoaderNotificationCallback, this, &dll_notification_cookie_);
}

void ModuleWatcher::UnregisterDllNotificationCallback() {
  // It's safe to pass the return value of ::GetModuleHandle() directly to
  // ::GetProcAddress() because ntdll is guaranteed to be loaded.
  LdrUnregisterDllNotificationFunc unreg_fn =
      reinterpret_cast<LdrUnregisterDllNotificationFunc>(::GetProcAddress(
          ::GetModuleHandle(kNtDll), kLdrUnregisterDllNotification));
  if (unreg_fn)
    unreg_fn(dll_notification_cookie_);
}

void ModuleWatcher::EnumerateAlreadyLoadedModules() {
  // Get all modules in the current process. According to MSDN,
  // CreateToolhelp32Snapshot should be retried as long as its returning
  // ERROR_BAD_LENGTH. To avoid locking up here a retry limit is enforced.
  base::win::ScopedHandle snap;
  DWORD process_id = ::GetCurrentProcessId();
  for (int i = 0; i < 5; ++i) {
    snap.Set(::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32,
                                        process_id));
    if (snap.IsValid())
      break;
    if (::GetLastError() != ERROR_BAD_LENGTH)
      return;
  }
  if (!snap.IsValid())
    return;

  // Walk the module list.
  MODULEENTRY32 module = {sizeof(module)};
  for (BOOL result = ::Module32First(snap.Get(), &module); result != FALSE;
       result = ::Module32Next(snap.Get(), &module)) {
    ModuleEvent event(mojom::ModuleEventType::MODULE_ALREADY_LOADED,
                      base::FilePath(module.szExePath), module.modBaseAddr,
                      module.modBaseSize);
    callback_.Run(event);
  }
}

// static
ModuleWatcher::OnModuleEventCallback ModuleWatcher::GetCallbackForContext(
    void* context) {
  base::AutoLock lock(g_module_watcher_lock.Get());
  if (context != g_module_watcher_instance)
    return OnModuleEventCallback();
  return g_module_watcher_instance->callback_;
}

// static
void __stdcall ModuleWatcher::LoaderNotificationCallback(
    unsigned long notification_reason,
    const LDR_DLL_NOTIFICATION_DATA* notification_data,
    void* context) {
  auto callback = GetCallbackForContext(context);
  if (!callback)
    return;

  switch (notification_reason) {
    case LDR_DLL_NOTIFICATION_REASON_LOADED:
      OnModuleEvent(mojom::ModuleEventType::MODULE_LOADED,
                    notification_data->Loaded, callback);
      break;

    case LDR_DLL_NOTIFICATION_REASON_UNLOADED:
      // Intentionally ignored.
      break;

    default:
      // This is unexpected, but not a reason to crash.
      NOTREACHED() << "Unknown LDR_DLL_NOTIFICATION_REASON: "
                   << notification_reason;
  }
}

ModuleWatcher::ModuleWatcher() = default;
