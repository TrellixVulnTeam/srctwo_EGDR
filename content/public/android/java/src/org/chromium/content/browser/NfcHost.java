// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.content.browser;

import android.app.Activity;
import android.util.SparseArray;

import org.chromium.base.Callback;
import org.chromium.base.annotations.CalledByNative;
import org.chromium.content_public.browser.WebContents;
import org.chromium.content_public.browser.WebContentsObserver;
import org.chromium.ui.base.WindowAndroid;

/** Tracks the Activiy for a given WebContents on behalf of a NFC instance that cannot talk
 * directly to WebContents.
 */
class NfcHost extends WebContentsObserver implements WindowAndroidChangedObserver {
    private static final SparseArray<NfcHost> sContextHostsMap = new SparseArray<NfcHost>();

    // The WebContents with which this host is associated.
    private final WebContents mWebContents;
    private final ContentViewCore mContentViewCore;

    // The context ID with which this host is associated.
    private final int mContextId;

    // The callback that the NFC instance has registered for being notified when the Activity
    // changes.
    private Callback<Activity> mCallback;

    /**
     * Provides access to NfcHost via context ID.
     */
    public static NfcHost fromContextId(int contextId) {
        return sContextHostsMap.get(contextId);
    }

    @CalledByNative
    private static NfcHost create(WebContents webContents, int contextId) {
        return new NfcHost(webContents, contextId);
    }

    NfcHost(WebContents webContents, int contextId) {
        super(webContents);

        mWebContents = webContents;
        mContentViewCore = ContentViewCore.fromWebContents(mWebContents);

        // NFC will not work if there is no CVC associated with the WebContents, and it will only
        // be requested in contexts where there is a CVC associated with the WebContents as far as
        // we are aware. If the latter ever proves false, then we will need to simply drop any NFC
        // request that comes in if there is no CVC available.
        assert mContentViewCore != null;
        mContextId = contextId;
        sContextHostsMap.put(mContextId, this);
    }

    /** Called by the NFC implementation (via ContentNfcDelegate) to allow that implementation to
     * track changes to the Activity associated with its context ID (i.e., the activity associated
     * with |mWebContents|).
     */

    public void trackActivityChanges(Callback<Activity> callback) {
        // Only the main frame is allowed to access NFC
        // (https://w3c.github.io/web-nfc/#security-policies). The renderer enforces this by
        // dropping connection requests from nested frames.  Therefore, this class should never see
        // a request to track activity changes while there is already such a request.
        assert mCallback == null : "Unexpected request to track activity changes";
        mCallback = callback;
        Activity activity = null;

        mContentViewCore.addWindowAndroidChangedObserver(this);
        if (mContentViewCore.getWindowAndroid() != null) {
            activity = mContentViewCore.getWindowAndroid().getActivity().get();
        }

        mCallback.onResult(activity);
    }

    /**
     * @see org.chromium.device.nfc.NfcDelegate#stopTrackingActivityForHost(int)
     */
    public void stopTrackingActivityChanges() {
        mCallback = null;
        mContentViewCore.removeWindowAndroidChangedObserver(this);
    }

    /**
     * Tears down current and future Activity tracking.
     */
    @Override
    public void destroy() {
        stopTrackingActivityChanges();
        sContextHostsMap.remove(mContextId);
        super.destroy();
    }

    /**
     * Updates the Activity associated with this instance.
     */
    @Override
    public void onWindowAndroidChanged(WindowAndroid newWindowAndroid) {
        Activity activity = null;
        if (newWindowAndroid != null) {
            activity = newWindowAndroid.getActivity().get();
        }
        assert mCallback != null : "should have callback";
        mCallback.onResult(activity);
    }
}
