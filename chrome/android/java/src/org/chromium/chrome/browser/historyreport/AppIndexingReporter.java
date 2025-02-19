// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.historyreport;

import org.chromium.blink.mojom.document_metadata.WebPage;
import org.chromium.chrome.browser.AppHooks;

/** Base class for reporting entities to App Indexing. */
public class AppIndexingReporter {
    private static final String TAG = "AppIndexingReporter";
    private static AppIndexingReporter sInstance;

    public static AppIndexingReporter getInstance() {
        if (sInstance == null) {
            sInstance = AppHooks.get().createAppIndexingReporter();
        }
        return sInstance;
    }

    /**
     * Reports provided entity to on-device index.
     * Base class does not implement any reporting, and call is a no-op. Child classes should
     * implement this functionality.
     */
    public void reportWebPage(WebPage webpage) {
        // Overriden by private class. Base class does nothing.
    }

    /**
     * Reports view of provided url to on-device index.
     * Base class does not implement any reporting, and call is a no-op. Child classes should
     * implement this functionality.
     */
    public void reportWebPageView(String url, String title) {
        // Overriden by private class. Base class does nothing.
    }

    /**
     * Clears history of reported entities.
     * Currently, we do not support clearing only a subset of history. Base class does not implement
     * any reporting, and call is a no-op. Child classes should implement this functionality.
     */
    public void clearHistory() {
        // Overriden by private class. Base class does nothing.
    }
}
