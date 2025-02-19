// Copyright (c) 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/download/trusted_sources_manager.h"

#include "base/command_line.h"
#include "chrome/common/chrome_switches.h"

TrustedSourcesManager::TrustedSourcesManager() {
  base::CommandLine* command_line(base::CommandLine::ForCurrentProcess());
  DCHECK(command_line);
  rules_.ParseFromString(
      command_line->GetSwitchValueASCII(switches::kTrustedDownloadSources));
}

TrustedSourcesManager::~TrustedSourcesManager() = default;

bool TrustedSourcesManager::IsFromTrustedSource(const GURL& url) const {
  return rules_.Matches(url);
}
