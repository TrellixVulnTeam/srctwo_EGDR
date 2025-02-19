// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

suite('drawer-test', function() {
  var app;

  suiteSetup(function() {
    app = $('history-app');
  });

  test('drawer has correct selection', function() {
    app.selectedPage_ = 'syncedTabs';
    app.hasDrawer_ = true;
    return PolymerTest.flushTasks().then(function() {
      var drawer = /** @type {CrLazyRenderElement} */ (app.$.drawer);
      var drawerSideBar = app.$$('#drawer-side-bar');

      assertTrue(!!drawer);
      // Drawer side bar doesn't exist until the first time the drawer is
      // opened.
      assertFalse(!!drawerSideBar);

      var menuButton = app.$.toolbar.$['main-toolbar'].$$('#menuButton');
      assertTrue(!!menuButton);

      MockInteractions.tap(menuButton);
      assertTrue(drawer.getIfExists().open);
      drawerSideBar = app.$$('#drawer-side-bar');
      assertTrue(!!drawerSideBar);

      assertEquals('syncedTabs', drawerSideBar.$.menu.selected);
    });
  });
});
