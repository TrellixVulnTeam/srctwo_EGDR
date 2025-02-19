// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_SHELL_BROWSER_SHELL_DISPLAY_INFO_PROVIDER_H_
#define EXTENSIONS_SHELL_BROWSER_SHELL_DISPLAY_INFO_PROVIDER_H_

#include "base/macros.h"
#include "extensions/browser/api/system_display/display_info_provider.h"

namespace extensions {

class ShellDisplayInfoProvider : public DisplayInfoProvider {
 public:
  ShellDisplayInfoProvider();
  ~ShellDisplayInfoProvider() override;

  // DisplayInfoProvider implementation.
  bool SetInfo(const std::string& display_id,
               const api::system_display::DisplayProperties& info,
               std::string* error) override;
  void UpdateDisplayUnitInfoForPlatform(
      const display::Display& display,
      extensions::api::system_display::DisplayUnitInfo* unit) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(ShellDisplayInfoProvider);
};

}  // namespace extensions

#endif  // EXTENSIONS_SHELL_BROWSER_SHELL_DISPLAY_INFO_PROVIDER_H_
