// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_TEST_MOCK_PERMISSION_MANAGER_H_
#define CONTENT_TEST_MOCK_PERMISSION_MANAGER_H_

#include "content/public/browser/permission_manager.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "url/gurl.h"

class GURL;

namespace content {

enum class PermissionType;

// Mock of the permission manager for unit tests.
class MockPermissionManager : public PermissionManager {
 public:
  MockPermissionManager();

  ~MockPermissionManager() override;

  // PermissionManager:
  MOCK_METHOD3(GetPermissionStatus,
               blink::mojom::PermissionStatus(PermissionType permission,
                                              const GURL& requesting_origin,
                                              const GURL& embedding_origin));
  int RequestPermission(
      PermissionType permission,
      RenderFrameHost* render_frame_host,
      const GURL& requesting_origin,
      bool user_gesture,
      const base::Callback<void(blink::mojom::PermissionStatus)>& callback)
      override;
  int RequestPermissions(
      const std::vector<PermissionType>& permission,
      RenderFrameHost* render_frame_host,
      const GURL& requesting_origin,
      bool user_gesture,
      const base::Callback<
          void(const std::vector<blink::mojom::PermissionStatus>&)>& callback)
      override;
  void CancelPermissionRequest(int request_id) override {}
  void ResetPermission(PermissionType permission,
                       const GURL& requesting_origin,
                       const GURL& embedding_origin) override {}
  int SubscribePermissionStatusChange(
      PermissionType permission,
      const GURL& requesting_origin,
      const GURL& embedding_origin,
      const base::Callback<void(blink::mojom::PermissionStatus)>& callback)
      override;
  void UnsubscribePermissionStatusChange(int subscription_id) override {}

 private:
  DISALLOW_COPY_AND_ASSIGN(MockPermissionManager);
};

}  // namespace content

#endif  // CONTENT_TEST_MOCK_PERMISSION_MANAGER_H_
