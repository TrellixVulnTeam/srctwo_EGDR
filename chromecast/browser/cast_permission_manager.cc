// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromecast/browser/cast_permission_manager.h"

#include "base/callback.h"
#include "base/logging.h"
#include "content/public/browser/permission_type.h"

namespace chromecast {
namespace shell {

CastPermissionManager::CastPermissionManager()
    : content::PermissionManager() {
}

CastPermissionManager::~CastPermissionManager() {
}

int CastPermissionManager::RequestPermission(
    content::PermissionType permission,
    content::RenderFrameHost* render_frame_host,
    const GURL& origin,
    bool user_gesture,
    const base::Callback<void(blink::mojom::PermissionStatus)>& callback) {
  LOG(INFO) << __FUNCTION__ << ": " << static_cast<int>(permission);
  callback.Run(blink::mojom::PermissionStatus::GRANTED);
  return kNoPendingOperation;
}

int CastPermissionManager::RequestPermissions(
    const std::vector<content::PermissionType>& permissions,
    content::RenderFrameHost* render_frame_host,
    const GURL& requesting_origin,
    bool user_gesture,
    const base::Callback<
        void(const std::vector<blink::mojom::PermissionStatus>&)>& callback) {
  callback.Run(std::vector<blink::mojom::PermissionStatus>(
      permissions.size(), blink::mojom::PermissionStatus::GRANTED));
  return kNoPendingOperation;
}

void CastPermissionManager::CancelPermissionRequest(int request_id) {
}

void CastPermissionManager::ResetPermission(
    content::PermissionType permission,
    const GURL& requesting_origin,
    const GURL& embedding_origin) {
}

blink::mojom::PermissionStatus CastPermissionManager::GetPermissionStatus(
    content::PermissionType permission,
    const GURL& requesting_origin,
    const GURL& embedding_origin) {
  LOG(INFO) << __FUNCTION__ << ": " << static_cast<int>(permission);
  return blink::mojom::PermissionStatus::GRANTED;
}

int CastPermissionManager::SubscribePermissionStatusChange(
    content::PermissionType permission,
    const GURL& requesting_origin,
    const GURL& embedding_origin,
    const base::Callback<void(blink::mojom::PermissionStatus)>& callback) {
  return kNoPendingOperation;
}

void CastPermissionManager::UnsubscribePermissionStatusChange(
    int subscription_id) {
}

}  // namespace shell
}  // namespace chromecast
