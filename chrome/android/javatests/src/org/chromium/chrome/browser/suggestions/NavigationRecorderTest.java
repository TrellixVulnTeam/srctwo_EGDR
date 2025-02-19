// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.suggestions;

import static junit.framework.Assert.assertNull;

import static org.junit.Assert.assertEquals;

import android.support.test.InstrumentationRegistry;
import android.support.test.filters.SmallTest;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.chromium.base.Callback;
import org.chromium.base.ThreadUtils;
import org.chromium.base.test.util.CallbackHelper;
import org.chromium.base.test.util.CommandLineFlags;
import org.chromium.chrome.browser.ChromeSwitches;
import org.chromium.chrome.browser.UrlConstants;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.chrome.test.BottomSheetTestRule;
import org.chromium.chrome.test.ChromeJUnit4ClassRunner;
import org.chromium.chrome.test.ChromeTabbedActivityTestRule;
import org.chromium.chrome.test.util.ChromeTabUtils;
import org.chromium.content_public.browser.LoadUrlParams;
import org.chromium.net.test.EmbeddedTestServer;

import java.util.concurrent.TimeoutException;

/**
 * Instrumentation tests for {@link NavigationRecorder}.
 */
@RunWith(ChromeJUnit4ClassRunner.class)
@CommandLineFlags.Add({ChromeSwitches.DISABLE_FIRST_RUN_EXPERIENCE,
        BottomSheetTestRule.DISABLE_NETWORK_PREDICTION_FLAG})
public class NavigationRecorderTest {
    @Rule
    public ChromeTabbedActivityTestRule mTestSetupRule = new ChromeTabbedActivityTestRule();

    private EmbeddedTestServer mTestServer;
    private String mNavUrl;
    private Tab mInitialTab;

    @Before
    public void setUp() throws InterruptedException {
        mTestSetupRule.startMainActivityWithURL(UrlConstants.NTP_URL);

        mTestServer = EmbeddedTestServer.createAndStartServer(
                InstrumentationRegistry.getInstrumentation().getContext());
        mNavUrl = mTestServer.getURL("/chrome/test/data/android/google.html");

        mInitialTab = mTestSetupRule.getActivity().getActivityTab();
    }

    @After
    public void tearDown() {
        mTestServer.stopAndDestroyServer();
    }

    @Test
    @SmallTest
    public void testRecordVisitInCurrentTabEndsWithBack()
            throws InterruptedException, TimeoutException {
        final CallbackHelper callback = new CallbackHelper();
        loadUrlAndRecordVisit(mNavUrl, new Callback<NavigationRecorder.VisitData>() {
            @Override
            public void onResult(NavigationRecorder.VisitData visit) {
                // When the tab is hidden we receive a notification with no end URL.
                assertEquals(UrlConstants.NTP_URL, visit.endUrl);
                callback.notifyCalled();
            }
        });

        ChromeTabUtils.waitForTabPageLoaded(mInitialTab, (String) null);
        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                mInitialTab.goBack();
            }
        });

        callback.waitForCallback(0);
    }

    @Test
    @SmallTest
    public void testRecordVisitInCurrentTabEndsWhenHidden()
            throws InterruptedException, TimeoutException {
        final CallbackHelper callback = new CallbackHelper();
        loadUrlAndRecordVisit(mNavUrl, new Callback<NavigationRecorder.VisitData>() {
            @Override
            public void onResult(NavigationRecorder.VisitData visit) {
                // When the tab is hidden we receive a notification with no end URL.
                assertNull(visit.endUrl);
                callback.notifyCalled();
            }
        });

        mTestSetupRule.loadUrlInNewTab(null);
        callback.waitForCallback(0);
    }

    @Test
    @SmallTest
    public void testRecordVisitInCurrentTabEndsWhenURLTyped()
            throws InterruptedException, TimeoutException {
        final CallbackHelper callback = new CallbackHelper();
        loadUrlAndRecordVisit(mNavUrl, new Callback<NavigationRecorder.VisitData>() {
            @Override
            public void onResult(NavigationRecorder.VisitData visit) {
                // When the visit is hidden because of the transition type we get no URL.
                assertNull(visit.endUrl);
                callback.notifyCalled();
            }
        });

        mTestSetupRule.loadUrl(null);
        callback.waitForCallback(0);
    }

    /** Loads the provided URL in the current tab and sets up navigation recording for it. */
    private void loadUrlAndRecordVisit(
            final String url, Callback<NavigationRecorder.VisitData> visitCallback) {
        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                mInitialTab.loadUrl(new LoadUrlParams(url));
            }
        });
        NavigationRecorder.record(mInitialTab, visitCallback);
    }
}
