// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/test/test_permission_message_provider.h"

namespace extensions {

TestPermissionMessageProvider::TestPermissionMessageProvider() {
}

TestPermissionMessageProvider::~TestPermissionMessageProvider() {
}

PermissionMessages TestPermissionMessageProvider::GetPermissionMessages(
    const PermissionIDSet& permissions) const {
  return PermissionMessages();
}

bool TestPermissionMessageProvider::IsPrivilegeIncrease(
    const PermissionSet& old_permissions,
    const PermissionSet& new_permissions,
    Manifest::Type extension_type) const {
  return false;
}

PermissionIDSet TestPermissionMessageProvider::GetAllPermissionIDs(
    const PermissionSet& permissions,
    Manifest::Type extension_type) const {
  return PermissionIDSet();
}

}  // namespace extensions
