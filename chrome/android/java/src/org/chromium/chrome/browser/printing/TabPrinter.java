// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.printing;

import android.text.TextUtils;

import org.chromium.base.ContextUtils;
import org.chromium.base.Log;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.printing.Printable;

import java.lang.ref.WeakReference;

/**
 * Wraps printing related functionality of a {@link Tab} object.
 *
 * This class doesn't have any lifetime expectations with regards to Tab, since we keep a weak
 * reference.
 */
public class TabPrinter implements Printable {
    private static final String TAG = "printing";

    private final WeakReference<Tab> mTab;
    private final String mDefaultTitle;

    public TabPrinter(Tab tab) {
        mTab = new WeakReference<Tab>(tab);
        mDefaultTitle = ContextUtils.getApplicationContext().getResources().getString(
                R.string.menu_print);
    }

    @Override
    public boolean print(int renderProcessId, int renderFrameId) {
        if (!canPrint()) return false;
        Tab tab = mTab.get();
        assert tab != null && tab.isInitialized();
        return tab.print(renderProcessId, renderFrameId);
    }

    @Override
    public String getTitle() {
        Tab tab = mTab.get();
        if (tab == null) return mDefaultTitle;

        String title = tab.getTitle();
        if (!TextUtils.isEmpty(title)) return title;

        String url = tab.getUrl();
        if (!TextUtils.isEmpty(url)) return url;

        return mDefaultTitle;
    }

    @Override
    public boolean canPrint() {
        Tab tab = mTab.get();
        if (tab == null || !tab.isInitialized()) {
            // tab.isInitialized() will be false if tab is in destroy process.
            Log.d(TAG, "Tab is not avaliable for printing.");
            return false;
        }
        return true;
    }
}
