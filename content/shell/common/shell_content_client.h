// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SHELL_COMMON_SHELL_CONTENT_CLIENT_H_
#define CONTENT_SHELL_COMMON_SHELL_CONTENT_CLIENT_H_

#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "content/public/common/content_client.h"
#include "content/public/common/origin_trial_policy.h"
#include "content/shell/common/shell_origin_trial_policy.h"

namespace content {

std::string GetShellUserAgent();

class ShellContentClient : public ContentClient {
 public:
  ShellContentClient();
  ~ShellContentClient() override;

  std::string GetUserAgent() const override;
  base::string16 GetLocalizedString(int message_id) const override;
  base::StringPiece GetDataResource(
      int resource_id,
      ui::ScaleFactor scale_factor) const override;
  base::RefCountedMemory* GetDataResourceBytes(
      int resource_id) const override;
  gfx::Image& GetNativeImageNamed(int resource_id) const override;
  bool IsSupplementarySiteIsolationModeEnabled() override;
  OriginTrialPolicy* GetOriginTrialPolicy() override;

 private:
  ShellOriginTrialPolicy origin_trial_policy_;
};

}  // namespace content

#endif  // CONTENT_SHELL_COMMON_SHELL_CONTENT_CLIENT_H_
