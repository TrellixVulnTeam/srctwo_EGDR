// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
package org.chromium.net.impl;

import android.support.annotation.IntDef;

import static org.chromium.net.UrlRequest.Builder.REQUEST_PRIORITY_HIGHEST;
import static org.chromium.net.UrlRequest.Builder.REQUEST_PRIORITY_IDLE;
import static org.chromium.net.UrlRequest.Builder.REQUEST_PRIORITY_LOW;
import static org.chromium.net.UrlRequest.Builder.REQUEST_PRIORITY_LOWEST;
import static org.chromium.net.UrlRequest.Builder.REQUEST_PRIORITY_MEDIUM;

import org.chromium.net.BidirectionalStream;
import org.chromium.net.ExperimentalBidirectionalStream;
import org.chromium.net.ExperimentalCronetEngine;
import org.chromium.net.ExperimentalUrlRequest;
import org.chromium.net.UrlRequest;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.net.URL;
import java.util.Collection;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Executor;

/**
 * Base class of {@link CronetUrlRequestContext} and {@link JavaCronetEngine} that contains
 * shared logic.
 */
public abstract class CronetEngineBase extends ExperimentalCronetEngine {
    /**
     * Creates a {@link UrlRequest} object. All callbacks will
     * be called on {@code executor}'s thread. {@code executor} must not run
     * tasks on the current thread to prevent blocking networking operations
     * and causing exceptions during shutdown.
     *
     * @param url {@link URL} for the request.
     * @param callback callback object that gets invoked on different events.
     * @param executor {@link Executor} on which all callbacks will be invoked.
     * @param priority priority of the request which should be one of the
     *         {@link UrlRequest.Builder#REQUEST_PRIORITY_IDLE REQUEST_PRIORITY_*}
     *         values.
     * @param requestAnnotations Objects to pass on to
     *        {@link org.chromium.net.RequestFinishedInfo.Listener}.
     * @param disableCache disables cache for the request.
     *         If context is not set up to use cache this param has no effect.
     * @param disableConnectionMigration disables connection migration for this
     *         request if it is enabled for the session.
     * @param allowDirectExecutor whether executors used by this request are permitted
     *         to execute submitted tasks inline.
     * @return new request.
     */
    protected abstract UrlRequestBase createRequest(String url, UrlRequest.Callback callback,
            Executor executor, @RequestPriority int priority, Collection<Object> requestAnnotations,
            boolean disableCache, boolean disableConnectionMigration, boolean allowDirectExecutor);

    /**
     * Creates a {@link BidirectionalStream} object. {@code callback} methods will
     * be invoked on {@code executor}. {@code executor} must not run
     * tasks on the current thread to prevent blocking networking operations
     * and causing exceptions during shutdown.
     *
     * @param url the URL for the stream
     * @param callback the object whose methods get invoked upon different events
     * @param executor the {@link Executor} on which all callbacks will be called
     * @param httpMethod the HTTP method to use for the stream
     * @param requestHeaders the list of request headers
     * @param priority priority of the stream which should be one of the
     *         {@link BidirectionalStream.Builder#STREAM_PRIORITY_IDLE STREAM_PRIORITY_*}
     *         values.
     * @param delayRequestHeadersUntilFirstFlush whether to delay sending request
     *         headers until flush() is called, and try to combine them
     *         with the next data frame.
     * @param requestAnnotations Objects to pass on to
     *       {@link org.chromium.net.RequestFinishedInfo.Listener}.
     * @return a new stream.
     */
    protected abstract ExperimentalBidirectionalStream createBidirectionalStream(String url,
            BidirectionalStream.Callback callback, Executor executor, String httpMethod,
            List<Map.Entry<String, String>> requestHeaders, @StreamPriority int priority,
            boolean delayRequestHeadersUntilFirstFlush, Collection<Object> requestAnnotations);

    @Override
    public ExperimentalUrlRequest.Builder newUrlRequestBuilder(
            String url, UrlRequest.Callback callback, Executor executor) {
        return new UrlRequestBuilderImpl(url, callback, executor, this);
    }

    @IntDef({
            REQUEST_PRIORITY_IDLE, REQUEST_PRIORITY_LOWEST, REQUEST_PRIORITY_LOW,
            REQUEST_PRIORITY_MEDIUM, REQUEST_PRIORITY_HIGHEST,
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface RequestPriority {}

    @IntDef({
            BidirectionalStream.Builder.STREAM_PRIORITY_IDLE,
            BidirectionalStream.Builder.STREAM_PRIORITY_LOWEST,
            BidirectionalStream.Builder.STREAM_PRIORITY_LOW,
            BidirectionalStream.Builder.STREAM_PRIORITY_MEDIUM,
            BidirectionalStream.Builder.STREAM_PRIORITY_HIGHEST,
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface StreamPriority {}
}
