// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.vr_shell;

import static org.chromium.chrome.browser.UrlConstants.BOOKMARKS_FOLDER_URL;
import static org.chromium.chrome.browser.UrlConstants.BOOKMARKS_UNCATEGORIZED_URL;
import static org.chromium.chrome.browser.UrlConstants.BOOKMARKS_URL;
import static org.chromium.chrome.browser.UrlConstants.DOWNLOADS_URL;
import static org.chromium.chrome.browser.UrlConstants.NATIVE_HISTORY_URL;
import static org.chromium.chrome.browser.UrlConstants.NTP_URL;
import static org.chromium.chrome.browser.UrlConstants.RECENT_TABS_URL;
import static org.chromium.chrome.browser.vr_shell.VrTestFramework.PAGE_LOAD_TIMEOUT_S;
import static org.chromium.chrome.browser.vr_shell.VrTestFramework.POLL_TIMEOUT_LONG_MS;
import static org.chromium.chrome.test.util.ChromeRestriction.RESTRICTION_TYPE_DEVICE_DAYDREAM;

import android.support.test.filters.MediumTest;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.chromium.base.test.util.CommandLineFlags;
import org.chromium.base.test.util.Restriction;
import org.chromium.chrome.browser.ChromeSwitches;
import org.chromium.chrome.browser.vr_shell.rules.ChromeTabbedActivityVrTestRule;
import org.chromium.chrome.browser.vr_shell.util.VrTransitionUtils;
import org.chromium.chrome.test.ChromeActivityTestRule;
import org.chromium.chrome.test.ChromeJUnit4ClassRunner;

import java.util.concurrent.TimeoutException;

/**
 * End-to-end tests for native UI presentation in VR Browser mode.
 */
@RunWith(ChromeJUnit4ClassRunner.class)
@CommandLineFlags.Add({ChromeSwitches.DISABLE_FIRST_RUN_EXPERIENCE,
        ChromeActivityTestRule.DISABLE_NETWORK_PREDICTION_FLAG, "enable-features=VrShell",
        "enable-webvr"})
@Restriction(RESTRICTION_TYPE_DEVICE_DAYDREAM)
public class VrShellNativeUiTest {
    // We explicitly instantiate a rule here instead of using parameterization since this class
    // only ever runs in ChromeTabbedActivity.
    @Rule
    public ChromeTabbedActivityVrTestRule mVrTestRule = new ChromeTabbedActivityVrTestRule();

    private static final String TEST_PAGE_2D_URL =
            VrTestFramework.getHtmlTestFile("test_navigation_2d_page");

    @Before
    public void setUp() throws Exception {
        VrTransitionUtils.forceEnterVr();
        VrTransitionUtils.waitForVrEntry(POLL_TIMEOUT_LONG_MS);
    }

    /**
     * Tests that URLs are not shown for native UI.
     */
    @Test
    @MediumTest
    public void testUrlOnNativeUi()
            throws IllegalArgumentException, InterruptedException, TimeoutException {
        mVrTestRule.loadUrl(BOOKMARKS_URL, PAGE_LOAD_TIMEOUT_S);
        Assert.assertFalse(
                "Should not be showing URL", VrShellDelegate.isDisplayingUrlForTesting());
        mVrTestRule.loadUrl(BOOKMARKS_FOLDER_URL, PAGE_LOAD_TIMEOUT_S);
        Assert.assertFalse(
                "Should not be showing URL", VrShellDelegate.isDisplayingUrlForTesting());
        mVrTestRule.loadUrl(BOOKMARKS_UNCATEGORIZED_URL, PAGE_LOAD_TIMEOUT_S);
        Assert.assertFalse(
                "Should not be showing URL", VrShellDelegate.isDisplayingUrlForTesting());
        mVrTestRule.loadUrl(DOWNLOADS_URL, PAGE_LOAD_TIMEOUT_S);
        Assert.assertFalse(
                "Should not be showing URL", VrShellDelegate.isDisplayingUrlForTesting());
        mVrTestRule.loadUrl(NTP_URL, PAGE_LOAD_TIMEOUT_S);
        Assert.assertFalse(
                "Should not be showing URL", VrShellDelegate.isDisplayingUrlForTesting());
        mVrTestRule.loadUrl(NATIVE_HISTORY_URL, PAGE_LOAD_TIMEOUT_S);
        Assert.assertFalse(
                "Should not be showing URL", VrShellDelegate.isDisplayingUrlForTesting());
        mVrTestRule.loadUrl(RECENT_TABS_URL, PAGE_LOAD_TIMEOUT_S);
        Assert.assertFalse(
                "Should not be showing URL", VrShellDelegate.isDisplayingUrlForTesting());
    }

    /**
     * Tests that URLs are shown for non-native UI.
     */
    @Test
    @MediumTest
    public void testUrlOnNonNativeUi()
            throws IllegalArgumentException, InterruptedException, TimeoutException {
        mVrTestRule.loadUrl(TEST_PAGE_2D_URL, PAGE_LOAD_TIMEOUT_S);
        Assert.assertTrue("Should be showing URL", VrShellDelegate.isDisplayingUrlForTesting());
    }
}
