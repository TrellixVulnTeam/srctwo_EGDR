// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.media.ui;

import static org.junit.Assert.assertEquals;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.doCallRealMethod;

import android.content.Intent;
import android.graphics.Bitmap;
import android.os.Build;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.annotation.Config;

import org.chromium.base.BaseChromiumApplication;
import org.chromium.base.BaseSwitches;
import org.chromium.base.BuildInfo;
import org.chromium.base.CommandLine;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.media.ui.MediaNotificationManager.ListenerService;
import org.chromium.testing.local.LocalRobolectricTestRunner;

/**
 * Test of media notifications to ensure that the favicon is displayed on normal devices and
 * not displayed on Android Go devices.
 */
@RunWith(LocalRobolectricTestRunner.class)
@Config(manifest = Config.NONE, application = BaseChromiumApplication.class,
        shadows = {MediaNotificationTestShadowResources.class,
                MediaNotificationTestShadowNotificationManager.class})
public class MediaNotificationFaviconTest extends MediaNotificationManagerTestBase {
    private static final int TAB_ID_1 = 1;
    private static final String IS_LOW_END_DEVICE_SWITCH =
            "--" + BaseSwitches.ENABLE_LOW_END_DEVICE_MODE;

    private final Bitmap mFavicon = Bitmap.createBitmap(192, 192, Bitmap.Config.ARGB_8888);
    private MediaNotificationTestTabHolder mTabHolder;

    @Before
    @Override
    public void setUp() {
        super.setUp();

        getManager().mThrottler.mManager = getManager();
        doCallRealMethod().when(getManager()).onServiceStarted(any(ListenerService.class));
        doCallRealMethod().when(mMockAppHooks).startForegroundService(any(Intent.class));
        mTabHolder = new MediaNotificationTestTabHolder(TAB_ID_1, "about:blank", "title1");
    }

    @After
    public void tearDown() {
        CommandLine.reset();
    }

    @Test
    public void testSetNotificationIcon() {
        mTabHolder.simulateMediaSessionStateChanged(true, false);
        mTabHolder.simulateFaviconUpdated(mFavicon);
        assertEquals(mFavicon, getDisplayedIcon());
    }

    @Test
    @Config(sdk = Build.VERSION_CODES.N_MR1)
    public void testSetNotificationIcon_lowMem_preO() {
        // Run tests as a low-end device.
        CommandLine.init(new String[] {"testcommand", IS_LOW_END_DEVICE_SWITCH});

        mTabHolder.simulateMediaSessionStateChanged(true, false);
        mTabHolder.simulateFaviconUpdated(mFavicon);
        assertEquals(mFavicon, getDisplayedIcon());
    }

    // TODO(crbug.com/729029): Specify O-SDK.
    @Test
    public void testSetNotificationIcon_lowMem_O() {
        // Run tests as a low-end device.
        CommandLine.init(new String[] {"testcommand", IS_LOW_END_DEVICE_SWITCH});

        mTabHolder.simulateMediaSessionStateChanged(true, false);
        mTabHolder.simulateFaviconUpdated(mFavicon);
        assertEquals(BuildInfo.isAtLeastO() ? null : mFavicon, getDisplayedIcon());
    }

    private Bitmap getDisplayedIcon() {
        return mTabHolder.mMediaSessionTabHelper.mFavicon;
    }

    @Override
    int getNotificationId() {
        return R.id.media_playback_notification;
    }
}
