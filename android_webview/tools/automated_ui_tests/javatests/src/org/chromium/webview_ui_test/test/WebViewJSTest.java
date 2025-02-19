// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.webview_ui_test.test;

import static android.support.test.espresso.Espresso.onView;
import static android.support.test.espresso.action.ViewActions.click;
import static android.support.test.espresso.assertion.ViewAssertions.doesNotExist;
import static android.support.test.espresso.assertion.ViewAssertions.matches;
import static android.support.test.espresso.matcher.RootMatchers.withDecorView;
import static android.support.test.espresso.matcher.ViewMatchers.isDisplayed;
import static android.support.test.espresso.matcher.ViewMatchers.isEnabled;
import static android.support.test.espresso.matcher.ViewMatchers.withText;
import static android.support.test.espresso.web.sugar.Web.onWebView;

import android.support.test.filters.MediumTest;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.chromium.base.test.BaseJUnit4ClassRunner;
import org.chromium.webview_ui_test.WebViewUiTestActivity;
import org.chromium.webview_ui_test.test.util.UseLayout;
import org.chromium.webview_ui_test.test.util.WebViewUiTestRule;


/**
 * This test suite is a collection of tests that loads Javascripts and interact with WebView.
 *
 * The ActivityTestRule used in this test ensures that Javascripts and webpages are loaded
 */
@RunWith(BaseJUnit4ClassRunner.class)
public class WebViewJSTest {
    @Rule
    public WebViewUiTestRule mWebViewActivityRule = new WebViewUiTestRule(
            WebViewUiTestActivity.class);

    @Before
    public void setUp() {
        mWebViewActivityRule.launchActivity();
        onWebView().forceJavascriptEnabled();
    }

    @Test
    @MediumTest
    @UseLayout("fullscreen_webview")
    public void testJsLoad() {
        mWebViewActivityRule.loadFileSync("alert.html", false);
        mWebViewActivityRule.loadJavaScriptSync(
                "document.getElementById('alert-button').click();", false);
        onView(withText("Clicked"))
                .inRoot(withDecorView(isEnabled()));
        onView(withText("OK"))
                .check(matches(isDisplayed()))
                .perform(click());
        onView(withText("OK")).check(doesNotExist());
    }
}
