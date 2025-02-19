// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.media.router;

import org.chromium.base.Log;
import org.chromium.base.ThreadUtils;
import org.chromium.chrome.browser.media.router.cast.MediaSink;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

/**
 * Mocked {@link MediaRouteProvider}.
 */
public class MockMediaRouteProvider implements MediaRouteProvider {
    private static final String TAG = "MediaRouter";

    private static final String SINK_ID1 = "test_sink_id_1";
    private static final String SINK_ID2 = "test_sink_id_2";
    private static final String SINK_NAME1 = "test-sink-1";
    private static final String SINK_NAME2 = "test-sink-2";

    private MediaRouteManager mManager;

    private final Map<String, MediaRoute> mRoutes = new HashMap<String, MediaRoute>();
    private final Map<String, MediaRoute> mPresentationIdToRoute =
            new HashMap<String, MediaRoute>();

    private int mSinksObservedDelayMillis = 0;
    private int mCreateRouteDelayMillis = 0;
    private boolean mIsSupportsSource = true;
    private String mCreateRouteErrorMessage = null;
    private String mJoinRouteErrorMessage = null;
    private boolean mCloseRouteWithErrorOnSend = false;

    /**
     * Factory for {@link MockMediaRouteProvider}.
     */
    public static class Factory implements MediaRouteProvider.Factory {
        public static final MockMediaRouteProvider sProvider = new MockMediaRouteProvider();

        @Override
        public void addProviders(MediaRouteManager manager) {
            sProvider.mManager = manager;
            manager.addMediaRouteProvider(sProvider);
        }
    }

    private MockMediaRouteProvider() {}

    public void setCreateRouteDelayMillis(int delayMillis) {
        assert delayMillis >= 0;
        mCreateRouteDelayMillis = delayMillis;
    }

    public void setSinksObservedDelayMillis(int delayMillis) {
        assert delayMillis >= 0;
        mSinksObservedDelayMillis = delayMillis;
    }

    public void setIsSupportsSource(boolean isSupportsSource) {
        mIsSupportsSource = isSupportsSource;
    }

    public void setCreateRouteErrorMessage(String errorMessage) {
        mCreateRouteErrorMessage = errorMessage;
    }

    public void setJoinRouteErrorMessage(String errorMessage) {
        mJoinRouteErrorMessage = errorMessage;
    }

    public void setCloseRouteWithErrorOnSend(boolean closeRouteWithErrorOnSend) {
        mCloseRouteWithErrorOnSend = closeRouteWithErrorOnSend;
    }

    @Override
    public boolean supportsSource(String sourceId) {
        return mIsSupportsSource;
    }

    @Override
    public void startObservingMediaSinks(final String sourceId) {
        final ArrayList<MediaSink> sinks = new ArrayList<MediaSink>();
        sinks.add(new MediaSink(SINK_ID1, SINK_NAME1, null));
        sinks.add(new MediaSink(SINK_ID2, SINK_NAME2, null));
        ThreadUtils.postOnUiThreadDelayed(new Runnable() {
                @Override
                public void run() {
                    mManager.onSinksReceived(sourceId, MockMediaRouteProvider.this, sinks);
                }
            }, mSinksObservedDelayMillis);
    }

    @Override
    public void stopObservingMediaSinks(String sourceId) {
    }

    @Override
    public void createRoute(final String sourceId, final String sinkId, final String presentationId,
                            final String origin, final int tabId, final boolean isIncognito,
                            final int nativeRequestId) {
        if (mCreateRouteErrorMessage != null) {
            mManager.onRouteRequestError(mCreateRouteErrorMessage, nativeRequestId);
            return;
        }

        if (mCreateRouteDelayMillis == 0) {
            doCreateRoute(sourceId, sinkId, presentationId, origin, tabId, nativeRequestId);
        } else {
            ThreadUtils.postOnUiThreadDelayed(new Runnable() {
                    @Override
                    public void run() {
                        doCreateRoute(
                                sourceId, sinkId, presentationId, origin, tabId, nativeRequestId);
                    }
                }, mCreateRouteDelayMillis);
        }
    }

    private void doCreateRoute(String sourceId, String sinkId, String presentationId, String origin,
                               int tabId, int nativeRequestId) {
        MediaRoute route = new MediaRoute(sinkId, sourceId, presentationId);
        mRoutes.put(route.id, route);
        mPresentationIdToRoute.put(presentationId, route);
        mManager.onRouteCreated(route.id, sinkId, nativeRequestId, this, true);
    }

    @Override
    public void joinRoute(String sourceId, String presentationId, String origin, int tabId,
                          int nativeRequestId) {
        if (mJoinRouteErrorMessage != null) {
            mManager.onRouteRequestError(mJoinRouteErrorMessage, nativeRequestId);
            return;
        }
        MediaRoute existingRoute = mPresentationIdToRoute.get(presentationId);
        if (existingRoute == null) {
            mManager.onRouteRequestError("Presentation does not exist", nativeRequestId);
            return;
        }
        mManager.onRouteCreated(
                existingRoute.id, existingRoute.sinkId, nativeRequestId, this, true);
    }

    @Override
    public void closeRoute(String routeId) {
        MediaRoute route = mRoutes.get(routeId);
        if (route == null) {
            Log.i(TAG, "closeRoute: Route does not exist: " + routeId);
            return;
        }
        mRoutes.remove(routeId);
        Map<String, MediaRoute> newPresentationIdToRoute = new HashMap<String, MediaRoute>();
        for (Map.Entry<String, MediaRoute> entry : mPresentationIdToRoute.entrySet()) {
            if (!entry.getValue().id.equals(routeId)) {
                newPresentationIdToRoute.put(entry.getKey(), entry.getValue());
            }
        }
        mPresentationIdToRoute.clear();
        mPresentationIdToRoute.putAll(newPresentationIdToRoute);
        mManager.onRouteClosed(routeId);
    }

    @Override
    public void detachRoute(String routeId) {
    }

    @Override
    public void sendStringMessage(String routeId, String message, int nativeCallbackId) {
        if (mCloseRouteWithErrorOnSend) {
            mManager.onRouteClosedWithError(routeId, "Sending message failed. Closing the route.");
        } else {
            mManager.onMessage(routeId, "Pong: " + message);
        }
    }
}
