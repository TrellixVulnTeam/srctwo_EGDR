// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/public/cpp/window_pin_type.h"

#include "ash/public/interfaces/window_pin_type.mojom.h"

namespace ash {

bool IsValidWindowPinType(int64_t value) {
  return value == int64_t(ash::mojom::WindowPinType::NONE) ||
         value == int64_t(ash::mojom::WindowPinType::PINNED) ||
         value == int64_t(ash::mojom::WindowPinType::TRUSTED_PINNED);
}

}  // namespace ash
