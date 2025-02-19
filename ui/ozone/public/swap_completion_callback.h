// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_OZONE_PUBLIC_SWAP_COMPLETION_CALLBACK_H_
#define UI_OZONE_PUBLIC_SWAP_COMPLETION_CALLBACK_H_

#include "base/callback.h"
#include "ui/gfx/swap_result.h"

namespace ui {

using SwapCompletionOnceCallback = base::OnceCallback<void(gfx::SwapResult)>;

}  // namespace ui

#endif  // UI_OZONE_PUBLIC_SWAP_COMPLETION_CALLBACK_H_
