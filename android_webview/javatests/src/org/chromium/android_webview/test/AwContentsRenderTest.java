// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.android_webview.test;

import android.graphics.Bitmap;
import android.graphics.Color;
import android.support.test.InstrumentationRegistry;
import android.support.test.filters.SmallTest;
import android.view.View;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.chromium.android_webview.AwContents;
import org.chromium.android_webview.AwContents.VisualStateCallback;
import org.chromium.android_webview.test.util.GraphicsTestUtils;
import org.chromium.base.ThreadUtils;
import org.chromium.base.test.util.Feature;
import org.chromium.content_public.common.ContentUrlConstants;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

/**
 * AwContents rendering / pixel tests.
 */
@RunWith(AwJUnit4ClassRunner.class)
public class AwContentsRenderTest {
    @Rule
    public AwActivityTestRule mActivityTestRule = new AwActivityTestRule();

    private TestAwContentsClient mContentsClient;
    private AwContents mAwContents;
    private AwTestContainerView mContainerView;

    @Before
    public void setUp() throws Exception {
        mContentsClient = new TestAwContentsClient();
        mContainerView = mActivityTestRule.createAwTestContainerViewOnMainSync(mContentsClient);
        mAwContents = mContainerView.getAwContents();
    }

    void setBackgroundColorOnUiThread(final int c) {
        ThreadUtils.runOnUiThreadBlocking(() -> mAwContents.setBackgroundColor(c));
    }

    @Test
    @SmallTest
    @Feature({"AndroidWebView"})
    public void testSetGetBackgroundColor() throws Throwable {
        setBackgroundColorOnUiThread(Color.MAGENTA);
        GraphicsTestUtils.pollForBackgroundColor(mAwContents, Color.MAGENTA);

        setBackgroundColorOnUiThread(Color.CYAN);
        GraphicsTestUtils.pollForBackgroundColor(mAwContents, Color.CYAN);

        mActivityTestRule.loadUrlSync(mAwContents, mContentsClient.getOnPageFinishedHelper(),
                ContentUrlConstants.ABOUT_BLANK_DISPLAY_URL);
        Assert.assertEquals(
                Color.CYAN, GraphicsTestUtils.sampleBackgroundColorOnUiThread(mAwContents));

        setBackgroundColorOnUiThread(Color.YELLOW);
        GraphicsTestUtils.pollForBackgroundColor(mAwContents, Color.YELLOW);

        mActivityTestRule.loadUrlSync(mAwContents, mContentsClient.getOnPageFinishedHelper(),
                "data:text/html,<html><head><style>body {background-color:#227788}</style></head>"
                        + "<body></body></html>");
        final int teal = 0xFF227788;
        GraphicsTestUtils.pollForBackgroundColor(mAwContents, teal);

        // Changing the base background should not override CSS background.
        setBackgroundColorOnUiThread(Color.MAGENTA);
        Assert.assertEquals(teal, GraphicsTestUtils.sampleBackgroundColorOnUiThread(mAwContents));
        // ...setting the background is asynchronous, so pause a bit and retest just to be sure.
        Thread.sleep(500);
        Assert.assertEquals(teal, GraphicsTestUtils.sampleBackgroundColorOnUiThread(mAwContents));
    }

    @Test
    @SmallTest
    @Feature({"AndroidWebView"})
    public void testPictureListener() throws Throwable {
        ThreadUtils.runOnUiThreadBlocking(() -> mAwContents.enableOnNewPicture(true, true));

        int pictureCount = mContentsClient.getPictureListenerHelper().getCallCount();
        mActivityTestRule.loadUrlSync(mAwContents, mContentsClient.getOnPageFinishedHelper(),
                ContentUrlConstants.ABOUT_BLANK_DISPLAY_URL);
        mContentsClient.getPictureListenerHelper().waitForCallback(pictureCount, 1);
        // Invalidation only, so picture should be null.
        Assert.assertNull(mContentsClient.getPictureListenerHelper().getPicture());
    }

    @Test
    @SmallTest
    @Feature({"AndroidWebView"})
    public void testForceDrawWhenInvisible() throws Throwable {
        mActivityTestRule.loadUrlSync(mAwContents, mContentsClient.getOnPageFinishedHelper(),
                "data:text/html,<html><head><style>body {background-color:#227788}</style></head>"
                        + "<body>Hello world!</body></html>");

        Bitmap visibleBitmap = null;
        Bitmap invisibleBitmap = null;
        final CountDownLatch latch = new CountDownLatch(1);
        InstrumentationRegistry.getInstrumentation().runOnMainSync(() -> {
            final long requestId1 = 1;
            mAwContents.insertVisualStateCallback(requestId1, new VisualStateCallback() {
                @Override
                public void onComplete(long id) {
                    Assert.assertEquals(requestId1, id);
                    latch.countDown();
                }
            });
        });
        Assert.assertTrue(latch.await(AwTestBase.WAIT_TIMEOUT_MS, TimeUnit.MILLISECONDS));

        final int width = ThreadUtils.runOnUiThreadBlockingNoException(
                () -> mContainerView.getWidth());
        final int height = ThreadUtils.runOnUiThreadBlockingNoException(
                () -> mContainerView.getHeight());
        visibleBitmap = GraphicsTestUtils.drawAwContentsOnUiThread(mAwContents, width, height);

        // Things that affect DOM page visibility:
        // 1. isPaused
        // 2. window's visibility, if the webview is attached to a window.
        // Note android.view.View's visibility does not affect DOM page visibility.
        InstrumentationRegistry.getInstrumentation().runOnMainSync(() -> {
            mContainerView.setVisibility(View.INVISIBLE);
            Assert.assertTrue(mAwContents.isPageVisible());

            mAwContents.onPause();
            Assert.assertFalse(mAwContents.isPageVisible());

            mAwContents.onResume();
            Assert.assertTrue(mAwContents.isPageVisible());

            // Simulate a window visiblity change. WebView test app can't
            // manipulate the window visibility directly.
            mAwContents.onWindowVisibilityChanged(View.INVISIBLE);
            Assert.assertFalse(mAwContents.isPageVisible());
        });

        // VisualStateCallback#onComplete won't be called when WebView is
        // invisible. So there is no reliable way to tell if View#setVisibility
        // has taken effect. Just sleep the test thread for 500ms.
        Thread.sleep(500);
        invisibleBitmap = GraphicsTestUtils.drawAwContentsOnUiThread(mAwContents, width, height);
        Assert.assertNotNull(invisibleBitmap);
        Assert.assertTrue(invisibleBitmap.sameAs(visibleBitmap));
    }
}
