// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/test/widget_test.h"

#include <algorithm>
#include <vector>

#include "testing/gtest/include/gtest/gtest.h"

#if defined(USE_AURA)
#include "ui/aura/window.h"
#endif

namespace views {
namespace test {
namespace {

// Insert |widget| into |expected| and ensure it's reported by GetAllWidgets().
void ExpectAdd(Widget::Widgets* expected, Widget* widget, const char* message) {
  SCOPED_TRACE(message);
  EXPECT_TRUE(expected->insert(widget).second);
  Widget::Widgets actual = WidgetTest::GetAllWidgets();
  EXPECT_EQ(expected->size(), actual.size());
  EXPECT_TRUE(std::equal(expected->begin(), expected->end(), actual.begin()));
}

// Close |widgets[0]|, and expect all |widgets| to be removed.
void ExpectClose(Widget::Widgets* expected,
                 std::vector<Widget*> widgets,
                 const char* message) {
  SCOPED_TRACE(message);
  for (Widget* widget : widgets)
    EXPECT_EQ(1u, expected->erase(widget));
  widgets[0]->CloseNow();
  Widget::Widgets actual = WidgetTest::GetAllWidgets();
  EXPECT_EQ(expected->size(), actual.size());
  EXPECT_TRUE(std::equal(expected->begin(), expected->end(), actual.begin()));
}

}  // namespace

using WidgetTestTest = WidgetTest;

// Ensure that Widgets with various root windows are correctly reported by
// WidgetTest::GetAllWidgets().
TEST_F(WidgetTestTest, GetAllWidgets) {
  // TODO: this test transitively uses GetContext(). That should go away for
  // aura-mus client. http://crbug.com/663781.
  if (IsMus())
    return;

  // Note Widget::Widgets is a std::set ordered by pointer value, so the order
  // that |expected| is updated below is not important.
  Widget::Widgets expected;

  EXPECT_EQ(expected, GetAllWidgets());

  Widget* platform = CreateTopLevelPlatformWidget();
  ExpectAdd(&expected, platform, "platform");

  Widget* platform_child = CreateChildPlatformWidget(platform->GetNativeView());
  ExpectAdd(&expected, platform_child, "platform_child");

  Widget* frameless = CreateTopLevelFramelessPlatformWidget();
  ExpectAdd(&expected, frameless, "frameless");

  Widget* native = CreateTopLevelNativeWidget();
  ExpectAdd(&expected, native, "native");

  Widget* native_child = CreateChildNativeWidgetWithParent(native);
  ExpectAdd(&expected, native_child, "native_child");

  Widget* desktop = CreateNativeDesktopWidget();
  ExpectAdd(&expected, desktop, "desktop");

  Widget* desktop_child = CreateChildNativeWidgetWithParent(desktop);
  ExpectAdd(&expected, desktop_child, "desktop_child");

#if defined(USE_AURA)
  // A DesktopWindowTreeHost has both a root aura::Window and a content window.
  // DesktopWindowTreeHostX11::GetAllOpenWindows() returns content windows, so
  // ensure that a Widget parented to the root window is also found.
  Widget* desktop_cousin =
      CreateChildPlatformWidget(desktop->GetNativeView()->GetRootWindow());
  ExpectAdd(&expected, desktop_cousin, "desktop_cousin");
  ExpectClose(&expected, {desktop_cousin}, "desktop_cousin");
#endif  // USE_AURA

  ExpectClose(&expected, {desktop, desktop_child}, "desktop");
  ExpectClose(&expected, {native, native_child}, "native");
  ExpectClose(&expected, {platform, platform_child}, "platform");
  ExpectClose(&expected, {frameless}, "frameless");
}

}  // namespace views
}  // namespace test
