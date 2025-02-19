// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>

#include "base/macros.h"
#include "build/build_config.h"
#include "content/browser/frame_host/render_frame_host_impl.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/ax_event_notification_details.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/content_browser_test.h"
#include "content/public/test/content_browser_test_utils.h"
#include "content/shell/browser/shell.h"
#include "content/test/accessibility_browser_test_utils.h"
#include "ui/accessibility/ax_node.h"
#include "ui/accessibility/ax_tree.h"

namespace content {

class AccessibilityIpcErrorBrowserTest : public ContentBrowserTest {
 public:
  AccessibilityIpcErrorBrowserTest() {}

 protected:
  // Convenience method to get the value of a particular AXNode
  // attribute as a UTF-8 string.
  std::string GetAttr(const ui::AXNode* node,
                      const ui::AXStringAttribute attr) {
    const ui::AXNodeData& data = node->data();
    for (size_t i = 0; i < data.string_attributes.size(); ++i) {
      if (data.string_attributes[i].first == attr)
        return data.string_attributes[i].second;
    }
    return std::string();
  }

  DISALLOW_COPY_AND_ASSIGN(AccessibilityIpcErrorBrowserTest);
};

IN_PROC_BROWSER_TEST_F(AccessibilityIpcErrorBrowserTest,
                       ResetBrowserAccessibilityManager) {
  // Create a data url and load it.
  const char url_str[] =
      "data:text/html,"
      "<div aria-live='polite'>"
      "  <p id='p1'>Paragraph One</p>"
      "  <p id='p2'>Paragraph Two</p>"
      "</div>"
      "<button id='button'>Button</button>";
  GURL url(url_str);
  NavigateToURL(shell(), url);

  // Simulate a condition where the RFH can't create a
  // BrowserAccessibilityManager - like if there's no view.
  RenderFrameHostImpl* frame = static_cast<RenderFrameHostImpl*>(
      shell()->web_contents()->GetMainFrame());
  frame->set_no_create_browser_accessibility_manager_for_testing(true);
  ASSERT_EQ(nullptr, frame->GetOrCreateBrowserAccessibilityManager());

  {
    // Enable accessibility (passing ui::kAXModeComplete to
    // AccessibilityNotificationWaiter does this automatically) and wait for
    // the first event.
    AccessibilityNotificationWaiter waiter(shell()->web_contents(),
                                           ui::kAXModeComplete,
                                           ui::AX_EVENT_LAYOUT_COMPLETE);
    waiter.WaitForNotification();
  }

  // Make sure we still didn't create a BrowserAccessibilityManager.
  // This means that at least one accessibility IPC was lost.
  ASSERT_EQ(nullptr, frame->GetOrCreateBrowserAccessibilityManager());

  // Now create a BrowserAccessibilityManager, simulating what would happen
  // if the RFH's view is created now - but then disallow recreating the
  // BrowserAccessibilityManager so that we can test that this one gets
  // destroyed.
  frame->set_no_create_browser_accessibility_manager_for_testing(false);
  ASSERT_TRUE(frame->GetOrCreateBrowserAccessibilityManager() != nullptr);
  frame->set_no_create_browser_accessibility_manager_for_testing(true);

  {
    // Hide one of the elements on the page, and wait for an accessibility
    // notification triggered by the hide.
    AccessibilityNotificationWaiter waiter(shell()->web_contents(),
                                           ui::kAXModeComplete,
                                           ui::AX_EVENT_LIVE_REGION_CHANGED);
    ASSERT_TRUE(ExecuteScript(
        shell(), "document.getElementById('p1').style.display = 'none';"));
    waiter.WaitForNotification();
  }

  // Show that accessibility was reset because the frame doesn't have a
  // BrowserAccessibilityManager anymore.
  ASSERT_EQ(nullptr, frame->browser_accessibility_manager());

  // Finally, allow creating a new accessibility manager and
  // ensure that we didn't kill the renderer; we can still send it messages.
  frame->set_no_create_browser_accessibility_manager_for_testing(false);
  const ui::AXTree* tree = nullptr;
  {
    AccessibilityNotificationWaiter waiter(
        shell()->web_contents(), ui::kAXModeComplete, ui::AX_EVENT_FOCUS);
    ASSERT_TRUE(
        ExecuteScript(shell(), "document.getElementById('button').focus();"));
    waiter.WaitForNotification();
    tree = &waiter.GetAXTree();
  }

  // Get the accessibility tree, ensure it reflects the final state of the
  // document.
  const ui::AXNode* root = tree->root();

  // Use this for debugging if the test fails.
  VLOG(1) << tree->ToString();

  EXPECT_EQ(ui::AX_ROLE_ROOT_WEB_AREA, root->data().role);
  ASSERT_EQ(2, root->child_count());

  const ui::AXNode* live_region = root->ChildAtIndex(0);
  ASSERT_EQ(1, live_region->child_count());
  EXPECT_EQ(ui::AX_ROLE_GENERIC_CONTAINER, live_region->data().role);

  const ui::AXNode* para = live_region->ChildAtIndex(0);
  EXPECT_EQ(ui::AX_ROLE_PARAGRAPH, para->data().role);

  const ui::AXNode* button = root->ChildAtIndex(1);
  EXPECT_EQ(ui::AX_ROLE_BUTTON, button->data().role);
}

#if defined(OS_ANDROID)
// http://crbug.com/542704
#define MAYBE_MultipleBadAccessibilityIPCsKillsRenderer DISABLED_MultipleBadAccessibilityIPCsKillsRenderer
#else
#define MAYBE_MultipleBadAccessibilityIPCsKillsRenderer MultipleBadAccessibilityIPCsKillsRenderer
#endif
IN_PROC_BROWSER_TEST_F(AccessibilityIpcErrorBrowserTest,
                       MAYBE_MultipleBadAccessibilityIPCsKillsRenderer) {
  // Create a data url and load it.
  const char url_str[] =
      "data:text/html,"
      "<button id='button'>Button</button>";
  GURL url(url_str);
  NavigateToURL(shell(), url);
  RenderFrameHostImpl* frame = static_cast<RenderFrameHostImpl*>(
      shell()->web_contents()->GetMainFrame());

  {
    // Enable accessibility (passing ui::kAXModeComplete to
    // AccessibilityNotificationWaiter does this automatically) and wait for
    // the first event.
    AccessibilityNotificationWaiter waiter(shell()->web_contents(),
                                           ui::kAXModeComplete,
                                           ui::AX_EVENT_LAYOUT_COMPLETE);
    waiter.WaitForNotification();
  }

  // Construct a bad accessibility message that BrowserAccessibilityManager
  // will reject.
  std::vector<AXEventNotificationDetails> bad_accessibility_event_list;
  bad_accessibility_event_list.push_back(AXEventNotificationDetails());
  bad_accessibility_event_list[0].update.node_id_to_clear = -2;

  // We should be able to reset accessibility |max_iterations-1| times
  // (see render_frame_host_impl.cc - kMaxAccessibilityResets),
  // but the subsequent time the renderer should be killed.
  int max_iterations = RenderFrameHostImpl::kMaxAccessibilityResets;

  for (int iteration = 0; iteration < max_iterations; iteration++) {
    // Send the browser accessibility the bad message.
    BrowserAccessibilityManager* manager =
        frame->GetOrCreateBrowserAccessibilityManager();
    manager->OnAccessibilityEvents(bad_accessibility_event_list);

    // Now the frame should have deleted the BrowserAccessibilityManager.
    ASSERT_EQ(nullptr, frame->browser_accessibility_manager());

    if (iteration == max_iterations - 1)
      break;

    AccessibilityNotificationWaiter waiter(shell()->web_contents(),
                                           ui::kAXModeComplete,
                                           ui::AX_EVENT_LOAD_COMPLETE);
    waiter.WaitForNotification();
  }

  // Wait for the renderer to be killed.
  if (frame->IsRenderFrameLive()) {
    RenderProcessHostWatcher render_process_watcher(
        frame->GetProcess(), RenderProcessHostWatcher::WATCH_FOR_PROCESS_EXIT);
    render_process_watcher.Wait();
  }
  ASSERT_FALSE(frame->IsRenderFrameLive());
}

}  // namespace content
