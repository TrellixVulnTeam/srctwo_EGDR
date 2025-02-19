// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.media.ui;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertSame;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.clearInvocations;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.verify;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.annotation.Config;

import org.chromium.base.BaseChromiumApplication;
import org.chromium.testing.local.LocalRobolectricTestRunner;

/**
 * JUnit tests for checking {@link MediaNotificationManager} throttles notification updates
 * correctly.
 */
@RunWith(LocalRobolectricTestRunner.class)
@Config(manifest = Config.NONE, application = BaseChromiumApplication.class,
        shadows = {MediaNotificationTestShadowResources.class})
public class MediaNotificationManagerThrottlerTest extends MediaNotificationManagerTestBase {
    private static final int THROTTLE_MILLIS = MediaNotificationManager.Throttler.THROTTLE_MILLIS;

    @Before
    @Override
    public void setUp() {
        super.setUp();
        getManager().mThrottler = spy(new MediaNotificationManager.Throttler(getManager()));
    }

    @Test
    public void testEmptyState() {
        assertNull(getThrottler().mTask);
        assertNull(getThrottler().mLastPendingInfo);
    }

    @Test
    public void testFirstNotificationIsUnthrottled() {
        MediaNotificationInfo info = mMediaNotificationInfoBuilder.build();
        getThrottler().queueNotification(info);

        // Verify the notification is shown immediately
        verify(getThrottler()).showNotificationImmediately(info);
        verify(getManager()).showNotification(info);

        // Verify entering the throttled state.
        assertNotNull(getThrottler().mTask);
        assertNull(getThrottler().mLastPendingInfo);
    }

    @Test
    public void testDelayedTaskFiresWithNoQueuedNotification() {
        MediaNotificationInfo info = mMediaNotificationInfoBuilder.build();
        getThrottler().queueNotification(info);
        clearInvocations(getThrottler());

        // When the task fires while no notification info is queued in throttled state, the
        // throttler enters unthrottled state.
        advanceTimeByMillis(THROTTLE_MILLIS);
        verify(getThrottler(), never()).showNotificationImmediately(info);
        assertNull(getThrottler().mTask);
    }

    @Test
    public void testDelayedTaskFiresWithQueuedNotification() {
        mMediaNotificationInfoBuilder.setPaused(false);
        MediaNotificationInfo info1 = mMediaNotificationInfoBuilder.build();
        mMediaNotificationInfoBuilder.setPaused(true);
        MediaNotificationInfo info2 = mMediaNotificationInfoBuilder.build();

        getThrottler().queueNotification(info1);
        getThrottler().queueNotification(info2);

        // In throttled state, queueing a notification info will only update the queued notification
        // info.
        verify(getThrottler()).showNotificationImmediately(info1);
        verify(getThrottler(), never()).showNotificationImmediately(info2);
        assertNotNull(getThrottler().mTask);
        assertSame(info2, getThrottler().mLastPendingInfo);
        clearInvocations(getThrottler());

        // When the delayed task fires, the queued notification info will be used to update
        // notification.
        advanceTimeByMillis(THROTTLE_MILLIS);
        verify(getThrottler()).showNotificationImmediately(info2);
        assertNotNull(getThrottler().mTask);
        assertNull(getThrottler().mLastPendingInfo);
    }

    @Test
    public void testQueueingNotificationWontResetTimer() {
        mMediaNotificationInfoBuilder.setPaused(false);
        MediaNotificationInfo info1 = mMediaNotificationInfoBuilder.build();
        mMediaNotificationInfoBuilder.setPaused(true);
        MediaNotificationInfo info2 = mMediaNotificationInfoBuilder.build();

        getThrottler().queueNotification(info1);
        // Wait till the timer goes half way.
        advanceTimeByMillis(THROTTLE_MILLIS / 2);
        getThrottler().queueNotification(info2);

        clearInvocations(getThrottler());

        // The task should fire after |THROTTLE_MILLIS| since the it is posted.
        advanceTimeByMillis(THROTTLE_MILLIS / 2);
        verify(getThrottler()).showNotificationImmediately(info2);
        assertNotNull(getThrottler().mTask);
        assertNull(getThrottler().mLastPendingInfo);

        // Can't check whether another task is posted due to the limitation of Robolectric shadows.
    }

    @Test
    public void testQueueNotificationIgnoresNotification_Unthrottled() {
        mMediaNotificationInfoBuilder.setPaused(false);
        MediaNotificationInfo infoPlaying1 = mMediaNotificationInfoBuilder.build();
        MediaNotificationInfo infoPlaying2 = mMediaNotificationInfoBuilder.build();
        mMediaNotificationInfoBuilder.setPaused(true);
        MediaNotificationInfo infoPaused1 = mMediaNotificationInfoBuilder.build();
        MediaNotificationInfo infoPaused2 = mMediaNotificationInfoBuilder.build();

        // This will update the notification immediately and enter throttled state.
        getThrottler().queueNotification(infoPlaying1);

        // This will not update the queued notification as it is the same as which stored in
        // MediaNotificationManager.
        getThrottler().queueNotification(infoPlaying2);
        assertNull(getThrottler().mLastPendingInfo);

        // This will update the queued notification info as it is a different one.
        getThrottler().queueNotification(infoPaused1);
        assertSame(infoPaused1, getThrottler().mLastPendingInfo);

        // This will not update the queued notification info as it is the same with the queued info.
        getThrottler().queueNotification(infoPaused2);
        assertSame(infoPaused1, getThrottler().mLastPendingInfo);

        // This will not update the queued notification info as it is different from the queued
        // info.
        getThrottler().queueNotification(infoPlaying1);
        assertSame(infoPlaying1, getThrottler().mLastPendingInfo);
    }

    @Test
    public void testQueueNotificationIgnoresNotification_Throttled() {
        MediaNotificationInfo info1 = mMediaNotificationInfoBuilder.build();
        MediaNotificationInfo info2 = mMediaNotificationInfoBuilder.build();

        getThrottler().queueNotification(info1);
        clearInvocations(getThrottler());
        getThrottler().queueNotification(info2);

        verify(getThrottler(), never())
                .showNotificationImmediately(any(MediaNotificationInfo.class));
        assertNotNull(getThrottler().mTask);
        assertNull(getThrottler().mLastPendingInfo);
    }

    @Test
    public void testClearNotificationClearsThrottler() {
        MediaNotificationInfo info = mMediaNotificationInfoBuilder.build();
        getThrottler().queueNotification(info);

        getManager().clearNotification();

        verify(getThrottler()).clearPendingNotifications();
    }

    @Test
    public void testClearThrottler() {
        MediaNotificationInfo info = mMediaNotificationInfoBuilder.build();
        getThrottler().queueNotification(info);
        getThrottler().clearPendingNotifications();
        clearInvocations(getThrottler());

        assertNull(getThrottler().mTask);
        assertNull(getThrottler().mLastPendingInfo);

        // Check the task is removed
        advanceTimeByMillis(THROTTLE_MILLIS);

        verify(getThrottler(), never())
                .showNotificationImmediately(any(MediaNotificationInfo.class));
    }

    private MediaNotificationManager.Throttler getThrottler() {
        return getManager().mThrottler;
    }
}
