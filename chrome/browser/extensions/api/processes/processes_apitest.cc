// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "chrome/browser/extensions/api/processes/processes_api.h"
#include "chrome/browser/extensions/extension_apitest.h"
#include "chrome/browser/task_manager/task_manager_interface.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_dialogs.h"
#include "chrome/browser/ui/browser_window.h"
#include "extensions/common/switches.h"
#include "extensions/test/extension_test_message_listener.h"

class ProcessesApiTest : public ExtensionApiTest {
 public:
  ProcessesApiTest() {}
  ~ProcessesApiTest() override {}

  int GetListenersCount() {
    return extensions::ProcessesAPI::Get(profile())->
        processes_event_router()->listeners_;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(ProcessesApiTest);
};


// This test is flaky. https://crbug.com/598445
IN_PROC_BROWSER_TEST_F(ProcessesApiTest, DISABLED_Processes) {
  ASSERT_TRUE(RunExtensionTest("processes/api")) << message_;
}

IN_PROC_BROWSER_TEST_F(ProcessesApiTest, ProcessesApiListeners) {
  EXPECT_EQ(0, GetListenersCount());

  // Load extension that adds a listener in background page
  ExtensionTestMessageListener listener1("ready", false /* will_reply */);
  const extensions::Extension* extension1 = LoadExtension(
      test_data_dir_.AppendASCII("processes").AppendASCII("onupdated"));
  ASSERT_TRUE(extension1);
  ASSERT_TRUE(listener1.WaitUntilSatisfied());

  // The memory refresh type of the task manager may or may not be enabled by
  // now depending on the presence of other task manager observers.
  // Ensure the listeners count has changed.
  EXPECT_EQ(1, GetListenersCount());

  // Load another extension that listen to the onUpdatedWithMemory.
  ExtensionTestMessageListener listener2("ready", false /* will_reply */);
  const extensions::Extension* extension2 = LoadExtension(
      test_data_dir_.AppendASCII("processes").AppendASCII(
          "onupdated_with_memory"));
  ASSERT_TRUE(extension2);
  ASSERT_TRUE(listener2.WaitUntilSatisfied());

  // The memory refresh type must be enabled now.
  const task_manager::TaskManagerInterface* task_manager =
      task_manager::TaskManagerInterface::GetTaskManager();
  EXPECT_EQ(2, GetListenersCount());
  EXPECT_TRUE(task_manager->IsResourceRefreshEnabled(
      task_manager::REFRESH_TYPE_MEMORY));

  // Unload the extensions and make sure the listeners count is updated.
  UnloadExtension(extension2->id());
  EXPECT_EQ(1, GetListenersCount());
  UnloadExtension(extension1->id());
  EXPECT_EQ(0, GetListenersCount());
}

IN_PROC_BROWSER_TEST_F(ProcessesApiTest, CannotTerminateBrowserProcess) {
  ASSERT_TRUE(RunExtensionTest("processes/terminate-browser-process"))
      << message_;
}
