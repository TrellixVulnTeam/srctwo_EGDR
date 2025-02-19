// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
package org.chromium.chrome.test.util.browser;

import android.text.TextUtils;

import org.chromium.base.ThreadUtils;
import org.chromium.base.test.util.CallbackHelper;
import org.chromium.chrome.browser.tab.EmptyTabObserver;
import org.chromium.chrome.browser.tab.Tab;

import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

/**
 * A utility class to get notified of title change in a tab.
 */
public class TabTitleObserver extends EmptyTabObserver {
    private final String mExpectedTitle;
    private final CallbackHelper mCallback;

    /**
     * A constructor.
     *
     * @param tab The tab to be observed.
     * @param expectedTitle The expected title to wait for.
     */
    public TabTitleObserver(final Tab tab, final String expectedTitle) {
        mExpectedTitle = expectedTitle;
        mCallback = new CallbackHelper();
        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                if (!notifyCallbackIfTitleMatches(tab)) {
                    tab.addObserver(TabTitleObserver.this);
                }
            }
        });
    }

    /**
     * Wait for title update up to the given number of seconds.
     *
     * @param seconds The number of seconds to wait.
     * @throws InterruptedException
     * @throws TimeoutException
     */
    public void waitForTitleUpdate(int seconds) throws InterruptedException, TimeoutException {
        mCallback.waitForCallback(0, 1, seconds, TimeUnit.SECONDS);
    }

    private boolean notifyCallbackIfTitleMatches(Tab tab) {
        if (TextUtils.equals(tab.getTitle(), mExpectedTitle)) {
            mCallback.notifyCalled();
            return true;
        }
        return false;
    }

    @Override
    public void onTitleUpdated(Tab tab) {
        notifyCallbackIfTitleMatches(tab);
    }
}