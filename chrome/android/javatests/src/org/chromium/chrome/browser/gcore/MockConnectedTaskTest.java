// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.gcore;

import android.support.test.filters.SmallTest;

import junit.framework.TestCase;

import org.chromium.base.test.util.Feature;
import org.chromium.chrome.test.gcore.MockChromeGoogleApiClient;

/** Tests for {@link ConnectedTask} */
public class MockConnectedTaskTest extends TestCase {
    private MockChromeGoogleApiClient mClient;
    private MockConnectedTask<MockChromeGoogleApiClient> mTask;

    @Override
    protected void setUp() throws Exception {
        mClient = new MockChromeGoogleApiClient();
        mTask = new MockConnectedTask<>(mClient);
    }

    @SmallTest
    @Feature({"GCore"})
    public void testConnectionSuccess() {
        mClient.setConnectionResult(true);

        mTask.run();

        mTask.assertDoWhenConnectedCalled(1);
        mTask.assertCleanUpCalled(1);
        mTask.assertNoOtherMethodsCalled();

        mClient.assertConnectWithTimeoutCalled(1);
        mClient.assertDisconnectCalled(1);
        mClient.assertNoOtherMethodsCalled();
    }

    @SmallTest
    @Feature({"GCore"})
    public void testConnectionFailureWithGooglePlayServicesAvailable() {
        mClient.setConnectionResult(false);
        mClient.setIsGooglePlayServicesAvailable(true);

        mTask.run();

        mTask.assertRescheduleCalled(1);
        mTask.assertNoOtherMethodsCalled();

        mClient.assertConnectWithTimeoutCalled(1);
        mClient.assertIsGooglePlayServicesAvailableCalled(1);
        mClient.assertNoOtherMethodsCalled();
    }

    @SmallTest
    @Feature({"GCore"})
    public void testConnectionFailureWithGooglePlayServicesUnavailable() {
        mClient.setConnectionResult(false);
        mClient.setIsGooglePlayServicesAvailable(false);

        mTask.run();

        mTask.assertCleanUpCalled(1);
        mTask.assertNoOtherMethodsCalled();

        mClient.assertConnectWithTimeoutCalled(1);
        mClient.assertIsGooglePlayServicesAvailableCalled(1);
        mClient.assertNoOtherMethodsCalled();
    }

    @SmallTest
    @Feature({"GCore"})
    public void testRetryLimit() {
        // Task rescheduled ConnectedTask.RETRY_NUMBER_LIMIT - 1 times.
        for (int i = 0; i < ConnectedTask.RETRY_NUMBER_LIMIT - 1; i++) {
            testConnectionFailureWithGooglePlayServicesAvailable();
        }

        mTask.run();

        mTask.assertCleanUpCalled(1);
        mTask.assertNoOtherMethodsCalled();

        mClient.assertConnectWithTimeoutCalled(1);
        mClient.assertNoOtherMethodsCalled();
    }
}
