// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/pointer_lock_browsertest.h"

#include "content/browser/frame_host/frame_tree.h"
#include "content/browser/renderer_host/render_widget_host_impl.h"
#include "content/browser/renderer_host/render_widget_host_input_event_router.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/content_browser_test.h"
#include "content/public/test/content_browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "content/shell/browser/shell.h"
#include "content/test/content_browser_test_utils_internal.h"
#include "net/dns/mock_host_resolver.h"
#include "net/test/embedded_test_server/embedded_test_server.h"

#ifdef USE_AURA
#include "content/browser/renderer_host/render_widget_host_view_aura.h"
#include "content/browser/web_contents/web_contents_view_aura.h"
#endif  // USE_AURA

namespace content {

namespace {

#ifdef USE_AURA
class MockRenderWidgetHostView : public RenderWidgetHostViewAura {
 public:
  MockRenderWidgetHostView(RenderWidgetHost* host, bool is_guest_view_hack)
      : RenderWidgetHostViewAura(host, is_guest_view_hack),
        host_(RenderWidgetHostImpl::From(host)) {}
  ~MockRenderWidgetHostView() override {
    if (mouse_locked_)
      UnlockMouse();
  }

  bool LockMouse() override {
    mouse_locked_ = true;
    return true;
  }

  void UnlockMouse() override {
    host_->LostMouseLock();
    mouse_locked_ = false;
  }

  bool IsMouseLocked() override { return mouse_locked_; }

  bool HasFocus() const override { return true; }

  void OnWindowFocused(aura::Window* gained_focus,
                       aura::Window* lost_focus) override {
    // Ignore window focus events.
  }

  RenderWidgetHostImpl* host_;
};
#endif  // USE_AURA

}  // namespace

class MockPointerLockWebContentsDelegate : public WebContentsDelegate {
 public:
  MockPointerLockWebContentsDelegate() {}
  ~MockPointerLockWebContentsDelegate() override {}

  void RequestToLockMouse(WebContents* web_contents,
                          bool user_gesture,
                          bool last_unlocked_by_target) override {
    web_contents->GotResponseToLockMouseRequest(true);
  }

  void LostMouseLock() override {}
};

#ifdef USE_AURA
void InstallCreateHooksForPointerLockBrowserTests() {
  WebContentsViewAura::InstallCreateHookForTests(
      [](RenderWidgetHost* host,
         bool is_guest_view_hack) -> RenderWidgetHostViewAura* {
        return new MockRenderWidgetHostView(host, is_guest_view_hack);
      });
}
#endif  // USE_AURA

class PointerLockBrowserTest : public ContentBrowserTest {
 public:
  PointerLockBrowserTest() {}

 protected:
  void SetUpCommandLine(base::CommandLine* command_line) override {
    IsolateAllSitesForTesting(command_line);
  }

  void SetUp() override {
    InstallCreateHooksForPointerLockBrowserTests();
    ContentBrowserTest::SetUp();
  }

  void SetUpOnMainThread() override {
    host_resolver()->AddRule("*", "127.0.0.1");
    SetupCrossSiteRedirector(embedded_test_server());
    ASSERT_TRUE(embedded_test_server()->Start());

    web_contents()->SetDelegate(&web_contents_delegate_);
  }

  WebContentsImpl* web_contents() const {
    return static_cast<WebContentsImpl*>(shell()->web_contents());
  }

 private:
  MockPointerLockWebContentsDelegate web_contents_delegate_;
};

IN_PROC_BROWSER_TEST_F(PointerLockBrowserTest, PointerLock) {
  GURL main_url(embedded_test_server()->GetURL(
      "a.com", "/cross_site_iframe_factory.html?a(b)"));
  EXPECT_TRUE(NavigateToURL(shell(), main_url));

  FrameTreeNode* root = web_contents()->GetFrameTree()->root();
  FrameTreeNode* child = root->child_at(0);

  // Request a pointer lock on the root frame's body.
  EXPECT_TRUE(ExecuteScript(root, "document.body.requestPointerLock()"));

  // Root frame should have been granted pointer lock.
  bool locked = false;
  EXPECT_TRUE(ExecuteScriptAndExtractBool(root,
                                          "window.domAutomationController.send("
                                          "document.pointerLockElement == "
                                          "document.body);",
                                          &locked));
  EXPECT_TRUE(locked);

  // Request a pointer lock on the child frame's body.
  EXPECT_TRUE(ExecuteScript(child, "document.body.requestPointerLock()"));

  // Child frame should not be granted pointer lock since the root frame has it.
  EXPECT_TRUE(ExecuteScriptAndExtractBool(child,
                                          "window.domAutomationController.send("
                                          "document.pointerLockElement == "
                                          "document.body);",
                                          &locked));
  EXPECT_FALSE(locked);

  // Release pointer lock on root frame.
  EXPECT_TRUE(ExecuteScript(root, "document.exitPointerLock()"));

  // Request a pointer lock on the child frame's body.
  EXPECT_TRUE(ExecuteScript(child, "document.body.requestPointerLock()"));

  // Child frame should have been granted pointer lock.
  EXPECT_TRUE(ExecuteScriptAndExtractBool(child,
                                          "window.domAutomationController.send("
                                          "document.pointerLockElement == "
                                          "document.body);",
                                          &locked));
  EXPECT_TRUE(locked);
}

IN_PROC_BROWSER_TEST_F(PointerLockBrowserTest, PointerLockEventRouting) {
  GURL main_url(embedded_test_server()->GetURL(
      "a.com", "/cross_site_iframe_factory.html?a(b)"));
  EXPECT_TRUE(NavigateToURL(shell(), main_url));

  FrameTreeNode* root = web_contents()->GetFrameTree()->root();
  FrameTreeNode* child = root->child_at(0);
  RenderWidgetHostInputEventRouter* router =
      web_contents()->GetInputEventRouter();
  RenderWidgetHostViewBase* root_view = static_cast<RenderWidgetHostViewBase*>(
      root->current_frame_host()->GetView());
  RenderWidgetHostViewBase* child_view = static_cast<RenderWidgetHostViewBase*>(
      child->current_frame_host()->GetView());

  // Request a pointer lock on the root frame's body.
  EXPECT_TRUE(ExecuteScript(root, "document.body.requestPointerLock()"));

  // Root frame should have been granted pointer lock.
  bool locked = false;
  EXPECT_TRUE(ExecuteScriptAndExtractBool(root,
                                          "window.domAutomationController.send("
                                          "document.pointerLockElement == "
                                          "document.body);",
                                          &locked));
  EXPECT_TRUE(locked);

  // Add a mouse move event listener to the root frame.
  EXPECT_TRUE(ExecuteScript(
      root,
      "var x; var y; var mX; var mY; document.addEventListener('mousemove', "
      "function(e) {x = e.x; y = e.y; mX = e.movementX; mY = e.movementY;});"));

  blink::WebMouseEvent mouse_event(blink::WebInputEvent::kMouseMove,
                                   blink::WebInputEvent::kNoModifiers,
                                   blink::WebInputEvent::kTimeStampForTesting);
  mouse_event.SetPositionInWidget(10, 11);
  mouse_event.movement_x = 12;
  mouse_event.movement_y = 13;
  router->RouteMouseEvent(root_view, &mouse_event, ui::LatencyInfo());

  // Make sure that the renderer handled the input event.
  MainThreadFrameObserver root_observer(root_view->GetRenderWidgetHost());
  root_observer.Wait();

  int x, y, movementX, movementY;
  EXPECT_TRUE(ExecuteScriptAndExtractInt(
      root, "window.domAutomationController.send(x);", &x));
  EXPECT_TRUE(ExecuteScriptAndExtractInt(
      root, "window.domAutomationController.send(y);", &y));
  EXPECT_TRUE(ExecuteScriptAndExtractInt(
      root, "window.domAutomationController.send(mX);", &movementX));
  EXPECT_TRUE(ExecuteScriptAndExtractInt(
      root, "window.domAutomationController.send(mY);", &movementY));
  EXPECT_EQ(10, x);
  EXPECT_EQ(11, y);
  EXPECT_EQ(12, movementX);
  EXPECT_EQ(13, movementY);

  // Release pointer lock on root frame.
  EXPECT_TRUE(ExecuteScript(root, "document.exitPointerLock()"));

  // Request a pointer lock on the child frame's body.
  EXPECT_TRUE(ExecuteScript(child, "document.body.requestPointerLock()"));

  // Child frame should have been granted pointer lock.
  EXPECT_TRUE(ExecuteScriptAndExtractBool(child,
                                          "window.domAutomationController.send("
                                          "document.pointerLockElement == "
                                          "document.body);",
                                          &locked));
  EXPECT_TRUE(locked);

  // Add a mouse move event listener to the child frame.
  EXPECT_TRUE(ExecuteScript(
      child,
      "var x; var y; var mX; var mY; document.addEventListener('mousemove', "
      "function(e) {x = e.x; y = e.y; mX = e.movementX; mY = e.movementY;});"));

  gfx::Point transformed_point;
  root_view->TransformPointToCoordSpaceForView(gfx::Point(0, 0), child_view,
                                               &transformed_point);

  mouse_event.SetPositionInWidget(-transformed_point.x() + 14,
                                  -transformed_point.y() + 15);
  mouse_event.movement_x = 16;
  mouse_event.movement_y = 17;
  // We use root_view intentionally as the RenderWidgetHostInputEventRouter is
  // responsible for correctly routing the event to the child frame.
  router->RouteMouseEvent(root_view, &mouse_event, ui::LatencyInfo());

  // Make sure that the renderer handled the input event.
  MainThreadFrameObserver child_observer(child_view->GetRenderWidgetHost());
  child_observer.Wait();

  EXPECT_TRUE(ExecuteScriptAndExtractInt(
      child, "window.domAutomationController.send(x);", &x));
  EXPECT_TRUE(ExecuteScriptAndExtractInt(
      child, "window.domAutomationController.send(y);", &y));
  EXPECT_TRUE(ExecuteScriptAndExtractInt(
      child, "window.domAutomationController.send(mX);", &movementX));
  EXPECT_TRUE(ExecuteScriptAndExtractInt(
      child, "window.domAutomationController.send(mY);", &movementY));
  EXPECT_EQ(14, x);
  EXPECT_EQ(15, y);
  EXPECT_EQ(16, movementX);
  EXPECT_EQ(17, movementY);
}

// Tests that the browser will not unlock the pointer if a RenderWidgetHostView
// that doesn't hold the pointer lock is destroyed.
IN_PROC_BROWSER_TEST_F(PointerLockBrowserTest, PointerLockChildFrameDetached) {
  GURL main_url(embedded_test_server()->GetURL(
      "a.com", "/cross_site_iframe_factory.html?a(b)"));
  EXPECT_TRUE(NavigateToURL(shell(), main_url));

  FrameTreeNode* root = web_contents()->GetFrameTree()->root();

  // Request a pointer lock on the root frame's body.
  EXPECT_TRUE(ExecuteScript(root, "document.body.requestPointerLock()"));

  // Root frame should have been granted pointer lock.
  bool locked = false;
  EXPECT_TRUE(ExecuteScriptAndExtractBool(root,
                                          "window.domAutomationController.send("
                                          "document.pointerLockElement == "
                                          "document.body);",
                                          &locked));
  EXPECT_TRUE(locked);

  // Root (platform) RenderWidgetHostView should have the pointer locked.
  EXPECT_TRUE(root->current_frame_host()->GetView()->IsMouseLocked());
  EXPECT_EQ(root->current_frame_host()->GetRenderWidgetHost(),
            web_contents()->GetMouseLockWidget());

  // Detach the child frame.
  EXPECT_TRUE(ExecuteScript(root, "document.querySelector('iframe').remove()"));

  // Root (platform) RenderWidgetHostView should still have the pointer locked.
  EXPECT_TRUE(root->current_frame_host()->GetView()->IsMouseLocked());
  EXPECT_EQ(root->current_frame_host()->GetRenderWidgetHost(),
            web_contents()->GetMouseLockWidget());
}

IN_PROC_BROWSER_TEST_F(PointerLockBrowserTest, PointerLockWheelEventRouting) {
  GURL main_url(embedded_test_server()->GetURL(
      "a.com", "/cross_site_iframe_factory.html?a(b)"));
  EXPECT_TRUE(NavigateToURL(shell(), main_url));

  FrameTreeNode* root = web_contents()->GetFrameTree()->root();
  FrameTreeNode* child = root->child_at(0);
  RenderWidgetHostInputEventRouter* router =
      web_contents()->GetInputEventRouter();
  RenderWidgetHostViewBase* root_view = static_cast<RenderWidgetHostViewBase*>(
      root->current_frame_host()->GetView());
  RenderWidgetHostViewBase* child_view = static_cast<RenderWidgetHostViewBase*>(
      child->current_frame_host()->GetView());

  // Request a pointer lock on the root frame's body.
  EXPECT_TRUE(ExecuteScript(root, "document.body.requestPointerLock()"));

  // Root frame should have been granted pointer lock.
  bool locked = false;
  EXPECT_TRUE(ExecuteScriptAndExtractBool(root,
                                          "window.domAutomationController.send("
                                          "document.pointerLockElement == "
                                          "document.body);",
                                          &locked));
  EXPECT_TRUE(locked);

  // Add a mouse move wheel event listener to the root frame.
  EXPECT_TRUE(ExecuteScript(
      root,
      "var x; var y; var mX; var mY; document.addEventListener('mousewheel', "
      "function(e) {x = e.x; y = e.y; dX = e.deltaX; dY = e.deltaY;});"));
  MainThreadFrameObserver root_observer(root_view->GetRenderWidgetHost());
  root_observer.Wait();

  blink::WebMouseWheelEvent wheel_event(
      blink::WebInputEvent::kMouseWheel, blink::WebInputEvent::kNoModifiers,
      blink::WebInputEvent::kTimeStampForTesting);
  wheel_event.SetPositionInWidget(10, 11);
  wheel_event.delta_x = -12;
  wheel_event.delta_y = -13;
  wheel_event.phase = blink::WebMouseWheelEvent::kPhaseBegan;
  router->RouteMouseWheelEvent(root_view, &wheel_event, ui::LatencyInfo());

  // Make sure that the renderer handled the input event.
  root_observer.Wait();

  if (root_view->wheel_scroll_latching_enabled()) {
    // When wheel scroll latching is enabled all wheel events during a scroll
    // sequence will be sent to a single target. Send a wheel end event to the
    // current target before sending wheel events to a new target.
    wheel_event.delta_x = 0;
    wheel_event.delta_y = 0;
    wheel_event.phase = blink::WebMouseWheelEvent::kPhaseEnded;
    router->RouteMouseWheelEvent(root_view, &wheel_event, ui::LatencyInfo());

    // Make sure that the renderer handled the input event.
    root_observer.Wait();
  }

  int x, y, deltaX, deltaY;
  EXPECT_TRUE(ExecuteScriptAndExtractInt(
      root, "window.domAutomationController.send(x);", &x));
  EXPECT_TRUE(ExecuteScriptAndExtractInt(
      root, "window.domAutomationController.send(y);", &y));
  EXPECT_TRUE(ExecuteScriptAndExtractInt(
      root, "window.domAutomationController.send(dX);", &deltaX));
  EXPECT_TRUE(ExecuteScriptAndExtractInt(
      root, "window.domAutomationController.send(dY);", &deltaY));
  EXPECT_EQ(10, x);
  EXPECT_EQ(11, y);
  EXPECT_EQ(12, deltaX);
  EXPECT_EQ(13, deltaY);

  // Release pointer lock on root frame.
  EXPECT_TRUE(ExecuteScript(root, "document.exitPointerLock()"));

  // Request a pointer lock on the child frame's body.
  EXPECT_TRUE(ExecuteScript(child, "document.body.requestPointerLock()"));

  // Child frame should have been granted pointer lock.
  EXPECT_TRUE(ExecuteScriptAndExtractBool(child,
                                          "window.domAutomationController.send("
                                          "document.pointerLockElement == "
                                          "document.body);",
                                          &locked));
  EXPECT_TRUE(locked);

  // Add a mouse move event listener to the child frame.
  EXPECT_TRUE(ExecuteScript(
      child,
      "var x; var y; var mX; var mY; document.addEventListener('mousewheel', "
      "function(e) {x = e.x; y = e.y; dX = e.deltaX; dY = e.deltaY;});"));
  MainThreadFrameObserver child_observer(child_view->GetRenderWidgetHost());
  child_observer.Wait();

  gfx::Point transformed_point;
  root_view->TransformPointToCoordSpaceForView(gfx::Point(0, 0), child_view,
                                               &transformed_point);

  wheel_event.SetPositionInWidget(-transformed_point.x() + 14,
                                  -transformed_point.y() + 15);
  wheel_event.delta_x = -16;
  wheel_event.delta_y = -17;
  if (root_view->wheel_scroll_latching_enabled())
    wheel_event.phase = blink::WebMouseWheelEvent::kPhaseBegan;
  // We use root_view intentionally as the RenderWidgetHostInputEventRouter is
  // responsible for correctly routing the event to the child frame.
  router->RouteMouseWheelEvent(root_view, &wheel_event, ui::LatencyInfo());

  // Make sure that the renderer handled the input event.
  child_observer.Wait();

  EXPECT_TRUE(ExecuteScriptAndExtractInt(
      child, "window.domAutomationController.send(x);", &x));
  EXPECT_TRUE(ExecuteScriptAndExtractInt(
      child, "window.domAutomationController.send(y);", &y));
  EXPECT_TRUE(ExecuteScriptAndExtractInt(
      child, "window.domAutomationController.send(dX);", &deltaX));
  EXPECT_TRUE(ExecuteScriptAndExtractInt(
      child, "window.domAutomationController.send(dY);", &deltaY));
  EXPECT_EQ(14, x);
  EXPECT_EQ(15, y);
  EXPECT_EQ(16, deltaX);
  EXPECT_EQ(17, deltaY);
}

}  // namespace content
