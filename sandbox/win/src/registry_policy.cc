// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include <string>

#include "sandbox/win/src/registry_policy.h"

#include "base/logging.h"
#include "sandbox/win/src/ipc_tags.h"
#include "sandbox/win/src/policy_engine_opcodes.h"
#include "sandbox/win/src/policy_params.h"
#include "sandbox/win/src/sandbox_types.h"
#include "sandbox/win/src/sandbox_utils.h"
#include "sandbox/win/src/win_utils.h"

namespace {

static const uint32_t kAllowedRegFlags =
    KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS | KEY_NOTIFY | KEY_READ |
    GENERIC_READ | GENERIC_EXECUTE | READ_CONTROL;

// Opens the key referenced by |obj_attributes| with |access| and
// checks what permission was given. Remove the WRITE flags and update
// |access| with the new value.
NTSTATUS TranslateMaximumAllowed(OBJECT_ATTRIBUTES* obj_attributes,
                                 DWORD* access) {
  NtOpenKeyFunction NtOpenKey = NULL;
  ResolveNTFunctionPtr("NtOpenKey", &NtOpenKey);

  NtCloseFunction NtClose = NULL;
  ResolveNTFunctionPtr("NtClose", &NtClose);

  NtQueryObjectFunction NtQueryObject = NULL;
  ResolveNTFunctionPtr("NtQueryObject", &NtQueryObject);

  // Open the key.
  HANDLE handle;
  NTSTATUS status = NtOpenKey(&handle, *access, obj_attributes);
  if (!NT_SUCCESS(status))
    return status;

  OBJECT_BASIC_INFORMATION info = {0};
  status = NtQueryObject(handle, ObjectBasicInformation, &info, sizeof(info),
                         NULL);
  CHECK(NT_SUCCESS(NtClose(handle)));
  if (!NT_SUCCESS(status))
    return status;

  *access = info.GrantedAccess & kAllowedRegFlags;
  return STATUS_SUCCESS;
}

NTSTATUS NtCreateKeyInTarget(HANDLE* target_key_handle,
                             ACCESS_MASK desired_access,
                             OBJECT_ATTRIBUTES* obj_attributes,
                             ULONG title_index,
                             UNICODE_STRING* class_name,
                             ULONG create_options,
                             ULONG* disposition,
                             HANDLE target_process) {
  NtCreateKeyFunction NtCreateKey = NULL;
  ResolveNTFunctionPtr("NtCreateKey", &NtCreateKey);

  if (MAXIMUM_ALLOWED & desired_access) {
    NTSTATUS status = TranslateMaximumAllowed(obj_attributes, &desired_access);
    if (!NT_SUCCESS(status))
      return STATUS_ACCESS_DENIED;
  }

  HANDLE local_handle = INVALID_HANDLE_VALUE;
  NTSTATUS status = NtCreateKey(&local_handle, desired_access, obj_attributes,
                                title_index, class_name, create_options,
                                disposition);
  if (!NT_SUCCESS(status))
    return status;

  if (!::DuplicateHandle(::GetCurrentProcess(), local_handle,
                         target_process, target_key_handle, 0, FALSE,
                         DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS)) {
    return STATUS_ACCESS_DENIED;
  }
  return STATUS_SUCCESS;
}

NTSTATUS NtOpenKeyInTarget(HANDLE* target_key_handle,
                           ACCESS_MASK desired_access,
                           OBJECT_ATTRIBUTES* obj_attributes,
                           HANDLE target_process) {
  NtOpenKeyFunction NtOpenKey = NULL;
  ResolveNTFunctionPtr("NtOpenKey", &NtOpenKey);

  if (MAXIMUM_ALLOWED & desired_access) {
    NTSTATUS status = TranslateMaximumAllowed(obj_attributes, &desired_access);
    if (!NT_SUCCESS(status))
      return STATUS_ACCESS_DENIED;
  }

  HANDLE local_handle = INVALID_HANDLE_VALUE;
  NTSTATUS status = NtOpenKey(&local_handle, desired_access, obj_attributes);

  if (!NT_SUCCESS(status))
    return status;

  if (!::DuplicateHandle(::GetCurrentProcess(), local_handle,
                         target_process, target_key_handle, 0, FALSE,
                         DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS)) {
    return STATUS_ACCESS_DENIED;
  }
  return STATUS_SUCCESS;
}

}

namespace sandbox {

bool RegistryPolicy::GenerateRules(const wchar_t* name,
                                   TargetPolicy::Semantics semantics,
                                   LowLevelPolicy* policy) {
  base::string16 resovled_name(name);
  if (resovled_name.empty()) {
    return false;
  }

  if (!ResolveRegistryName(resovled_name, &resovled_name))
    return false;

  name = resovled_name.c_str();

  EvalResult result = ASK_BROKER;

  PolicyRule open(result);
  PolicyRule create(result);

  switch (semantics) {
    case TargetPolicy::REG_ALLOW_READONLY: {
      // We consider all flags that are not known to be readonly as potentially
      // used for write. Here we also support MAXIMUM_ALLOWED, but we are going
      // to expand it to read-only before the call.
      uint32_t restricted_flags = ~(kAllowedRegFlags | MAXIMUM_ALLOWED);
      open.AddNumberMatch(IF_NOT, OpenKey::ACCESS, restricted_flags, AND);
      create.AddNumberMatch(IF_NOT, OpenKey::ACCESS, restricted_flags, AND);
      break;
    }
    case TargetPolicy::REG_ALLOW_ANY: {
      break;
    }
    default: {
      NOTREACHED();
      return false;
    }
  }

  if (!create.AddStringMatch(IF, OpenKey::NAME, name, CASE_INSENSITIVE) ||
      !policy->AddRule(IPC_NTCREATEKEY_TAG, &create)) {
    return false;
  }

  if (!open.AddStringMatch(IF, OpenKey::NAME, name, CASE_INSENSITIVE) ||
      !policy->AddRule(IPC_NTOPENKEY_TAG, &open)) {
    return false;
  }

  return true;
}

bool RegistryPolicy::CreateKeyAction(EvalResult eval_result,
                                     const ClientInfo& client_info,
                                     const base::string16& key,
                                     uint32_t attributes,
                                     HANDLE root_directory,
                                     uint32_t desired_access,
                                     uint32_t title_index,
                                     uint32_t create_options,
                                     HANDLE* handle,
                                     NTSTATUS* nt_status,
                                     ULONG* disposition) {
  // The only action supported is ASK_BROKER which means create the requested
  // file as specified.
  if (ASK_BROKER != eval_result) {
    *nt_status = STATUS_ACCESS_DENIED;
    return false;
  }

  // We don't support creating link keys, volatile keys or backup/restore.
  if (create_options) {
    *nt_status = STATUS_ACCESS_DENIED;
    return false;
  }

  UNICODE_STRING uni_name = {0};
  OBJECT_ATTRIBUTES obj_attributes = {0};
  InitObjectAttribs(key, attributes, root_directory, &obj_attributes,
                    &uni_name, NULL);
  *nt_status = NtCreateKeyInTarget(handle, desired_access, &obj_attributes,
                                   title_index, NULL, create_options,
                                   disposition, client_info.process);
  return true;
}

bool RegistryPolicy::OpenKeyAction(EvalResult eval_result,
                                   const ClientInfo& client_info,
                                   const base::string16& key,
                                   uint32_t attributes,
                                   HANDLE root_directory,
                                   uint32_t desired_access,
                                   HANDLE* handle,
                                   NTSTATUS* nt_status) {
  // The only action supported is ASK_BROKER which means open the requested
  // file as specified.
  if (ASK_BROKER != eval_result) {
    *nt_status = STATUS_ACCESS_DENIED;
    return true;
  }

  UNICODE_STRING uni_name = {0};
  OBJECT_ATTRIBUTES obj_attributes = {0};
  InitObjectAttribs(key, attributes, root_directory, &obj_attributes,
                    &uni_name, NULL);
  *nt_status = NtOpenKeyInTarget(handle, desired_access, &obj_attributes,
                                client_info.process);
  return true;
}

}  // namespace sandbox
