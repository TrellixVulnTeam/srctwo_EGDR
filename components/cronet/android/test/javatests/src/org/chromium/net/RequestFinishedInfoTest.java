// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.net;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import static org.chromium.base.CollectionUtil.newHashSet;
import static org.chromium.net.CronetTestRule.getContext;

import android.os.ConditionVariable;
import android.support.test.filters.SmallTest;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.chromium.base.test.BaseJUnit4ClassRunner;
import org.chromium.base.test.util.Feature;
import org.chromium.net.CronetTestRule.CronetTestFramework;
import org.chromium.net.CronetTestRule.OnlyRunNativeCronet;
import org.chromium.net.MetricsTestUtil.TestExecutor;
import org.chromium.net.MetricsTestUtil.TestRequestFinishedListener;
import org.chromium.net.impl.CronetMetrics;
import org.chromium.net.test.EmbeddedTestServer;

import java.util.ArrayList;
import java.util.Date;
import java.util.HashSet;
import java.util.List;
import java.util.concurrent.Executor;
import java.util.concurrent.RejectedExecutionException;

/**
 * Test RequestFinishedInfo.Listener and the metrics information it provides.
 */
@RunWith(BaseJUnit4ClassRunner.class)
public class RequestFinishedInfoTest {
    @Rule
    public final CronetTestRule mTestRule = new CronetTestRule();

    CronetTestFramework mTestFramework;
    private EmbeddedTestServer mTestServer;
    private String mUrl;

    // A subclass of TestRequestFinishedListener to additionally assert that UrlRequest.Callback's
    // terminal callbacks have been invoked at the time of onRequestFinished().
    // See crbug.com/710877.
    private static class AssertCallbackDoneRequestFinishedListener
            extends TestRequestFinishedListener {
        private final TestUrlRequestCallback mCallback;
        public AssertCallbackDoneRequestFinishedListener(TestUrlRequestCallback callback) {
            mCallback = callback;
        }

        @Override
        public void onRequestFinished(RequestFinishedInfo requestInfo) {
            assertTrue(mCallback.isDone());
            super.onRequestFinished(requestInfo);
        }
    };

    @Before
    public void setUp() throws Exception {
        mTestServer = EmbeddedTestServer.createAndStartServer(getContext());
        mUrl = mTestServer.getURL("/echo?status=200");
    }

    @After
    public void tearDown() throws Exception {
        mTestServer.stopAndDestroyServer();
    }

    static class DirectExecutor implements Executor {
        private ConditionVariable mBlock = new ConditionVariable();

        @Override
        public void execute(Runnable task) {
            task.run();
            mBlock.open();
        }

        public void blockUntilDone() {
            mBlock.block();
        }
    }

    static class ThreadExecutor implements Executor {
        private List<Thread> mThreads = new ArrayList<Thread>();

        @Override
        public void execute(Runnable task) {
            Thread newThread = new Thread(task);
            mThreads.add(newThread);
            newThread.start();
        }

        public void joinAll() throws InterruptedException {
            for (Thread thread : mThreads) {
                thread.join();
            }
        }
    }

    @Test
    @SmallTest
    @OnlyRunNativeCronet
    @Feature({"Cronet"})
    @SuppressWarnings("deprecation")
    public void testRequestFinishedListener() throws Exception {
        mTestFramework = mTestRule.startCronetTestFramework();
        TestRequestFinishedListener requestFinishedListener = new TestRequestFinishedListener();
        mTestFramework.mCronetEngine.addRequestFinishedListener(requestFinishedListener);
        TestUrlRequestCallback callback = new TestUrlRequestCallback();
        ExperimentalUrlRequest.Builder urlRequestBuilder =
                (ExperimentalUrlRequest.Builder) mTestFramework.mCronetEngine.newUrlRequestBuilder(
                        mUrl, callback, callback.getExecutor());
        Date startTime = new Date();
        urlRequestBuilder.addRequestAnnotation("request annotation")
                .addRequestAnnotation(this)
                .build()
                .start();
        callback.blockForDone();
        requestFinishedListener.blockUntilDone();
        Date endTime = new Date();

        RequestFinishedInfo requestInfo = requestFinishedListener.getRequestInfo();
        MetricsTestUtil.checkRequestFinishedInfo(requestInfo, mUrl, startTime, endTime);
        assertEquals(RequestFinishedInfo.SUCCEEDED, requestInfo.getFinishedReason());
        MetricsTestUtil.checkHasConnectTiming(requestInfo.getMetrics(), startTime, endTime, false);
        assertEquals(newHashSet("request annotation", this), // Use sets for unordered comparison.
                new HashSet<Object>(requestInfo.getAnnotations()));
        mTestFramework.mCronetEngine.shutdown();
    }

    @Test
    @SmallTest
    @OnlyRunNativeCronet
    @Feature({"Cronet"})
    @SuppressWarnings("deprecation")
    public void testRequestFinishedListenerDirectExecutor() throws Exception {
        mTestFramework = mTestRule.startCronetTestFramework();
        DirectExecutor testExecutor = new DirectExecutor();
        TestRequestFinishedListener requestFinishedListener =
                new TestRequestFinishedListener(testExecutor);
        mTestFramework.mCronetEngine.addRequestFinishedListener(requestFinishedListener);
        TestUrlRequestCallback callback = new TestUrlRequestCallback();
        ExperimentalUrlRequest.Builder urlRequestBuilder =
                (ExperimentalUrlRequest.Builder) mTestFramework.mCronetEngine.newUrlRequestBuilder(
                        mUrl, callback, callback.getExecutor());
        Date startTime = new Date();
        urlRequestBuilder.addRequestAnnotation("request annotation")
                .addRequestAnnotation(this)
                .build()
                .start();
        callback.blockForDone();
        // Block on the executor, not the listener, since blocking on the listener doesn't work when
        // it's created with a non-default executor.
        testExecutor.blockUntilDone();
        Date endTime = new Date();

        RequestFinishedInfo requestInfo = requestFinishedListener.getRequestInfo();
        MetricsTestUtil.checkRequestFinishedInfo(requestInfo, mUrl, startTime, endTime);
        assertEquals(RequestFinishedInfo.SUCCEEDED, requestInfo.getFinishedReason());
        MetricsTestUtil.checkHasConnectTiming(requestInfo.getMetrics(), startTime, endTime, false);
        assertEquals(newHashSet("request annotation", this), // Use sets for unordered comparison.
                new HashSet<Object>(requestInfo.getAnnotations()));
        mTestFramework.mCronetEngine.shutdown();
    }

    @Test
    @SmallTest
    @OnlyRunNativeCronet
    @Feature({"Cronet"})
    @SuppressWarnings("deprecation")
    public void testRequestFinishedListenerDifferentThreads() throws Exception {
        mTestFramework = mTestRule.startCronetTestFramework();
        TestRequestFinishedListener firstListener = new TestRequestFinishedListener();
        TestRequestFinishedListener secondListener = new TestRequestFinishedListener();
        mTestFramework.mCronetEngine.addRequestFinishedListener(firstListener);
        mTestFramework.mCronetEngine.addRequestFinishedListener(secondListener);
        TestUrlRequestCallback callback = new TestUrlRequestCallback();
        ExperimentalUrlRequest.Builder urlRequestBuilder =
                (ExperimentalUrlRequest.Builder) mTestFramework.mCronetEngine.newUrlRequestBuilder(
                        mUrl, callback, callback.getExecutor());
        Date startTime = new Date();
        urlRequestBuilder.addRequestAnnotation("request annotation")
                .addRequestAnnotation(this)
                .build()
                .start();
        callback.blockForDone();
        firstListener.blockUntilDone();
        secondListener.blockUntilDone();
        Date endTime = new Date();

        RequestFinishedInfo firstRequestInfo = firstListener.getRequestInfo();
        RequestFinishedInfo secondRequestInfo = secondListener.getRequestInfo();

        MetricsTestUtil.checkRequestFinishedInfo(firstRequestInfo, mUrl, startTime, endTime);
        assertEquals(RequestFinishedInfo.SUCCEEDED, firstRequestInfo.getFinishedReason());
        MetricsTestUtil.checkHasConnectTiming(
                firstRequestInfo.getMetrics(), startTime, endTime, false);

        MetricsTestUtil.checkRequestFinishedInfo(secondRequestInfo, mUrl, startTime, endTime);
        assertEquals(RequestFinishedInfo.SUCCEEDED, secondRequestInfo.getFinishedReason());
        MetricsTestUtil.checkHasConnectTiming(
                secondRequestInfo.getMetrics(), startTime, endTime, false);

        assertEquals(newHashSet("request annotation", this), // Use sets for unordered comparison.
                new HashSet<Object>(firstRequestInfo.getAnnotations()));
        assertEquals(newHashSet("request annotation", this),
                new HashSet<Object>(secondRequestInfo.getAnnotations()));
        mTestFramework.mCronetEngine.shutdown();
    }

    @Test
    @SmallTest
    @OnlyRunNativeCronet
    @Feature({"Cronet"})
    @SuppressWarnings("deprecation")
    public void testRequestFinishedListenerFailedRequest() throws Exception {
        String connectionRefusedUrl = "http://127.0.0.1:3";
        mTestFramework = mTestRule.startCronetTestFramework();
        TestRequestFinishedListener requestFinishedListener = new TestRequestFinishedListener();
        mTestFramework.mCronetEngine.addRequestFinishedListener(requestFinishedListener);
        TestUrlRequestCallback callback = new TestUrlRequestCallback();
        UrlRequest.Builder urlRequestBuilder = mTestFramework.mCronetEngine.newUrlRequestBuilder(
                connectionRefusedUrl, callback, callback.getExecutor());
        Date startTime = new Date();
        urlRequestBuilder.build().start();
        callback.blockForDone();
        assertTrue(callback.mOnErrorCalled);
        requestFinishedListener.blockUntilDone();
        Date endTime = new Date();

        RequestFinishedInfo requestInfo = requestFinishedListener.getRequestInfo();
        assertNotNull("RequestFinishedInfo.Listener must be called", requestInfo);
        assertEquals(connectionRefusedUrl, requestInfo.getUrl());
        assertTrue(requestInfo.getAnnotations().isEmpty());
        assertEquals(RequestFinishedInfo.FAILED, requestInfo.getFinishedReason());
        assertNotNull(requestInfo.getException());
        assertEquals(NetworkException.ERROR_CONNECTION_REFUSED,
                ((NetworkException) requestInfo.getException()).getErrorCode());
        RequestFinishedInfo.Metrics metrics = requestInfo.getMetrics();
        assertNotNull("RequestFinishedInfo.getMetrics() must not be null", metrics);
        // The failure is occasionally fast enough that time reported is 0, so just check for null
        assertNotNull(metrics.getTotalTimeMs());
        assertNull(metrics.getTtfbMs());

        // Check the timing metrics
        assertNotNull(metrics.getRequestStart());
        MetricsTestUtil.assertAfter(metrics.getRequestStart(), startTime);
        MetricsTestUtil.checkNoConnectTiming(metrics);
        assertNull(metrics.getSendingStart());
        assertNull(metrics.getSendingEnd());
        assertNull(metrics.getResponseStart());
        assertNotNull(metrics.getRequestEnd());
        MetricsTestUtil.assertAfter(endTime, metrics.getRequestEnd());
        MetricsTestUtil.assertAfter(metrics.getRequestEnd(), metrics.getRequestStart());
        assertTrue(metrics.getSentByteCount() == 0);
        assertTrue(metrics.getReceivedByteCount() == 0);
        mTestFramework.mCronetEngine.shutdown();
    }

    @Test
    @SmallTest
    @OnlyRunNativeCronet
    @Feature({"Cronet"})
    @SuppressWarnings("deprecation")
    public void testRequestFinishedListenerRemoved() throws Exception {
        mTestFramework = mTestRule.startCronetTestFramework();
        TestExecutor testExecutor = new TestExecutor();
        TestRequestFinishedListener requestFinishedListener =
                new TestRequestFinishedListener(testExecutor);
        mTestFramework.mCronetEngine.addRequestFinishedListener(requestFinishedListener);
        TestUrlRequestCallback callback = new TestUrlRequestCallback();
        UrlRequest.Builder urlRequestBuilder = mTestFramework.mCronetEngine.newUrlRequestBuilder(
                mUrl, callback, callback.getExecutor());
        UrlRequest request = urlRequestBuilder.build();
        mTestFramework.mCronetEngine.removeRequestFinishedListener(requestFinishedListener);
        request.start();
        callback.blockForDone();
        testExecutor.runAllTasks();

        assertNull("RequestFinishedInfo.Listener must not be called",
                requestFinishedListener.getRequestInfo());
        mTestFramework.mCronetEngine.shutdown();
    }

    @Test
    @SmallTest
    @OnlyRunNativeCronet
    @Feature({"Cronet"})
    public void testRequestFinishedListenerCanceledRequest() throws Exception {
        mTestFramework = mTestRule.startCronetTestFramework();
        TestRequestFinishedListener requestFinishedListener = new TestRequestFinishedListener();
        mTestFramework.mCronetEngine.addRequestFinishedListener(requestFinishedListener);
        TestUrlRequestCallback callback = new TestUrlRequestCallback() {
            @Override
            public void onResponseStarted(UrlRequest request, UrlResponseInfo info) {
                super.onResponseStarted(request, info);
                request.cancel();
            }
        };
        ExperimentalUrlRequest.Builder urlRequestBuilder =
                mTestFramework.mCronetEngine.newUrlRequestBuilder(
                        mUrl, callback, callback.getExecutor());
        Date startTime = new Date();
        urlRequestBuilder.addRequestAnnotation("request annotation")
                .addRequestAnnotation(this)
                .build()
                .start();
        callback.blockForDone();
        requestFinishedListener.blockUntilDone();
        Date endTime = new Date();

        RequestFinishedInfo requestInfo = requestFinishedListener.getRequestInfo();
        MetricsTestUtil.checkRequestFinishedInfo(requestInfo, mUrl, startTime, endTime);
        assertEquals(RequestFinishedInfo.CANCELED, requestInfo.getFinishedReason());
        MetricsTestUtil.checkHasConnectTiming(requestInfo.getMetrics(), startTime, endTime, false);

        assertEquals(newHashSet("request annotation", this), // Use sets for unordered comparison.
                new HashSet<Object>(requestInfo.getAnnotations()));
        mTestFramework.mCronetEngine.shutdown();
    }

    private static class RejectAllTasksExecutor implements Executor {
        @Override
        public void execute(Runnable task) {
            throw new RejectedExecutionException();
        }
    }

    // Checks that CronetURLRequestAdapter::DestroyOnNetworkThread() doesn't crash when metrics
    // collection is enabled and the URLRequest hasn't been created. See http://crbug.com/675629.
    @Test
    @SmallTest
    @OnlyRunNativeCronet
    @Feature({"Cronet"})
    public void testExceptionInRequestStart() throws Exception {
        mTestFramework = mTestRule.startCronetTestFramework();
        // The listener in this test shouldn't get any tasks.
        Executor executor = new RejectAllTasksExecutor();
        TestRequestFinishedListener requestFinishedListener =
                new TestRequestFinishedListener(executor);
        mTestFramework.mCronetEngine.addRequestFinishedListener(requestFinishedListener);
        TestUrlRequestCallback callback = new TestUrlRequestCallback();
        ExperimentalUrlRequest.Builder urlRequestBuilder =
                mTestFramework.mCronetEngine.newUrlRequestBuilder(
                        mUrl, callback, callback.getExecutor());
        // Empty headers are invalid and will cause start() to throw an exception.
        UrlRequest request = urlRequestBuilder.addHeader("", "").build();
        try {
            request.start();
            fail("UrlRequest.start() should throw IllegalArgumentException");
        } catch (IllegalArgumentException e) {
            assertEquals("Invalid header =", e.getMessage());
        }

        mTestFramework.mCronetEngine.shutdown();
    }

    @Test
    @SmallTest
    @Feature({"Cronet"})
    public void testMetricsGetters() throws Exception {
        long requestStart = 1;
        long dnsStart = 2;
        long dnsEnd = -1;
        long connectStart = 4;
        long connectEnd = 5;
        long sslStart = 6;
        long sslEnd = 7;
        long sendingStart = 8;
        long sendingEnd = 9;
        long pushStart = 10;
        long pushEnd = 11;
        long responseStart = 12;
        long requestEnd = 13;
        boolean socketReused = true;
        long sentByteCount = 14;
        long receivedByteCount = 15;
        // Make sure nothing gets reordered inside the Metrics class
        RequestFinishedInfo.Metrics metrics = new CronetMetrics(requestStart, dnsStart, dnsEnd,
                connectStart, connectEnd, sslStart, sslEnd, sendingStart, sendingEnd, pushStart,
                pushEnd, responseStart, requestEnd, socketReused, sentByteCount, receivedByteCount);
        assertEquals(new Date(requestStart), metrics.getRequestStart());
        // -1 timestamp should translate to null
        assertNull(metrics.getDnsEnd());
        assertEquals(new Date(dnsStart), metrics.getDnsStart());
        assertEquals(new Date(connectStart), metrics.getConnectStart());
        assertEquals(new Date(connectEnd), metrics.getConnectEnd());
        assertEquals(new Date(sslStart), metrics.getSslStart());
        assertEquals(new Date(sslEnd), metrics.getSslEnd());
        assertEquals(new Date(pushStart), metrics.getPushStart());
        assertEquals(new Date(pushEnd), metrics.getPushEnd());
        assertEquals(new Date(responseStart), metrics.getResponseStart());
        assertEquals(new Date(requestEnd), metrics.getRequestEnd());
        assertEquals(socketReused, metrics.getSocketReused());
        assertEquals(sentByteCount, (long) metrics.getSentByteCount());
        assertEquals(receivedByteCount, (long) metrics.getReceivedByteCount());
    }

    @Test
    @SmallTest
    @OnlyRunNativeCronet
    @Feature({"Cronet"})
    @SuppressWarnings("deprecation")
    public void testOrderSuccessfulRequest() throws Exception {
        mTestFramework = mTestRule.startCronetTestFramework();
        final TestUrlRequestCallback callback = new TestUrlRequestCallback();
        TestRequestFinishedListener requestFinishedListener =
                new AssertCallbackDoneRequestFinishedListener(callback);
        mTestFramework.mCronetEngine.addRequestFinishedListener(requestFinishedListener);
        ExperimentalUrlRequest.Builder urlRequestBuilder =
                (ExperimentalUrlRequest.Builder) mTestFramework.mCronetEngine.newUrlRequestBuilder(
                        mUrl, callback, callback.getExecutor());
        Date startTime = new Date();
        urlRequestBuilder.addRequestAnnotation("request annotation")
                .addRequestAnnotation(this)
                .build()
                .start();
        callback.blockForDone();
        requestFinishedListener.blockUntilDone();
        Date endTime = new Date();

        RequestFinishedInfo requestInfo = requestFinishedListener.getRequestInfo();
        MetricsTestUtil.checkRequestFinishedInfo(requestInfo, mUrl, startTime, endTime);
        assertEquals(RequestFinishedInfo.SUCCEEDED, requestInfo.getFinishedReason());
        MetricsTestUtil.checkHasConnectTiming(requestInfo.getMetrics(), startTime, endTime, false);
        assertEquals(newHashSet("request annotation", this), // Use sets for unordered comparison.
                new HashSet<Object>(requestInfo.getAnnotations()));
        mTestFramework.mCronetEngine.shutdown();
    }

    @Test
    @SmallTest
    @OnlyRunNativeCronet
    @Feature({"Cronet"})
    // Tests a failed request where the error originates from Java.
    public void testOrderFailedRequestJava() throws Exception {
        mTestFramework = mTestRule.startCronetTestFramework();
        final TestUrlRequestCallback callback = new TestUrlRequestCallback() {
            @Override
            public void onResponseStarted(UrlRequest request, UrlResponseInfo info) {
                throw new RuntimeException("make this request fail");
            }
        };
        TestRequestFinishedListener requestFinishedListener =
                new AssertCallbackDoneRequestFinishedListener(callback);
        mTestFramework.mCronetEngine.addRequestFinishedListener(requestFinishedListener);
        UrlRequest.Builder urlRequestBuilder = mTestFramework.mCronetEngine.newUrlRequestBuilder(
                mUrl, callback, callback.getExecutor());
        urlRequestBuilder.build().start();
        callback.blockForDone();
        assertTrue(callback.mOnErrorCalled);
        requestFinishedListener.blockUntilDone();
        RequestFinishedInfo requestInfo = requestFinishedListener.getRequestInfo();
        assertNotNull("RequestFinishedInfo.Listener must be called", requestInfo);
        assertEquals(mUrl, requestInfo.getUrl());
        assertTrue(requestInfo.getAnnotations().isEmpty());
        assertEquals(RequestFinishedInfo.FAILED, requestInfo.getFinishedReason());
        assertNotNull(requestInfo.getException());
        assertEquals("Exception received from UrlRequest.Callback",
                requestInfo.getException().getMessage());
        RequestFinishedInfo.Metrics metrics = requestInfo.getMetrics();
        assertNotNull("RequestFinishedInfo.getMetrics() must not be null", metrics);
        mTestFramework.mCronetEngine.shutdown();
    }

    @Test
    @SmallTest
    @OnlyRunNativeCronet
    @Feature({"Cronet"})
    // Tests a failed request where the error originates from native code.
    public void testOrderFailedRequestNative() throws Exception {
        String connectionRefusedUrl = "http://127.0.0.1:3";
        mTestFramework = mTestRule.startCronetTestFramework();
        final TestUrlRequestCallback callback = new TestUrlRequestCallback();
        TestRequestFinishedListener requestFinishedListener =
                new AssertCallbackDoneRequestFinishedListener(callback);
        mTestFramework.mCronetEngine.addRequestFinishedListener(requestFinishedListener);
        UrlRequest.Builder urlRequestBuilder = mTestFramework.mCronetEngine.newUrlRequestBuilder(
                connectionRefusedUrl, callback, callback.getExecutor());
        urlRequestBuilder.build().start();
        callback.blockForDone();
        assertTrue(callback.mOnErrorCalled);
        requestFinishedListener.blockUntilDone();
        RequestFinishedInfo requestInfo = requestFinishedListener.getRequestInfo();
        assertNotNull("RequestFinishedInfo.Listener must be called", requestInfo);
        assertEquals(connectionRefusedUrl, requestInfo.getUrl());
        assertTrue(requestInfo.getAnnotations().isEmpty());
        assertEquals(RequestFinishedInfo.FAILED, requestInfo.getFinishedReason());
        assertNotNull(requestInfo.getException());
        assertEquals(NetworkException.ERROR_CONNECTION_REFUSED,
                ((NetworkException) requestInfo.getException()).getErrorCode());
        RequestFinishedInfo.Metrics metrics = requestInfo.getMetrics();
        assertNotNull("RequestFinishedInfo.getMetrics() must not be null", metrics);
        mTestFramework.mCronetEngine.shutdown();
    }

    @Test
    @SmallTest
    @OnlyRunNativeCronet
    @Feature({"Cronet"})
    public void testOrderCanceledRequest() throws Exception {
        mTestFramework = mTestRule.startCronetTestFramework();
        final TestUrlRequestCallback callback = new TestUrlRequestCallback() {
            @Override
            public void onResponseStarted(UrlRequest request, UrlResponseInfo info) {
                super.onResponseStarted(request, info);
                request.cancel();
            }
        };

        TestRequestFinishedListener requestFinishedListener =
                new AssertCallbackDoneRequestFinishedListener(callback);
        mTestFramework.mCronetEngine.addRequestFinishedListener(requestFinishedListener);
        ExperimentalUrlRequest.Builder urlRequestBuilder =
                mTestFramework.mCronetEngine.newUrlRequestBuilder(
                        mUrl, callback, callback.getExecutor());
        Date startTime = new Date();
        urlRequestBuilder.addRequestAnnotation("request annotation")
                .addRequestAnnotation(this)
                .build()
                .start();
        callback.blockForDone();
        requestFinishedListener.blockUntilDone();
        Date endTime = new Date();

        RequestFinishedInfo requestInfo = requestFinishedListener.getRequestInfo();
        MetricsTestUtil.checkRequestFinishedInfo(requestInfo, mUrl, startTime, endTime);
        assertEquals(RequestFinishedInfo.CANCELED, requestInfo.getFinishedReason());
        MetricsTestUtil.checkHasConnectTiming(requestInfo.getMetrics(), startTime, endTime, false);

        assertEquals(newHashSet("request annotation", this), // Use sets for unordered comparison.
                new HashSet<Object>(requestInfo.getAnnotations()));
        mTestFramework.mCronetEngine.shutdown();
    }
}
