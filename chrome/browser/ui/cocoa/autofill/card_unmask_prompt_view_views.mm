// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/cocoa/autofill/card_unmask_prompt_view_bridge.h"
#include "chrome/browser/ui/views/autofill/card_unmask_prompt_views.h"
#include "ui/base/material_design/material_design_controller.h"

namespace autofill {

CardUnmaskPromptView* CreateCardUnmaskPromptView(
    CardUnmaskPromptController* controller,
    content::WebContents* web_contents) {
  if (ui::MaterialDesignController::IsSecondaryUiMaterial())
    return new CardUnmaskPromptViews(controller, web_contents);
  return new CardUnmaskPromptViewBridge(controller, web_contents);
}

}
