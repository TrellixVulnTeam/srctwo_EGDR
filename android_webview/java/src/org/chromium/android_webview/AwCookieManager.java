// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.android_webview;

import android.os.Handler;
import android.os.Looper;
import android.webkit.ValueCallback;

import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;
import org.chromium.base.library_loader.LibraryLoader;
import org.chromium.base.library_loader.LibraryProcessType;
import org.chromium.base.library_loader.ProcessInitException;

/**
 * AwCookieManager manages cookies according to RFC2109 spec.
 *
 * Methods in this class are thread safe.
 */
@JNINamespace("android_webview")
public final class AwCookieManager {

    public AwCookieManager() {
        try {
            LibraryLoader.get(LibraryProcessType.PROCESS_WEBVIEW).ensureInitialized();
        } catch (ProcessInitException e) {
            throw new RuntimeException("Error initializing WebView library", e);
        }
    }

    /**
     * Control whether cookie is enabled or disabled
     * @param accept TRUE if accept cookie
     */
    public void setAcceptCookie(boolean accept) {
        nativeSetShouldAcceptCookies(accept);
    }

    /**
     * Return whether cookie is enabled
     * @return TRUE if accept cookie
     */
    public boolean acceptCookie() {
        return nativeGetShouldAcceptCookies();
    }

    /**
     * Synchronous version of setCookie.
     */
    public void setCookie(String url, String value) {
        nativeSetCookieSync(url, value);
    }

    /**
     * Deprecated synchronous version of removeSessionCookies.
     */
    public void removeSessionCookies() {
        nativeRemoveSessionCookiesSync();
    }

    /**
     * Deprecated synchronous version of removeAllCookies.
     */
    public void removeAllCookies() {
        nativeRemoveAllCookiesSync();
    }

    /**
     * Set cookie for a given url. The old cookie with same host/path/name will
     * be removed. The new cookie will be added if it is not expired or it does
     * not have expiration which implies it is session cookie.
     * @param url The url which cookie is set for.
     * @param value The value for set-cookie: in http response header.
     * @param callback A callback called with the success status after the cookie is set.
     */
    public void setCookie(final String url, final String value,
            final ValueCallback<Boolean> callback) {
        try {
            nativeSetCookie(url, value, CookieCallback.convert(callback));
        } catch (IllegalStateException e) {
            throw new IllegalStateException(
                    "SetCookie must be called on a thread with a running Looper.");
        }
    }

    /**
     * Get cookie(s) for a given url so that it can be set to "cookie:" in http
     * request header.
     * @param url The url needs cookie
     * @return The cookies in the format of NAME=VALUE [; NAME=VALUE]
     */
    public String getCookie(final String url) {
        String cookie = nativeGetCookie(url.toString());
        // Return null if the string is empty to match legacy behavior
        return cookie == null || cookie.trim().isEmpty() ? null : cookie;
    }

    /**
     * Remove all session cookies, the cookies without an expiration date.
     * The value of the callback is true iff at least one cookie was removed.
     * @param callback A callback called after the cookies (if any) are removed.
     */
    public void removeSessionCookies(ValueCallback<Boolean> callback) {
        try {
            nativeRemoveSessionCookies(CookieCallback.convert(callback));
        } catch (IllegalStateException e) {
            throw new IllegalStateException(
                    "removeSessionCookies must be called on a thread with a running Looper.");
        }
    }

    /**
     * Remove all cookies.
     * The value of the callback is true iff at least one cookie was removed.
     * @param callback A callback called after the cookies (if any) are removed.
     */
    public void removeAllCookies(ValueCallback<Boolean> callback) {
        try {
            nativeRemoveAllCookies(CookieCallback.convert(callback));
        } catch (IllegalStateException e) {
            throw new IllegalStateException(
                    "removeAllCookies must be called on a thread with a running Looper.");
        }
    }

    /**
     *  Return true if there are stored cookies.
     */
    public boolean hasCookies() {
        return nativeHasCookies();
    }

    /**
     * Remove all expired cookies
     */
    public void removeExpiredCookies() {
        nativeRemoveExpiredCookies();
    }

    public void flushCookieStore() {
        nativeFlushCookieStore();
    }

    /**
     * Whether cookies are accepted for file scheme URLs.
     */
    public boolean allowFileSchemeCookies() {
        return nativeAllowFileSchemeCookies();
    }

    /**
     * Sets whether cookies are accepted for file scheme URLs.
     *
     * Use of cookies with file scheme URLs is potentially insecure. Do not use this feature unless
     * you can be sure that no unintentional sharing of cookie data can take place.
     * <p>
     * Note that calls to this method will have no effect if made after a WebView or CookieManager
     * instance has been created.
     */
    public void setAcceptFileSchemeCookies(boolean accept) {
        nativeSetAcceptFileSchemeCookies(accept);
    }

    @CalledByNative
    public static void invokeBooleanCookieCallback(CookieCallback<Boolean> callback,
            boolean result) {
        callback.onReceiveValue(result);
    }

    /**
     * CookieCallback is a bridge that knows how to call a ValueCallback on its original thread.
     * We need to arrange for the users ValueCallback#onReceiveValue to be called on the original
     * thread after the work is done. When the API is called we construct a CookieCallback which
     * remembers the handler of the current thread. Later the native code uses
     * invokeBooleanCookieCallback to call CookieCallback#onReceiveValue which posts a Runnable
     * on the handler of the original thread which in turn calls ValueCallback#onReceiveValue.
     */
    private static class CookieCallback<T> {
        ValueCallback<T> mCallback;
        Handler mHandler;

        public CookieCallback(ValueCallback<T> callback, Handler handler) {
            mCallback = callback;
            mHandler = handler;
        }

        public static <T> CookieCallback<T> convert(ValueCallback<T> callback) throws
                IllegalStateException {
            if (callback == null) return null;
            if (Looper.myLooper() == null) {
                throw new IllegalStateException("CookieCallback.convert should be called on "
                        + "a thread with a running Looper.");
            }
            return new CookieCallback<T>(callback, new Handler());
        }

        public void onReceiveValue(final T t) {
            mHandler.post(() -> mCallback.onReceiveValue(t));
        }
    }

    private native void nativeSetShouldAcceptCookies(boolean accept);
    private native boolean nativeGetShouldAcceptCookies();

    private native void nativeSetCookie(String url, String value,
            CookieCallback<Boolean> callback);
    private native void nativeSetCookieSync(String url, String value);
    private native String nativeGetCookie(String url);

    private native void nativeRemoveSessionCookies(CookieCallback<Boolean> callback);
    private native void nativeRemoveSessionCookiesSync();
    private native void nativeRemoveAllCookies(CookieCallback<Boolean> callback);
    private native void nativeRemoveAllCookiesSync();
    private native void nativeRemoveExpiredCookies();
    private native void nativeFlushCookieStore();

    private native boolean nativeHasCookies();

    private native boolean nativeAllowFileSchemeCookies();
    private native void nativeSetAcceptFileSchemeCookies(boolean accept);
}
