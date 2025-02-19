// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/layout/LayoutThemeDefault.h"

namespace blink {
namespace {

// Fuchsia is headless-only for now, so no native themes are applied.
// TODO(fuchsia): Implement this when we enable the UI. (crbug.com/750946)
class LayoutThemeFuchsia : public LayoutThemeDefault {
 public:
  static RefPtr<LayoutTheme> Create() {
    return AdoptRef(new LayoutThemeFuchsia());
  }
};

}  // namespace

LayoutTheme& LayoutTheme::NativeTheme() {
  DEFINE_STATIC_REF(LayoutTheme, layout_theme, (LayoutThemeFuchsia::Create()));
  return *layout_theme;
}

}  // namespace blink
