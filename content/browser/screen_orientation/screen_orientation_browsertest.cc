// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdlib.h>

#include "base/command_line.h"
#include "base/macros.h"
#include "base/strings/stringprintf.h"
#include "build/build_config.h"
#include "content/browser/frame_host/frame_tree.h"
#include "content/browser/frame_host/render_frame_host_impl.h"
#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/common/page_messages.h"
#include "content/common/view_messages.h"
#include "content/public/browser/render_widget_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_switches.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/content_browser_test.h"
#include "content/public/test/content_browser_test_utils.h"
#include "content/public/test/test_navigation_observer.h"
#include "content/public/test/test_utils.h"
#include "content/shell/browser/shell.h"
#include "content/shell/common/shell_switches.h"
#include "net/dns/mock_host_resolver.h"
#include "ui/compositor/compositor_switches.h"
#include "ui/display/screen.h"

namespace content {

class ScreenOrientationBrowserTest : public ContentBrowserTest  {
 public:
  ScreenOrientationBrowserTest() {
  }

  WebContentsImpl* web_contents() {
    return static_cast<WebContentsImpl*>(shell()->web_contents());
  }

 protected:
  void SendFakeScreenOrientation(unsigned angle, const std::string& strType) {
    RenderWidgetHost* main_frame_rwh =
        web_contents()->GetMainFrame()->GetRenderWidgetHost();
    ScreenInfo screen_info;
    main_frame_rwh->GetScreenInfo(&screen_info);
    screen_info.orientation_angle = angle;

    ScreenOrientationValues type = SCREEN_ORIENTATION_VALUES_DEFAULT;
    if (strType == "portrait-primary") {
      type = SCREEN_ORIENTATION_VALUES_PORTRAIT_PRIMARY;
    } else if (strType == "portrait-secondary") {
      type = SCREEN_ORIENTATION_VALUES_PORTRAIT_SECONDARY;
    } else if (strType == "landscape-primary") {
      type = SCREEN_ORIENTATION_VALUES_LANDSCAPE_PRIMARY;
    } else if (strType == "landscape-secondary") {
      type = SCREEN_ORIENTATION_VALUES_LANDSCAPE_SECONDARY;
    }
    ASSERT_NE(SCREEN_ORIENTATION_VALUES_DEFAULT, type);
    screen_info.orientation_type = type;

    ResizeParams params;
    params.screen_info = screen_info;
    params.new_size = gfx::Size(0, 0);
    params.physical_backing_size = gfx::Size(300, 300);
    params.top_controls_height = 0.f;
    params.browser_controls_shrink_blink_size = false;
    params.is_fullscreen_granted = false;

    std::set<RenderWidgetHost*> rwhs;
    for (RenderFrameHost* rfh : web_contents()->GetAllFrames()) {
      if (rfh == web_contents()->GetMainFrame())
        continue;

      rwhs.insert(static_cast<RenderFrameHostImpl*>(rfh)
                      ->frame_tree_node()
                      ->render_manager()
                      ->GetRenderWidgetHostView()
                      ->GetRenderWidgetHost());
    }

    // This simulates what the browser process does when the screen orientation
    // is changed:
    // 1. The top-level frame is resized and a ViweMsg_Resize is sent to the
    // top-level frame.
    main_frame_rwh->Send(
        new ViewMsg_Resize(main_frame_rwh->GetRoutingID(), params));

    // 2. The WebContents sends a PageMsg_UpdateScreenInfo to all the renderers
    // involved in the FrameTree.
    web_contents()->GetFrameTree()->root()->render_manager()->SendPageMessage(
        new PageMsg_UpdateScreenInfo(MSG_ROUTING_NONE, screen_info), nullptr);

    // 3. When the top-level frame gets the ViewMsg_Resize, it'll dispatch a
    // FrameHostMsg_FrameRectsChanged IPC that causes the remote subframes to
    // receive the ViewMsg_Resize from the browser.
    for (auto* rwh : rwhs)
      rwh->Send(new ViewMsg_Resize(rwh->GetRoutingID(), params));
  }

  int GetOrientationAngle() {
    int angle;
    ExecuteScriptAndGetValue(shell()->web_contents()->GetMainFrame(),
                             "screen.orientation.angle")->GetAsInteger(&angle);
    return angle;
  }

  std::string GetOrientationType() {
    std::string type;
    ExecuteScriptAndGetValue(shell()->web_contents()->GetMainFrame(),
                             "screen.orientation.type")->GetAsString(&type);
    return type;
  }

  bool ScreenOrientationSupported() {
    bool support;
    ExecuteScriptAndGetValue(shell()->web_contents()->GetMainFrame(),
                             "'orientation' in screen")->GetAsBoolean(&support);
    return support;
  }

  bool WindowOrientationSupported() {
    bool support;
    ExecuteScriptAndGetValue(shell()->web_contents()->GetMainFrame(),
                             "'orientation' in window")->GetAsBoolean(&support);
    return support;
  }

  int GetWindowOrientationAngle() {
    int angle;
    ExecuteScriptAndGetValue(shell()->web_contents()->GetMainFrame(),
                             "window.orientation")->GetAsInteger(&angle);
    return angle;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(ScreenOrientationBrowserTest);
};

class ScreenOrientationOOPIFBrowserTest : public ScreenOrientationBrowserTest {
 public:
  ScreenOrientationOOPIFBrowserTest() {}

  void SetUpCommandLine(base::CommandLine* command_line) override {
    IsolateAllSitesForTesting(command_line);
  }

  void SetUpOnMainThread() override {
    host_resolver()->AddRule("*", "127.0.0.1");
    SetupCrossSiteRedirector(embedded_test_server());
    ASSERT_TRUE(embedded_test_server()->Start());
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(ScreenOrientationOOPIFBrowserTest);
};

// This test doesn't work on MacOS X but the reason is mostly because it is not
// used Aura. It could be set as !defined(OS_MACOSX) but the rule below will
// actually support MacOS X if and when it switches to Aura.
#if defined(USE_AURA) || defined(OS_ANDROID)
// Flaky on Chrome OS: http://crbug.com/468259
#if defined(OS_CHROMEOS)
#define MAYBE_ScreenOrientationChange DISABLED_ScreenOrientationChange
#else
#define MAYBE_ScreenOrientationChange ScreenOrientationChange
#endif
IN_PROC_BROWSER_TEST_F(ScreenOrientationBrowserTest,
                       MAYBE_ScreenOrientationChange) {
  std::string types[] = { "portrait-primary",
                          "portrait-secondary",
                          "landscape-primary",
                          "landscape-secondary" };
  GURL test_url = GetTestUrl("screen_orientation",
                             "screen_orientation_screenorientationchange.html");

  TestNavigationObserver navigation_observer(shell()->web_contents(), 1);
  shell()->LoadURL(test_url);
  navigation_observer.Wait();
  WaitForResizeComplete(shell()->web_contents());

  int angle = GetOrientationAngle();

  for (int i = 0; i < 4; ++i) {
    angle = (angle + 90) % 360;
    SendFakeScreenOrientation(angle, types[i]);

    TestNavigationObserver navigation_observer(shell()->web_contents());
    navigation_observer.Wait();
    EXPECT_EQ(angle, GetOrientationAngle());
    EXPECT_EQ(types[i], GetOrientationType());
  }
}
#endif // defined(USE_AURA) || defined(OS_ANDROID)

// Flaky on Chrome OS: http://crbug.com/468259
#if defined(OS_CHROMEOS)
#define MAYBE_WindowOrientationChange DISABLED_WindowOrientationChange
#else
#define MAYBE_WindowOrientationChange WindowOrientationChange
#endif
IN_PROC_BROWSER_TEST_F(ScreenOrientationBrowserTest,
                       MAYBE_WindowOrientationChange) {
  GURL test_url = GetTestUrl("screen_orientation",
                             "screen_orientation_windoworientationchange.html");

  TestNavigationObserver navigation_observer(shell()->web_contents(), 1);
  shell()->LoadURL(test_url);
  navigation_observer.Wait();
#if USE_AURA || defined(OS_ANDROID)
  WaitForResizeComplete(shell()->web_contents());
#endif  // USE_AURA || defined(OS_ANDROID)

  if (!WindowOrientationSupported())
    return;

  int angle = GetWindowOrientationAngle();

  for (int i = 0; i < 4; ++i) {
    angle = (angle + 90) % 360;
    SendFakeScreenOrientation(angle, "portrait-primary");

    TestNavigationObserver navigation_observer(shell()->web_contents(), 1);
    navigation_observer.Wait();
    EXPECT_EQ(angle == 270 ? -90 : angle, GetWindowOrientationAngle());
  }
}

// LockSmoke test seems to have become flaky on all non-ChromeOS platforms.
// The cause is unfortunately unknown. See https://crbug.com/448876
// Chromium Android does not support fullscreen
IN_PROC_BROWSER_TEST_F(ScreenOrientationBrowserTest, DISABLED_LockSmoke) {
  GURL test_url = GetTestUrl("screen_orientation",
                             "screen_orientation_lock_smoke.html");

  TestNavigationObserver navigation_observer(shell()->web_contents(), 2);
  shell()->LoadURL(test_url);

  navigation_observer.Wait();
#if USE_AURA || defined(OS_ANDROID)
  WaitForResizeComplete(shell()->web_contents());
#endif  // USE_AURA || defined(OS_ANDROID)

  std::string expected =
#if defined(OS_ANDROID)
      "SecurityError"; // WebContents need to be fullscreen.
#else
      "NotSupportedError"; // Locking isn't supported.
#endif

  EXPECT_EQ(expected, shell()->web_contents()->GetLastCommittedURL().ref());
}

// Check that using screen orientation after a frame is detached doesn't crash
// the renderer process.
// This could be a LayoutTest if they were not using a mock screen orientation
// controller.
IN_PROC_BROWSER_TEST_F(ScreenOrientationBrowserTest, CrashTest_UseAfterDetach) {
  GURL test_url = GetTestUrl("screen_orientation",
                             "screen_orientation_use_after_detach.html");

  TestNavigationObserver navigation_observer(shell()->web_contents(), 2);
  shell()->LoadURL(test_url);

  navigation_observer.Wait();

  // This is a success if the renderer process did not crash, thus, we end up
  // here.
}

#if defined(OS_ANDROID)
class ScreenOrientationLockDisabledBrowserTest : public ContentBrowserTest  {
 public:
  ScreenOrientationLockDisabledBrowserTest() {}
  ~ScreenOrientationLockDisabledBrowserTest() override {}

  void SetUpCommandLine(base::CommandLine* command_line) override {
    command_line->AppendSwitch(switches::kDisableScreenOrientationLock);
  }
};

// Check that when --disable-screen-orientation-lock is passed to the command
// line, screen.orientation.lock() correctly reports to not be supported.
IN_PROC_BROWSER_TEST_F(ScreenOrientationLockDisabledBrowserTest,
    DISABLED_NotSupported) {
  GURL test_url = GetTestUrl("screen_orientation",
                             "screen_orientation_lock_disabled.html");

  TestNavigationObserver navigation_observer(shell()->web_contents(), 1);
  shell()->LoadURL(test_url);
  navigation_observer.Wait();

  {
    ASSERT_TRUE(ExecuteScript(shell(), "run();"));

    TestNavigationObserver navigation_observer(shell()->web_contents(), 1);
    navigation_observer.Wait();
    EXPECT_EQ("NotSupportedError",
              shell()->web_contents()->GetLastCommittedURL().ref());
  }
}
#endif // defined(OS_ANDROID)

IN_PROC_BROWSER_TEST_F(ScreenOrientationOOPIFBrowserTest, ScreenOrientation) {
  GURL main_url(embedded_test_server()->GetURL(
      "a.com", "/cross_site_iframe_factory.html?a(b)"));
  EXPECT_TRUE(NavigateToURL(shell(), main_url));
#if USE_AURA || defined(OS_ANDROID)
  WaitForResizeComplete(shell()->web_contents());
#endif  // USE_AURA || defined(OS_ANDROID)

  std::string types[] = {"portrait-primary", "portrait-secondary",
                         "landscape-primary", "landscape-secondary"};

  int angle = GetOrientationAngle();

  FrameTreeNode* root = web_contents()->GetFrameTree()->root();
  FrameTreeNode* child = root->child_at(0);
  MainThreadFrameObserver root_observer(
      root->current_frame_host()->GetRenderWidgetHost());
  MainThreadFrameObserver child_observer(
      child->current_frame_host()->GetRenderWidgetHost());
  for (int i = 0; i < 4; ++i) {
    angle = (angle + 90) % 360;
    SendFakeScreenOrientation(angle, types[i]);

    root_observer.Wait();
    child_observer.Wait();

    int orientation_angle;
    std::string orientation_type;

    EXPECT_TRUE(ExecuteScriptAndExtractInt(
        root->current_frame_host(),
        "window.domAutomationController.send(screen.orientation.angle)",
        &orientation_angle));
    EXPECT_EQ(angle, orientation_angle);
    EXPECT_TRUE(ExecuteScriptAndExtractInt(
        child->current_frame_host(),
        "window.domAutomationController.send(screen.orientation.angle)",
        &orientation_angle));
    EXPECT_EQ(angle, orientation_angle);

    EXPECT_TRUE(ExecuteScriptAndExtractString(
        root->current_frame_host(),
        "window.domAutomationController.send(screen.orientation.type)",
        &orientation_type));
    EXPECT_EQ(types[i], orientation_type);
    EXPECT_TRUE(ExecuteScriptAndExtractString(
        child->current_frame_host(),
        "window.domAutomationController.send(screen.orientation.type)",
        &orientation_type));
    EXPECT_EQ(types[i], orientation_type);
  }
}

#ifdef OS_ANDROID
// This test is disabled because th trybots run in system portrait lock, which
// prevents the test from changing the screen orientation.
IN_PROC_BROWSER_TEST_F(ScreenOrientationOOPIFBrowserTest,
                       DISABLED_ScreenOrientationLock) {
  GURL main_url(embedded_test_server()->GetURL(
      "a.com", "/cross_site_iframe_factory.html?a(b)"));
  EXPECT_TRUE(NavigateToURL(shell(), main_url));
  WaitForResizeComplete(shell()->web_contents());

  const char* types[] = {"portrait-primary", "portrait-secondary",
                         "landscape-primary", "landscape-secondary"};

  FrameTreeNode* root = web_contents()->GetFrameTree()->root();
  FrameTreeNode* child = root->child_at(0);
  RenderFrameHostImpl* frames[] = {root->current_frame_host(),
                                   child->current_frame_host()};

  EXPECT_TRUE(ExecuteScript(root->current_frame_host(),
                            "document.body.webkitRequestFullscreen()"));
  for (const char* type : types) {
    std::string script =
        base::StringPrintf("screen.orientation.lock('%s')", type);
    EXPECT_TRUE(ExecuteScript(child->current_frame_host(), script));

    for (auto* frame : frames) {
      std::string orientation_type;
      while (type != orientation_type) {
        EXPECT_TRUE(ExecuteScriptAndExtractString(
            frame,
            "window.domAutomationController.send(screen.orientation.type)",
            &orientation_type));
      }
    }

    EXPECT_TRUE(ExecuteScript(child->current_frame_host(),
                              "screen.orientation.unlock()"));
  }
}
#endif  // OS_ANDROID

} // namespace content
