// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_PHYSICAL_WEB_PHYSICAL_WEB_UI_H_
#define CHROME_BROWSER_UI_WEBUI_PHYSICAL_WEB_PHYSICAL_WEB_UI_H_

#include "base/macros.h"
#include "content/public/browser/web_ui_controller.h"

class PhysicalWebUI : public content::WebUIController {
 public:
  explicit PhysicalWebUI(content::WebUI* web_ui);
  ~PhysicalWebUI() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(PhysicalWebUI);
};

#endif  // CHROME_BROWSER_UI_WEBUI_PHYSICAL_WEB_PHYSICAL_WEB_UI_H_
