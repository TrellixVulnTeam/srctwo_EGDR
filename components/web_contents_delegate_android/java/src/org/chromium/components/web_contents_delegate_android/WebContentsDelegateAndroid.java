// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.components.web_contents_delegate_android;

import android.view.KeyEvent;

import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;
import org.chromium.content.browser.ContentVideoViewEmbedder;
import org.chromium.content_public.browser.WebContents;
import org.chromium.content_public.common.ResourceRequestBody;

/**
 * Java peer of the native class of the same name.
 */
@JNINamespace("web_contents_delegate_android")
public class WebContentsDelegateAndroid {
    // Equivalent of WebCore::WebConsoleMessage::LevelTip.
    public static final int LOG_LEVEL_TIP = 0;
    // Equivalent of WebCore::WebConsoleMessage::LevelLog.
    public static final int LOG_LEVEL_LOG = 1;
    // Equivalent of WebCore::WebConsoleMessage::LevelWarning.
    public static final int LOG_LEVEL_WARNING = 2;
    // Equivalent of WebCore::WebConsoleMessage::LevelError.
    public static final int LOG_LEVEL_ERROR = 3;

    // The most recent load progress callback received from WebContents, as a percentage.
    // Initialize to 100 to indicate that we're not in a loading state.
    private int mMostRecentProgress = 100;

    public int getMostRecentProgress() {
        return mMostRecentProgress;
    }

    /**
     * @param disposition         The new tab disposition, defined in
     *                            //ui/base/mojo/window_open_disposition.mojom.
     * @param isRendererInitiated Whether or not the renderer initiated this action.
     */
    @CalledByNative
    public void openNewTab(String url, String extraHeaders, ResourceRequestBody postData,
            int disposition, boolean isRendererInitiated) {}

    @CalledByNative
    public void activateContents() {}

    @CalledByNative
    public void closeContents() {}

    @CalledByNative
    public void loadingStateChanged(boolean toDifferentDocument) {}

    @CalledByNative
    public void navigationStateChanged(int flags) {}

    @CalledByNative
    public void visibleSSLStateChanged() {}

    @SuppressWarnings("unused")
    @CalledByNative
    private final void notifyLoadProgressChanged(double progress) {
        mMostRecentProgress = (int) (100.0 * progress);
        onLoadProgressChanged(mMostRecentProgress);
    }

    /**
     * @param progress The load progress [0, 100] for the current web contents.
     */
    public void onLoadProgressChanged(int progress) {}

    /**
     * Signaled when the renderer has been deemed to be unresponsive.
     */
    @CalledByNative
    public void rendererUnresponsive() {}

    /**
     * Signaled when the render has been deemed to be responsive.
     */
    @CalledByNative
    public void rendererResponsive() {}

    @CalledByNative
    public void webContentsCreated(WebContents sourceWebContents, long openerRenderProcessId,
            long openerRenderFrameId, String frameName, String targetUrl,
            WebContents newWebContents) {}

    @CalledByNative
    public boolean shouldCreateWebContents(String targetUrl) {
        return true;
    }

    @CalledByNative
    public void onUpdateUrl(String url) {}

    @CalledByNative
    public boolean takeFocus(boolean reverse) {
        return false;
    }

    @CalledByNative
    public void handleKeyboardEvent(KeyEvent event) {
        // TODO(bulach): we probably want to re-inject the KeyEvent back into
        // the system. Investigate if this is at all possible.
    }

    /**
     * Report a JavaScript console message.
     *
     * @param level message level. One of WebContentsDelegateAndroid.LOG_LEVEL*.
     * @param message the error message.
     * @param lineNumber the line number int the source file at which the error is reported.
     * @param sourceId the name of the source file that caused the error.
     * @return true if the client will handle logging the message.
     */
    @CalledByNative
    public boolean addMessageToConsole(int level, String message, int lineNumber, String sourceId) {
        return false;
    }

    /**
     * Report a form resubmission. The overwriter of this function should eventually call
     * either of NavigationController.ContinuePendingReload or
     * NavigationController.CancelPendingReload.
     */
    @CalledByNative
    public void showRepostFormWarningDialog() {}

    @CalledByNative
    public void toggleFullscreenModeForTab(boolean enterFullscreen) {}

    @CalledByNative
    public boolean isFullscreenForTabOrPending() {
        return false;
    }

    @CalledByNative
    public ContentVideoViewEmbedder getContentVideoViewEmbedder() {
        return null;
    }

    /**
     * Called when BrowserMediaPlayerManager wants to load a media resource.
     * @param url the URL of media resource to load.
     * @return true to prevent the resource from being loaded.
     */
    @CalledByNative
    public boolean shouldBlockMediaRequest(String url) {
        return false;
    }
}
