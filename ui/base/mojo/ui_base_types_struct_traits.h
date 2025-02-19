// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_MOJO_UI_BASE_TYPES_STRUCT_TRAITS_H_
#define UI_BASE_MOJO_UI_BASE_TYPES_STRUCT_TRAITS_H_

#include "mojo/public/cpp/bindings/enum_traits.h"
#include "ui/base/mojo/ui_base_types.mojom.h"
#include "ui/base/ui_base_types.h"

namespace mojo {

template <>
struct EnumTraits<ui::mojom::ModalType, ui::ModalType> {
  static ui::mojom::ModalType ToMojom(ui::ModalType modal_type) {
    switch (modal_type) {
      case ui::MODAL_TYPE_NONE:
        return ui::mojom::ModalType::NONE;
      case ui::MODAL_TYPE_WINDOW:
        return ui::mojom::ModalType::WINDOW;
      case ui::MODAL_TYPE_CHILD:
        return ui::mojom::ModalType::CHILD;
      case ui::MODAL_TYPE_SYSTEM:
        return ui::mojom::ModalType::SYSTEM;
      default:
        NOTREACHED();
        return ui::mojom::ModalType::NONE;
    }
  }

  static bool FromMojom(ui::mojom::ModalType modal_type, ui::ModalType* out) {
    switch (modal_type) {
      case ui::mojom::ModalType::NONE:
        *out = ui::MODAL_TYPE_NONE;
        return true;
      case ui::mojom::ModalType::WINDOW:
        *out = ui::MODAL_TYPE_WINDOW;
        return true;
      case ui::mojom::ModalType::CHILD:
        *out = ui::MODAL_TYPE_CHILD;
        return true;
      case ui::mojom::ModalType::SYSTEM:
        *out = ui::MODAL_TYPE_SYSTEM;
        return true;
      default:
        NOTREACHED();
        return false;
    }
  }
};

}  // namespace mojo

#endif  // UI_BASE_MOJO_WINDOW_OPEN_DISPOSITION_STRUCT_TRAITS_H_
