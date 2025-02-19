// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_TASK_MANAGER_PROVIDERS_WEB_CONTENTS_DEVTOOLS_TASK_H_
#define CHROME_BROWSER_TASK_MANAGER_PROVIDERS_WEB_CONTENTS_DEVTOOLS_TASK_H_

#include "base/macros.h"
#include "chrome/browser/task_manager/providers/web_contents/tab_contents_task.h"

namespace task_manager {

// Defines a task manager representation of the developer tools WebContents.
class DevToolsTask : public TabContentsTask {
 public:
  explicit DevToolsTask(content::WebContents* web_contents);
  ~DevToolsTask() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(DevToolsTask);
};

}  // namespace task_manager

#endif  // CHROME_BROWSER_TASK_MANAGER_PROVIDERS_WEB_CONTENTS_DEVTOOLS_TASK_H_
