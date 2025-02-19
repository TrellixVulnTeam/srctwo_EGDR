// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_ACCELERATORS_MOJO_ACCELERATOR_STRUCT_TRAITS_H_
#define UI_BASE_ACCELERATORS_MOJO_ACCELERATOR_STRUCT_TRAITS_H_

#include "ui/base/accelerators/accelerator.h"
#include "ui/base/accelerators/mojo/accelerator.mojom.h"
#include "ui/events/keycodes/keyboard_codes.h"

namespace mojo {

template <>
struct EnumTraits<ui::mojom::AcceleratorKeyState, ui::Accelerator::KeyState> {
  static ui::mojom::AcceleratorKeyState ToMojom(
      ui::Accelerator::KeyState input) {
    switch (input) {
      case ui::Accelerator::KeyState::PRESSED:
        return ui::mojom::AcceleratorKeyState::PRESSED;
      case ui::Accelerator::KeyState::RELEASED:
        return ui::mojom::AcceleratorKeyState::RELEASED;
    }
    NOTREACHED();
    return ui::mojom::AcceleratorKeyState::PRESSED;
  }

  static bool FromMojom(ui::mojom::AcceleratorKeyState input,
                        ui::Accelerator::KeyState* out) {
    switch (input) {
      case ui::mojom::AcceleratorKeyState::PRESSED:
        *out = ui::Accelerator::KeyState::PRESSED;
        return true;
      case ui::mojom::AcceleratorKeyState::RELEASED:
        *out = ui::Accelerator::KeyState::RELEASED;
        return true;
    }
    NOTREACHED();
    return false;
  }
};

template <>
struct StructTraits<ui::mojom::AcceleratorDataView, ui::Accelerator> {
  static int32_t key_code(const ui::Accelerator& p) {
    return static_cast<int32_t>(p.key_code());
  }
  static ui::Accelerator::KeyState key_state(const ui::Accelerator& p) {
    return p.key_state();
  }
  static int32_t modifiers(const ui::Accelerator& p) { return p.modifiers(); }
  static bool Read(ui::mojom::AcceleratorDataView data, ui::Accelerator* out) {
    ui::Accelerator::KeyState key_state;
    if (!data.ReadKeyState(&key_state))
      return false;
    *out = ui::Accelerator(static_cast<ui::KeyboardCode>(data.key_code()),
                           data.modifiers());
    out->set_key_state(key_state);
    return true;
  }
};

}  // namespace mojo

#endif  // UI_BASE_ACCELERATORS_MOJO_ACCELERATOR_STRUCT_TRAITS_H_
