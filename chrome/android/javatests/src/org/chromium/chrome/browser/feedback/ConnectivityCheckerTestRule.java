// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.feedback;

import android.support.test.InstrumentationRegistry;

import org.junit.runner.Description;
import org.junit.runners.model.Statement;

import org.chromium.chrome.browser.test.ChromeBrowserTestRule;
import org.chromium.net.test.EmbeddedTestServer;

/**
 * Base class for tests related to checking connectivity.
 *
 * It includes a {@link ConnectivityTestServer} which is set up and torn down automatically
 * for tests.
 */
public class ConnectivityCheckerTestRule extends ChromeBrowserTestRule {
    public static final int TIMEOUT_MS = 5000;

    private EmbeddedTestServer mTestServer;
    private String mGenerated200Url;
    private String mGenerated204Url;
    private String mGenerated302Url;
    private String mGenerated404Url;
    private String mGeneratedSlowUrl;

    @Override
    public Statement apply(final Statement base, Description description) {
        return super.apply(new Statement() {
            @Override
            public void evaluate() throws Throwable {
                setUp();
                try {
                    base.evaluate();
                } finally {
                    tearDown();
                }
            }
        }, description);
    }

    public String getGenerated200Url() {
        return mGenerated200Url;
    }
    public String getGenerated204Url() {
        return mGenerated204Url;
    }
    public String getGenerated302Url() {
        return mGenerated302Url;
    }
    public String getGenerated404Url() {
        return mGenerated404Url;
    }
    public String getGeneratedSlowUrl() {
        return mGeneratedSlowUrl;
    }

    private void setUp() throws Exception {
        mTestServer = EmbeddedTestServer.createAndStartServer(
                InstrumentationRegistry.getInstrumentation().getContext());
        mGenerated200Url = mTestServer.getURL("/echo?status=200");
        mGenerated204Url = mTestServer.getURL("/echo?status=204");
        mGenerated302Url = mTestServer.getURL("/echo?status=302");
        mGenerated404Url = mTestServer.getURL("/echo?status=404");
        mGeneratedSlowUrl = mTestServer.getURL("/slow?5");
    }

    private void tearDown() throws Exception {
        mTestServer.stopAndDestroyServer();
    }
}
