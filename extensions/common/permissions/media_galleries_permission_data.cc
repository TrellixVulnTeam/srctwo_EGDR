// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/common/permissions/media_galleries_permission_data.h"

#include "base/strings/string_util.h"
#include "base/values.h"
#include "extensions/common/permissions/media_galleries_permission.h"

namespace extensions {

MediaGalleriesPermissionData::MediaGalleriesPermissionData() {
}

bool MediaGalleriesPermissionData::Check(
    const APIPermission::CheckParam* param) const {
  if (!param)
    return false;

  const MediaGalleriesPermission::CheckParam& specific_param =
      *static_cast<const MediaGalleriesPermission::CheckParam*>(param);
  return permission_ == specific_param.permission;
}

std::unique_ptr<base::Value> MediaGalleriesPermissionData::ToValue() const {
  return std::unique_ptr<base::Value>(new base::Value(permission_));
}

bool MediaGalleriesPermissionData::FromValue(const base::Value* value) {
  if (!value)
    return false;

  std::string raw_permission;
  if (!value->GetAsString(&raw_permission))
    return false;

  std::string permission;
  base::TrimWhitespaceASCII(raw_permission, base::TRIM_ALL, &permission);

  if (permission == MediaGalleriesPermission::kAllAutoDetectedPermission ||
      permission == MediaGalleriesPermission::kReadPermission ||
      permission == MediaGalleriesPermission::kCopyToPermission ||
      permission == MediaGalleriesPermission::kDeletePermission) {
    permission_ = permission;
    return true;
  }
  return false;
}

bool MediaGalleriesPermissionData::operator<(
    const MediaGalleriesPermissionData& rhs) const {
  return permission_ < rhs.permission_;
}

bool MediaGalleriesPermissionData::operator==(
    const MediaGalleriesPermissionData& rhs) const {
  return permission_ == rhs.permission_;
}

}  // namespace extensions
