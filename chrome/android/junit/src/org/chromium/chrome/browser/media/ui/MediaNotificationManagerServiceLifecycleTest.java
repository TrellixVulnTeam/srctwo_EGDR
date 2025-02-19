// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.media.ui;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.anyBoolean;
import static org.mockito.Mockito.anyInt;
import static org.mockito.Mockito.doCallRealMethod;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.inOrder;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import android.app.Notification;
import android.content.Intent;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InOrder;
import org.robolectric.annotation.Config;

import org.chromium.base.BaseChromiumApplication;
import org.chromium.chrome.browser.media.ui.MediaNotificationManager.ListenerService;
import org.chromium.chrome.browser.notifications.NotificationUmaTracker;
import org.chromium.chrome.browser.notifications.channels.ChannelDefinitions;
import org.chromium.content_public.common.MediaMetadata;
import org.chromium.testing.local.LocalRobolectricTestRunner;

import java.util.concurrent.TimeoutException;

/**
 * JUnit tests for checking {@link MediaNotificationManager} handles the listener service life cycle
 * correctly.
 */
@RunWith(LocalRobolectricTestRunner.class)
@Config(manifest = Config.NONE, application = BaseChromiumApplication.class,
        shadows = {MediaNotificationTestShadowResources.class,
                MediaNotificationTestShadowNotificationManager.class})
public class MediaNotificationManagerServiceLifecycleTest extends MediaNotificationManagerTestBase {
    @Test
    public void testServiceLifeCycle() {
        ensureMediaNotificationInfo();

        Intent intent = getManager().createIntent();

        assertNull(mService);
        mMockContext.startService(intent);
        verify(getManager()).onServiceStarted(mService);
        assertNotNull(mService);
        verify(mService).onStartCommand(intent, 0, 0);

        mService.stopListenerService();
        assertNull(mService);
    }

    @Test
    public void testProcessIntentFailureStopsService() throws TimeoutException {
        MediaNotificationManager manager = getManager();
        setUpService();

        ListenerService service = mService;
        doReturn(false).when(mService).processIntent(any(Intent.class));
        mMockContext.startService(new Intent());
        verify(service).stopListenerService();
        assertNull(getManager());
        verify(manager).onServiceDestroyed();
    }

    @Test
    public void testProcessNullIntent() {
        setUpService();
        assertFalse(mService.processIntent(null));
    }

    @Test
    public void testProcessIntentWhenManagerIsNull() {
        setUpService();
        MediaNotificationManager.setManagerForTesting(getNotificationId(), null);
        assertFalse(mService.processIntent(new Intent()));
    }

    @Test
    public void testProcessIntentWhenNotificationInfoIsNull() {
        setUpService();
        getManager().mMediaNotificationInfo = null;
        assertFalse(mService.processIntent(new Intent()));
    }

    @Test
    public void testShowNotificationIsNoOpWhenInfoMatches() {
        doCallRealMethod().when(getManager()).onServiceStarted(any(ListenerService.class));
        doNothing().when(getManager()).updateNotification(anyBoolean());
        setUpServiceAndClearInvocations();

        MediaNotificationInfo newInfo = mMediaNotificationInfoBuilder.build();
        getManager().showNotification(newInfo);

        verify(getManager()).showNotification(newInfo);
        verifyNoMoreInteractions(getManager());
        verify(mMockAppHooks, never()).startForegroundService(any(Intent.class));
        verify(mMockContext, never()).startService(any(Intent.class));
        verify(mMockUmaTracker, never()).onNotificationShown(anyInt(), anyString());
    }

    @Test
    public void testShowNotificationIsNoOpWhenInfoIsPausedAndFromAnotherTab() {
        doCallRealMethod().when(getManager()).onServiceStarted(any(ListenerService.class));
        doNothing().when(getManager()).updateNotification(anyBoolean());
        mMediaNotificationInfoBuilder.setTabId(0);
        setUpServiceAndClearInvocations();

        mMediaNotificationInfoBuilder.setTabId(1).setPaused(true);
        MediaNotificationInfo newInfo = mMediaNotificationInfoBuilder.build();
        getManager().showNotification(newInfo);

        verify(getManager()).showNotification(newInfo);
        verifyNoMoreInteractions(getManager());
        verify(mMockAppHooks, never()).startForegroundService(any(Intent.class));
        verify(mMockContext, never()).startService(any(Intent.class));
        verify(mMockUmaTracker, never()).onNotificationShown(anyInt(), anyString());
    }

    @Test
    public void testShowNotificationWhenServiceNotCreated() {
        MediaNotificationInfo newInfo = mMediaNotificationInfoBuilder.build();
        getManager().showNotification(newInfo);

        verify(getManager(), times(1)).updateMediaSession();
        verify(getManager(), times(1)).updateNotificationBuilder();
        verify(mMockContext, never()).startService(any(Intent.class));
        verify(mMockAppHooks, times(1)).startForegroundService(any(Intent.class));
        verify(getManager(), never()).updateNotification(anyBoolean());
    }

    @Test
    public void testShowNotificationWhenServiceAlreadyCreated() {
        doCallRealMethod().when(getManager()).onServiceStarted(any(ListenerService.class));
        doNothing().when(getManager()).updateNotification(anyBoolean());
        setUpServiceAndClearInvocations();

        mMediaNotificationInfoBuilder.setPaused(true);
        MediaNotificationInfo newInfo = mMediaNotificationInfoBuilder.build();
        getManager().showNotification(newInfo);

        verify(getManager()).showNotification(newInfo);
        verify(mMockAppHooks, never()).startForegroundService(any(Intent.class));
        verify(mMockContext, never()).startService(any(Intent.class));
        verify(getManager()).updateNotification(anyBoolean());
        verify(mMockUmaTracker, never()).onNotificationShown(anyInt(), anyString());
    }

    @Test
    public void testShowNotificationBeforeServiceCreatedUpdatesNotificationInfoAndLogsUma() {
        doCallRealMethod().when(getManager()).onServiceStarted(any(ListenerService.class));
        doNothing().when(getManager()).updateNotification(anyBoolean());

        // The initial call to |showNotification()| should update the notification info and request
        // to start the service.
        MediaNotificationInfo oldInfo = mMediaNotificationInfoBuilder.build();
        getManager().showNotification(oldInfo);

        InOrder order = inOrder(getManager(), mMockAppHooks);

        assertEquals(oldInfo, getManager().mMediaNotificationInfo);
        order.verify(getManager(), times(1)).updateMediaSession();
        order.verify(getManager(), times(1)).updateNotificationBuilder();
        order.verify(mMockAppHooks, times(1)).startForegroundService(any(Intent.class));
        order.verify(getManager(), never()).updateNotification(anyBoolean());

        // The second call to |showNotification()| should only update the notification info.
        mMediaNotificationInfoBuilder.setMetadata(new MediaMetadata("new title", "", ""));
        MediaNotificationInfo newInfo = mMediaNotificationInfoBuilder.build();
        getManager().showNotification(newInfo);

        assertEquals(newInfo, getManager().mMediaNotificationInfo);
        order.verify(getManager(), times(1)).updateMediaSession();
        order.verify(getManager(), times(1)).updateNotificationBuilder();
        order.verify(mMockAppHooks, times(1)).startForegroundService(any(Intent.class));
        order.verify(getManager(), never()).updateNotification(anyBoolean());

        verify(getManager(), never()).onServiceStarted(any(ListenerService.class));

        // Simulate the service has started.
        mMockContext.startService(getManager().createIntent());
        order.verify(getManager(), times(1)).onServiceStarted(mService);
        order.verify(getManager(), times(1)).updateNotification(anyBoolean());
        verify(mMockUmaTracker)
                .onNotificationShown(
                        NotificationUmaTracker.MEDIA, ChannelDefinitions.CHANNEL_ID_MEDIA);
    }

    @Test
    public void updateNotificationIsNoOpBeforeServiceCreated() {
        getManager().mMediaNotificationInfo = mMediaNotificationInfoBuilder.build();
        getManager().updateNotification(false);

        verify(getManager()).updateNotification(anyBoolean());
        verify(getManager(), never()).updateMediaSession();
        verify(getManager(), never()).updateNotificationBuilder();
    }

    @Test
    public void updateNotificationIsNoOpWhenNotificiationInfoIsNull() {
        setUpService();
        getManager().mService = mService;
        getManager().mMediaNotificationInfo = null;
        getManager().updateNotification(false);

        verify(getManager()).updateNotification(anyBoolean());
        verify(getManager(), never()).updateMediaSession();
        verify(getManager(), never()).updateNotificationBuilder();

        verify(mService, never()).stopForeground(anyBoolean());
        verify(mService, never()).startForeground(anyInt(), any(Notification.class));
    }

    @Test
    public void updateNotificationSetsServiceBackgroundWhenPausedAndSupportsSwipeAway() {
        mMediaNotificationInfoBuilder.setPaused(true);
        setUpService();
        getManager().mService = mService;
        getManager().mMediaNotificationInfo = mMediaNotificationInfoBuilder.build();
        getManager().updateNotification(false);

        verify(mService).stopForeground(false);
        // One of the invocations comes from |setUpService()|.
        verify(MediaNotificationTestShadowNotificationManager.sMockObserver, times(2))
                .notify(eq(getNotificationId()), any(Notification.class));
    }

    @Test
    public void updateNotificationSetsServiceBackgroundWhenPausedButDoesntSupportSwipeAway() {
        mMediaNotificationInfoBuilder.setPaused(true).setActions(0);
        setUpService();
        getManager().mService = mService;
        getManager().mMediaNotificationInfo = mMediaNotificationInfoBuilder.build();
        getManager().updateNotification(false);

        verify(mService).startForeground(eq(getNotificationId()), any(Notification.class));
    }

    @Test
    public void updateNotificationSetsServiceForegroundWhenPlaying() {
        mMediaNotificationInfoBuilder.setPaused(false);
        setUpService();
        getManager().mService = mService;
        getManager().mMediaNotificationInfo = mMediaNotificationInfoBuilder.build();
        getManager().updateNotification(false);

        verify(mService).startForeground(eq(getNotificationId()), any(Notification.class));
    }
}
