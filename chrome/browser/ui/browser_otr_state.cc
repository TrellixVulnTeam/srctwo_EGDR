// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/browser_otr_state.h"

#include "chrome/browser/ui/browser_list.h"

namespace chrome {

bool IsIncognitoSessionActive() {
  return BrowserList::IsIncognitoSessionActive();
}

}  // namespace chrome
