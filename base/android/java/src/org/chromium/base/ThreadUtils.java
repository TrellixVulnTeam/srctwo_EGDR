// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.base;

import android.os.Handler;
import android.os.Looper;
import android.os.Process;

import org.chromium.base.annotations.CalledByNative;

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.FutureTask;

/**
 * Helper methods to deal with threading related tasks.
 */
public class ThreadUtils {

    private static final Object sLock = new Object();

    private static boolean sWillOverride;

    private static Handler sUiThreadHandler;

    private static boolean sThreadAssertsDisabled;

    public static void setWillOverrideUiThread() {
        synchronized (sLock) {
            sWillOverride = true;
        }
    }

    @VisibleForTesting
    public static void setUiThread(Looper looper) {
        synchronized (sLock) {
            if (looper == null) {
                // Used to reset the looper after tests.
                sUiThreadHandler = null;
                return;
            }
            if (sUiThreadHandler != null && sUiThreadHandler.getLooper() != looper) {
                throw new RuntimeException("UI thread looper is already set to "
                        + sUiThreadHandler.getLooper() + " (Main thread looper is "
                        + Looper.getMainLooper() + "), cannot set to new looper " + looper);
            } else {
                sUiThreadHandler = new Handler(looper);
            }
        }
    }

    private static Handler getUiThreadHandler() {
        synchronized (sLock) {
            if (sUiThreadHandler == null) {
                if (sWillOverride) {
                    throw new RuntimeException("Did not yet override the UI thread");
                }
                sUiThreadHandler = new Handler(Looper.getMainLooper());
            }
            return sUiThreadHandler;
        }
    }

    /**
     * Run the supplied Runnable on the main thread. The method will block until the Runnable
     * completes.
     *
     * @param r The Runnable to run.
     */
    public static void runOnUiThreadBlocking(final Runnable r) {
        if (runningOnUiThread()) {
            r.run();
        } else {
            FutureTask<Void> task = new FutureTask<Void>(r, null);
            postOnUiThread(task);
            try {
                task.get();
            } catch (Exception e) {
                throw new RuntimeException("Exception occurred while waiting for runnable", e);
            }
        }
    }

    /**
     * Run the supplied Callable on the main thread, wrapping any exceptions in a RuntimeException.
     * The method will block until the Callable completes.
     *
     * @param c The Callable to run
     * @return The result of the callable
     */
    @VisibleForTesting
    public static <T> T runOnUiThreadBlockingNoException(Callable<T> c) {
        try {
            return runOnUiThreadBlocking(c);
        } catch (ExecutionException e) {
            throw new RuntimeException("Error occurred waiting for callable", e);
        }
    }

    /**
     * Run the supplied Callable on the main thread, The method will block until the Callable
     * completes.
     *
     * @param c The Callable to run
     * @return The result of the callable
     * @throws ExecutionException c's exception
     */
    public static <T> T runOnUiThreadBlocking(Callable<T> c) throws ExecutionException {
        FutureTask<T> task = new FutureTask<T>(c);
        runOnUiThread(task);
        try {
            return task.get();
        } catch (InterruptedException e) {
            throw new RuntimeException("Interrupted waiting for callable", e);
        }
    }

    /**
     * Run the supplied FutureTask on the main thread. The method will block only if the current
     * thread is the main thread.
     *
     * @param task The FutureTask to run
     * @return The queried task (to aid inline construction)
     */
    public static <T> FutureTask<T> runOnUiThread(FutureTask<T> task) {
        if (runningOnUiThread()) {
            task.run();
        } else {
            postOnUiThread(task);
        }
        return task;
    }

    /**
     * Run the supplied Callable on the main thread. The method will block only if the current
     * thread is the main thread.
     *
     * @param c The Callable to run
     * @return A FutureTask wrapping the callable to retrieve results
     */
    public static <T> FutureTask<T> runOnUiThread(Callable<T> c) {
        return runOnUiThread(new FutureTask<T>(c));
    }

    /**
     * Run the supplied Runnable on the main thread. The method will block only if the current
     * thread is the main thread.
     *
     * @param r The Runnable to run
     */
    public static void runOnUiThread(Runnable r) {
        if (runningOnUiThread()) {
            r.run();
        } else {
            getUiThreadHandler().post(r);
        }
    }

    /**
     * Post the supplied FutureTask to run on the main thread. The method will not block, even if
     * called on the UI thread.
     *
     * @param task The FutureTask to run
     * @return The queried task (to aid inline construction)
     */
    public static <T> FutureTask<T> postOnUiThread(FutureTask<T> task) {
        getUiThreadHandler().post(task);
        return task;
    }

    /**
     * Post the supplied Runnable to run on the main thread. The method will not block, even if
     * called on the UI thread.
     *
     * @param task The Runnable to run
     */
    public static void postOnUiThread(Runnable task) {
        getUiThreadHandler().post(task);
    }

    /**
     * Post the supplied Runnable to run on the main thread after the given amount of time. The
     * method will not block, even if called on the UI thread.
     *
     * @param task The Runnable to run
     * @param delayMillis The delay in milliseconds until the Runnable will be run
     */
    @VisibleForTesting
    public static void postOnUiThreadDelayed(Runnable task, long delayMillis) {
        getUiThreadHandler().postDelayed(task, delayMillis);
    }

    /**
     * Throw an exception (when DCHECKs are enabled) if currently not running on the UI thread.
     *
     * Can be disabled by setThreadAssertsDisabledForTesting(true).
     */
    public static void assertOnUiThread() {
        if (sThreadAssertsDisabled) return;

        assert runningOnUiThread() : "Must be called on the UI thread.";
    }

    /**
     * Throw an exception (regardless of build) if currently not running on the UI thread.
     *
     * Can be disabled by setThreadAssertsEnabledForTesting(false).
     *
     * @see #assertOnUiThread()
     */
    public static void checkUiThread() {
        if (!sThreadAssertsDisabled && !runningOnUiThread()) {
            throw new IllegalStateException("Must be called on the UI thread.");
        }
    }

    /**
     * Throw an exception (when DCHECKs are enabled) if currently running on the UI thread.
     *
     * Can be disabled by setThreadAssertsDisabledForTesting(true).
     */
    public static void assertOnBackgroundThread() {
        if (sThreadAssertsDisabled) return;

        assert !runningOnUiThread() : "Must be called on a thread other than UI.";
    }

    /**
     * Disables thread asserts.
     *
     * Can be used by tests where code that normally runs multi-threaded is going to run
     * single-threaded for the test (otherwise asserts that are valid in production would fail in
     * those tests).
     */
    public static void setThreadAssertsDisabledForTesting(boolean disabled) {
        sThreadAssertsDisabled = disabled;
    }

    /**
     * @return true iff the current thread is the main (UI) thread.
     */
    public static boolean runningOnUiThread() {
        return getUiThreadHandler().getLooper() == Looper.myLooper();
    }

    public static Looper getUiThreadLooper() {
        return getUiThreadHandler().getLooper();
    }

    /**
     * Set thread priority to audio.
     */
    @CalledByNative
    public static void setThreadPriorityAudio(int tid) {
        Process.setThreadPriority(tid, Process.THREAD_PRIORITY_AUDIO);
    }

    /**
     * Checks whether Thread priority is THREAD_PRIORITY_AUDIO or not.
     * @param tid Thread id.
     * @return true for THREAD_PRIORITY_AUDIO and false otherwise.
     */
    @CalledByNative
    private static boolean isThreadPriorityAudio(int tid) {
        return Process.getThreadPriority(tid) == Process.THREAD_PRIORITY_AUDIO;
    }
}
