// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.android_webview.test;

import android.support.test.InstrumentationRegistry;
import android.support.test.filters.SmallTest;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.chromium.android_webview.AwContents;
import org.chromium.base.test.util.Feature;
import org.chromium.base.test.util.UrlUtils;
import org.chromium.content.browser.test.util.HistoryUtils;
import org.chromium.content.browser.test.util.TestCallbackHelperContainer.OnPageFinishedHelper;
import org.chromium.content_public.browser.WebContents;

/**
 * Tests for a wanted clearHistory method.
 */
@RunWith(AwJUnit4ClassRunner.class)
public class ClearHistoryTest {
    @Rule
    public AwActivityTestRule mActivityTestRule = new AwActivityTestRule();

    private static final String[] URLS = new String[3];
    {
        for (int i = 0; i < URLS.length; i++) {
            URLS[i] = UrlUtils.encodeHtmlDataUri(
                    "<html><head></head><body>" + i + "</body></html>");
        }
    }

    @Test
    @SmallTest
    @Feature({"History", "Main"})
    public void testClearHistory() throws Throwable {
        final TestAwContentsClient contentsClient = new TestAwContentsClient();
        final AwTestContainerView testContainerView =
                mActivityTestRule.createAwTestContainerViewOnMainSync(contentsClient);
        final AwContents awContents = testContainerView.getAwContents();
        final WebContents webContents = awContents.getWebContents();
        OnPageFinishedHelper onPageFinishedHelper = contentsClient.getOnPageFinishedHelper();
        for (int i = 0; i < 3; i++) {
            mActivityTestRule.loadUrlSync(awContents, onPageFinishedHelper, URLS[i]);
        }

        HistoryUtils.goBackSync(
                InstrumentationRegistry.getInstrumentation(), webContents, onPageFinishedHelper);
        Assert.assertTrue("Should be able to go back",
                HistoryUtils.canGoBackOnUiThread(
                        InstrumentationRegistry.getInstrumentation(), webContents));
        Assert.assertTrue("Should be able to go forward",
                HistoryUtils.canGoForwardOnUiThread(
                        InstrumentationRegistry.getInstrumentation(), webContents));

        HistoryUtils.clearHistoryOnUiThread(
                InstrumentationRegistry.getInstrumentation(), webContents);
        Assert.assertFalse("Should not be able to go back",
                HistoryUtils.canGoBackOnUiThread(
                        InstrumentationRegistry.getInstrumentation(), webContents));
        Assert.assertFalse("Should not be able to go forward",
                HistoryUtils.canGoForwardOnUiThread(
                        InstrumentationRegistry.getInstrumentation(), webContents));
    }
}
