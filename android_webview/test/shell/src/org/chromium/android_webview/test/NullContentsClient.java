// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.android_webview.test;

import android.graphics.Bitmap;
import android.graphics.Picture;
import android.net.http.SslError;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.webkit.ConsoleMessage;
import android.webkit.GeolocationPermissions;
import android.webkit.ValueCallback;
import android.webkit.WebChromeClient;

import org.chromium.android_webview.AwContentsClient;
import org.chromium.android_webview.AwContentsClientBridge;
import org.chromium.android_webview.AwHttpAuthHandler;
import org.chromium.android_webview.AwRenderProcessGoneDetail;
import org.chromium.android_webview.AwSafeBrowsingResponse;
import org.chromium.android_webview.AwWebResourceResponse;
import org.chromium.android_webview.JsPromptResultReceiver;
import org.chromium.android_webview.JsResultReceiver;
import org.chromium.android_webview.SafeBrowsingAction;
import org.chromium.android_webview.permission.AwPermissionRequest;
import org.chromium.base.ThreadUtils;

import java.security.Principal;

/**
 * As a convience for tests that only care about specefic callbacks, this class provides
 * empty implementations of all abstract methods.
 */
public class NullContentsClient extends AwContentsClient {

    private static final String TAG = "NullContentsClient";

    public NullContentsClient() {
        this(ThreadUtils.getUiThreadLooper());
    }

    public NullContentsClient(Looper looper) {
        super(looper);  // "...beams are gonna blind me".
    }

    @Override
    public boolean hasWebViewClient() {
        return true;
    }

    @Override
    public boolean shouldOverrideUrlLoading(AwContentsClient.AwWebResourceRequest request) {
        return false;
    }

    @Override
    public void onUnhandledKeyEvent(KeyEvent event) {
    }

    @Override
    public void getVisitedHistory(ValueCallback<String[]> callback) {
    }

    @Override
    public void doUpdateVisitedHistory(String url, boolean isReload) {
    }

    @Override
    public void onProgressChanged(int progress) {
    }

    @Override
    public AwWebResourceResponse shouldInterceptRequest(
            AwContentsClient.AwWebResourceRequest request) {
        return null;
    }

    @Override
    public boolean shouldOverrideKeyEvent(KeyEvent event) {
        return false;
    }

    @Override
    public void onLoadResource(String url) {
    }

    @Override
    public boolean onConsoleMessage(ConsoleMessage consoleMessage) {
        return false;
    }

    @Override
    public void onReceivedHttpAuthRequest(AwHttpAuthHandler handler, String host, String realm) {
        handler.cancel();
    }

    @Override
    public void onReceivedSslError(ValueCallback<Boolean> callback, SslError error) {
        callback.onReceiveValue(false);
    }

    @Override
    public void onReceivedClientCertRequest(
            final AwContentsClientBridge.ClientCertificateRequestCallback callback,
            final String[] keyTypes, final Principal[] principals, final String host,
            final int port) {
        callback.proceed(null, null);
    }

    @Override
    public void onReceivedLoginRequest(String realm, String account, String args) {
    }

    @Override
    public void showFileChooser(ValueCallback<String[]> uploadFilePathsCallback,
            FileChooserParamsImpl fileChooserParams) {
    }

    @Override
    public void onGeolocationPermissionsShowPrompt(String origin,
            GeolocationPermissions.Callback callback) {
    }

    @Override
    public void onGeolocationPermissionsHidePrompt() {
    }

    @Override
    public void handleJsAlert(String url, String message, JsResultReceiver receiver) {
        Log.i(TAG, "handleJsAlert(" + url + ", " + message + ")");
        receiver.cancel();
    }

    @Override
    public void handleJsBeforeUnload(String url, String message, JsResultReceiver receiver) {
        Log.i(TAG, "handleJsBeforeUnload(" + url + ", " + message + ")");
        receiver.confirm();
    }

    @Override
    public void handleJsConfirm(String url, String message, JsResultReceiver receiver) {
        Log.i(TAG, "handleJsConfirm(" + url + ", " + message + ")");
        receiver.cancel();
    }

    @Override
    public void handleJsPrompt(
            String url, String message, String defaultValue, JsPromptResultReceiver receiver) {
        Log.i(TAG, "handleJsPrompt(" + url + ", " + message + ")");
        receiver.cancel();
    }

    @Override
    public void onFindResultReceived(int activeMatchOrdinal, int numberOfMatches,
            boolean isDoneCounting) {
    }

    @Override
    public void onNewPicture(Picture picture) {
    }

    @Override
    public void onPageStarted(String url) {
    }

    @Override
    public void onPageFinished(String url) {
    }

    @Override
    public void onPageCommitVisible(String url) {
    }

    @Override
    public void onReceivedError(int errorCode, String description, String failingUrl) {
    }

    @Override
    public void onReceivedError2(AwWebResourceRequest request, AwWebResourceError error) {
    }

    @Override
    public void onSafeBrowsingHit(AwWebResourceRequest request, int threatType,
            ValueCallback<AwSafeBrowsingResponse> callback) {
        callback.onReceiveValue(new AwSafeBrowsingResponse(SafeBrowsingAction.SHOW_INTERSTITIAL,
                /* reporting */ true));
    }

    @Override
    public void onReceivedHttpError(AwWebResourceRequest request, AwWebResourceResponse response) {
    }

    @Override
    public void onFormResubmission(Message dontResend, Message resend) {
        dontResend.sendToTarget();
    }

    @Override
    public void onDownloadStart(String url,
                                String userAgent,
                                String contentDisposition,
                                String mimeType,
                                long contentLength) {
    }

    @Override
    public boolean onCreateWindow(boolean isDialog, boolean isUserGesture) {
        return false;
    }

    @Override
    public void onCloseWindow() {
    }

    @Override
    public void onRequestFocus() {
    }

    @Override
    public void onReceivedTouchIconUrl(String url, boolean precomposed) {
    }

    @Override
    public void onReceivedIcon(Bitmap bitmap) {
    }

    @Override
    public void onReceivedTitle(String title) {
    }

    @Override
    public void onShowCustomView(View view, WebChromeClient.CustomViewCallback callback) {
    }

    @Override
    public void onHideCustomView() {
    }

    @Override
    public void onScaleChangedScaled(float oldScale, float newScale) {
    }

    @Override
    protected View getVideoLoadingProgressView() {
        return null;
    }

    @Override
    public Bitmap getDefaultVideoPoster() {
        return null;
    }

    @Override
    public void onPermissionRequest(AwPermissionRequest awPermissionRequest) {
        awPermissionRequest.deny();
    }

    @Override
    public void onPermissionRequestCanceled(AwPermissionRequest awPermissionRequest) {
    }

    @Override
    public boolean onRenderProcessGone(AwRenderProcessGoneDetail detail) {
        return false;
    }
}
