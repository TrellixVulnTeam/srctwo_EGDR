// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/controls/native/native_view_host_mac.h"

#import <Cocoa/Cocoa.h>

#include <memory>

#import "base/mac/scoped_nsautorelease_pool.h"
#import "base/mac/scoped_nsobject.h"
#include "base/macros.h"
#import "testing/gtest_mac.h"
#include "ui/views/controls/native/native_view_host.h"
#include "ui/views/controls/native/native_view_host_test_base.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"

namespace views {

class NativeViewHostMacTest : public test::NativeViewHostTestBase {
 public:
  NativeViewHostMacTest() {}

  // testing::Test:
  void TearDown() override {
    // On Aura, the compositor is destroyed when the WindowTreeHost provided by
    // AuraTestHelper is destroyed. On Mac, the Widget is the host, so it must
    // be closed before the ContextFactory is torn down by ViewsTestBase.
    DestroyTopLevel();
    NativeViewHostTestBase::TearDown();
  }

  NativeViewHostMac* native_host() {
    return static_cast<NativeViewHostMac*>(GetNativeWrapper());
  }

  void CreateHost() {
    CreateTopLevel();
    CreateTestingHost();
    native_view_.reset([[NSView alloc] initWithFrame:NSZeroRect]);

    // Verify the expectation that the NativeViewHostWrapper is only created
    // after the NativeViewHost is added to a widget.
    EXPECT_FALSE(native_host());
    toplevel()->GetRootView()->AddChildView(host());
    EXPECT_TRUE(native_host());

    host()->Attach(native_view_);
  }

 protected:
  base::scoped_nsobject<NSView> native_view_;

 private:
  DISALLOW_COPY_AND_ASSIGN(NativeViewHostMacTest);
};

// Test destroying the top level widget before destroying the NativeViewHost.
// On Mac, also ensure that the native view is removed from its superview when
// the Widget containing its host is destroyed.
TEST_F(NativeViewHostMacTest, DestroyWidget) {
  ResetHostDestroyedCount();
  CreateHost();
  ReleaseHost();
  EXPECT_EQ(0, host_destroyed_count());
  EXPECT_TRUE([native_view_ superview]);
  DestroyTopLevel();
  EXPECT_FALSE([native_view_ superview]);
  EXPECT_EQ(1, host_destroyed_count());
}

// Ensure the native view receives the correct bounds when it is attached. On
// Mac, the bounds of the native view is relative to the NSWindow it is in, not
// the screen, and the coordinates have to be flipped.
TEST_F(NativeViewHostMacTest, Attach) {
  CreateHost();
  EXPECT_TRUE([native_view_ superview]);
  EXPECT_TRUE([native_view_ window]);

  host()->Detach();

  [native_view_ setFrame:NSZeroRect];
  toplevel()->SetBounds(gfx::Rect(64, 48, 100, 200));
  host()->SetBounds(10, 10, 80, 60);

  EXPECT_FALSE([native_view_ superview]);
  EXPECT_FALSE([native_view_ window]);
  EXPECT_NSEQ(NSZeroRect, [native_view_ frame]);

  host()->Attach(native_view_);
  EXPECT_TRUE([native_view_ superview]);
  EXPECT_TRUE([native_view_ window]);

  // Expect the top-left to be 10 pixels below the titlebar.
  int bottom = toplevel()->GetClientAreaBoundsInScreen().height() - 10 - 60;
  EXPECT_NSEQ(NSMakeRect(10, bottom, 80, 60), [native_view_ frame]);

  DestroyHost();
}

// Ensure the native view is hidden along with its host, and when detaching, or
// when attaching to a host that is already hidden.
TEST_F(NativeViewHostMacTest, NativeViewHidden) {
  CreateHost();
  toplevel()->SetBounds(gfx::Rect(0, 0, 100, 100));
  host()->SetBounds(10, 10, 80, 60);

  EXPECT_FALSE([native_view_ isHidden]);
  host()->SetVisible(false);
  EXPECT_TRUE([native_view_ isHidden]);
  host()->SetVisible(true);
  EXPECT_FALSE([native_view_ isHidden]);

  host()->Detach();
  EXPECT_TRUE([native_view_ isHidden]);  // Hidden when detached.
  [native_view_ setHidden:NO];

  host()->SetVisible(false);
  EXPECT_FALSE([native_view_ isHidden]);  // Stays visible.
  host()->Attach(native_view_);
  EXPECT_TRUE([native_view_ isHidden]);  // Hidden when attached.

  host()->Detach();
  [native_view_ setHidden:YES];
  host()->SetVisible(true);
  EXPECT_TRUE([native_view_ isHidden]);  // Stays hidden.
  host()->Attach(native_view_);
  EXPECT_FALSE([native_view_ isHidden]);  // Made visible when attached.

  EXPECT_TRUE([native_view_ superview]);
  toplevel()->GetRootView()->RemoveChildView(host());
  EXPECT_TRUE([native_view_ isHidden]);  // Hidden when removed from Widget.
  EXPECT_FALSE([native_view_ superview]);

  toplevel()->GetRootView()->AddChildView(host());
  EXPECT_FALSE([native_view_ isHidden]);  // And visible when added.
  EXPECT_TRUE([native_view_ superview]);

  DestroyHost();
}

// Check that we can destroy cleanly even if the native view has already been
// released.
TEST_F(NativeViewHostMacTest, NativeViewReleased) {
  {
    base::mac::ScopedNSAutoreleasePool pool;
    CreateHost();
    // In practice the native view is a WebContentsViewCocoa which is retained
    // by its superview (a TabContentsContainerView) and by WebContentsViewMac.
    // It's possible for both of them to be destroyed without calling
    // NativeHostView::Detach().
    [native_view_ removeFromSuperview];
    native_view_.reset();
  }

  // During teardown, NativeViewDetaching() is called in RemovedFromWidget().
  // Just trigger it with Detach().
  host()->Detach();

  DestroyHost();
}

}  // namespace views
