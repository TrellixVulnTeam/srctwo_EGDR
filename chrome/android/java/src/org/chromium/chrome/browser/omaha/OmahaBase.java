// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.omaha;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Build;
import android.support.annotation.IntDef;

import org.chromium.base.Log;
import org.chromium.base.StreamUtil;
import org.chromium.base.VisibleForTesting;
import org.chromium.chrome.browser.ChromeVersionInfo;

import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.Date;
import java.util.concurrent.TimeUnit;

/**
 * Keeps tabs on the current state of Chrome, tracking if and when a request should be sent to the
 * Omaha Server.
 *
 * When Chrome is brought to the foreground, it will trigger a call to
 * {@link OmahaBase#onForegroundSessionStart}, which kicks off a series of scheduled events
 * that allow the class to run.  A single alarm is used to trigger the whole pipeline when needed.
 * - If Chrome isn't running when the alarm is fired, no pings or update checks will be performed.
 * - If Chrome doesn't have a pending request to POST, no POST will be performed.
 *
 * When a fresh install is detected (or the user clears their data), OmahaBase will send an XML
 * request saying that a new install was detected, then follow up with an XML request saying that
 * the user was active and that we need to check for Chrome updates.
 *
 * mevissen suggested being conservative with our timers for sending requests.
 * POST attempts that fail to be acknowledged by the server are re-attempted, with at least
 * one hour between each attempt.
 *
 * Status is saved directly to the the disk after every run of the pipeline.
 *
 * Implementation notes:
 * http://docs.google.com/a/google.com/document/d/1scTCovqASf5ktkOeVj8wFRkWTCeDYw2LrOBNn05CDB0/edit
 */
public class OmahaBase {
    private static final String TAG = "omaha";

    // Flags for retrieving the OmahaClient's state after it's written to disk.
    // The PREF_PACKAGE doesn't match the current OmahaClient package for historical reasons.
    static final String PREF_PACKAGE = "com.google.android.apps.chrome.omaha";
    static final String PREF_INSTALL_SOURCE = "installSource";
    static final String PREF_LATEST_VERSION = "latestVersion";
    static final String PREF_MARKET_URL = "marketURL";
    static final String PREF_PERSISTED_REQUEST_ID = "persistedRequestID";
    static final String PREF_SEND_INSTALL_EVENT = "sendInstallEvent";
    static final String PREF_TIMESTAMP_FOR_NEW_REQUEST = "timestampForNewRequest";
    static final String PREF_TIMESTAMP_FOR_NEXT_POST_ATTEMPT = "timestampForNextPostAttempt";
    static final String PREF_TIMESTAMP_OF_INSTALL = "timestampOfInstall";
    static final String PREF_TIMESTAMP_OF_REQUEST = "timestampOfRequest";

    static final int MIN_API_JOB_SCHEDULER = Build.VERSION_CODES.M;

    /** Whether or not the Omaha server should really be contacted. */
    private static boolean sIsDisabled;

    // Results of {@link #handlePostRequest()}.
    @Retention(RetentionPolicy.SOURCE)
    @IntDef({POST_RESULT_NO_REQUEST, POST_RESULT_SENT, POST_RESULT_FAILED, POST_RESULT_SCHEDULED})
    @interface PostResult {}
    static final int POST_RESULT_NO_REQUEST = 0;
    static final int POST_RESULT_SENT = 1;
    static final int POST_RESULT_FAILED = 2;
    static final int POST_RESULT_SCHEDULED = 3;

    /** Deprecated; kept around to cancel alarms set for OmahaClient pre-M58. */
    private static final String ACTION_REGISTER_REQUEST =
            "org.chromium.chrome.browser.omaha.ACTION_REGISTER_REQUEST";

    // Delays between events.
    static final long MS_POST_BASE_DELAY = TimeUnit.HOURS.toMillis(1);
    static final long MS_POST_MAX_DELAY = TimeUnit.HOURS.toMillis(5);
    static final long MS_BETWEEN_REQUESTS = TimeUnit.HOURS.toMillis(5);
    static final int MS_CONNECTION_TIMEOUT = (int) TimeUnit.MINUTES.toMillis(1);

    // Strings indicating how the Chrome APK arrived on the user's device. These values MUST NOT
    // be changed without updating the corresponding Omaha server strings.
    private static final String INSTALL_SOURCE_SYSTEM = "system_image";
    private static final String INSTALL_SOURCE_ORGANIC = "organic";

    private static final long INVALID_TIMESTAMP = -1;
    private static final String INVALID_REQUEST_ID = "invalid";

    // Member fields not persisted to disk.
    private final OmahaDelegate mDelegate;
    private boolean mStateHasBeenRestored;

    // State saved written to and read from disk.
    private RequestData mCurrentRequest;
    private long mTimestampOfInstall;
    private long mTimestampForNextPostAttempt;
    private long mTimestampForNewRequest;
    private String mLatestVersion;
    private String mMarketURL;
    private String mInstallSource;
    protected boolean mSendInstallEvent;

    /** See {@link #sIsDisabled}. */
    public static void setIsDisabledForTesting(boolean state) {
        sIsDisabled = state;
    }

    /** See {@link #sIsDisabled}. */
    static boolean isDisabled() {
        return sIsDisabled;
    }

    /**
     * Constructs a new OmahaBase.
     * @param delegate The {@link OmahaDelegate} used to interact with the system.
     */
    OmahaBase(OmahaDelegate delegate) {
        mDelegate = delegate;
    }

    protected void run() {
        if (OmahaBase.isDisabled() || getRequestGenerator() == null) {
            Log.v(TAG, "Disabled.  Ignoring intent.");
            return;
        }

        restoreState(getContext());

        long nextTimestamp = Long.MAX_VALUE;
        if (mDelegate.isChromeBeingUsed()) {
            handleRegisterActiveRequest();
            nextTimestamp = Math.min(nextTimestamp, mTimestampForNewRequest);
        }

        if (hasRequest()) {
            int result = handlePostRequest();
            if (result == POST_RESULT_FAILED || result == POST_RESULT_SCHEDULED) {
                nextTimestamp = Math.min(nextTimestamp, mTimestampForNextPostAttempt);
            }
        }

        // TODO(dfalcantara): Prevent Omaha code from repeatedly rescheduling itself immediately in
        //                    case a scheduling error occurs.
        if (nextTimestamp != Long.MAX_VALUE && nextTimestamp >= 0) {
            long currentTimestamp = mDelegate.getScheduler().getCurrentTime();
            Log.i(TAG, "Attempting to schedule next job for: " + new Date(nextTimestamp));
            mDelegate.scheduleService(currentTimestamp, nextTimestamp);
        }

        saveState(getContext());
    }

    /**
     * Determines if a new request should be generated.  New requests are only generated if enough
     * time has passed between now and the last time a request was generated.
     */
    private void handleRegisterActiveRequest() {
        // If the current request is too old, generate a new one.
        long currentTimestamp = getBackoffScheduler().getCurrentTime();
        boolean isTooOld = hasRequest()
                && mCurrentRequest.getAgeInMilliseconds(currentTimestamp) >= MS_BETWEEN_REQUESTS;
        boolean isOverdue = currentTimestamp >= mTimestampForNewRequest;
        if (isTooOld || isOverdue) {
            registerNewRequest(currentTimestamp);
        }
    }

    /**
     * Sends the request it is holding.
     */
    private int handlePostRequest() {
        if (!hasRequest()) {
            mDelegate.onHandlePostRequestDone(POST_RESULT_NO_REQUEST, false);
            return POST_RESULT_NO_REQUEST;
        }

        // If enough time has passed since the last attempt, try sending a request.
        int result;
        long currentTimestamp = getBackoffScheduler().getCurrentTime();
        boolean installEventWasSent = false;
        if (currentTimestamp >= mTimestampForNextPostAttempt) {
            // All requests made during the same session should have the same ID.
            String sessionID = mDelegate.generateUUID();
            boolean sendingInstallRequest = mSendInstallEvent;
            boolean succeeded = generateAndPostRequest(currentTimestamp, sessionID);

            if (succeeded && sendingInstallRequest) {
                // Only the first request ever generated should contain an install event.
                mSendInstallEvent = false;
                installEventWasSent = true;

                // Create and immediately send another request for a ping and update check.
                registerNewRequest(currentTimestamp);
                succeeded &= generateAndPostRequest(currentTimestamp, sessionID);
            }

            result = succeeded ? POST_RESULT_SENT : POST_RESULT_FAILED;
        } else {
            result = POST_RESULT_SCHEDULED;
        }

        mDelegate.onHandlePostRequestDone(result, installEventWasSent);
        return result;
    }

    private boolean generateAndPostRequest(long currentTimestamp, String sessionID) {
        ExponentialBackoffScheduler scheduler = getBackoffScheduler();
        boolean succeeded = false;
        try {
            // Generate the XML for the current request.
            long installAgeInDays = RequestGenerator.installAge(
                    currentTimestamp, mTimestampOfInstall, mCurrentRequest.isSendInstallEvent());
            String version =
                    VersionNumberGetter.getInstance().getCurrentlyUsedVersion(getContext());
            String xml = getRequestGenerator().generateXML(
                    sessionID, version, installAgeInDays, mCurrentRequest);

            // Send the request to the server & wait for a response.
            String response = postRequest(currentTimestamp, xml);

            // Parse out the response.
            String appId = getRequestGenerator().getAppId();
            boolean sentPingAndUpdate = !mSendInstallEvent;
            ResponseParser parser = new ResponseParser(
                    appId, mSendInstallEvent, sentPingAndUpdate, sentPingAndUpdate);
            parser.parseResponse(response);
            mLatestVersion = parser.getNewVersion();
            mMarketURL = parser.getURL();

            succeeded = true;
        } catch (RequestFailureException e) {
            Log.e(TAG, "Failed to contact server: ", e);
        }

        if (succeeded) {
            // If we've gotten this far, we've successfully sent a request.
            mCurrentRequest = null;

            scheduler.resetFailedAttempts();
            mTimestampForNewRequest = scheduler.getCurrentTime() + MS_BETWEEN_REQUESTS;
            mTimestampForNextPostAttempt = scheduler.calculateNextTimestamp();
            Log.i(TAG,
                    "Request to Server Successful. Timestamp for next request:"
                            + mTimestampForNextPostAttempt);
        } else {
            // Set the alarm to try again later.  Failures are incremented after setting the timer
            // to allow the first failure to incur the minimum base delay between POSTs.
            mTimestampForNextPostAttempt = scheduler.calculateNextTimestamp();
            scheduler.increaseFailedAttempts();
        }

        mDelegate.onGenerateAndPostRequestDone(succeeded);
        return succeeded;
    }

    /**
     * Registers a new request with the current timestamp.  Internal timestamps are reset to start
     * fresh.
     * @param currentTimestamp Current time.
     */
    private void registerNewRequest(long currentTimestamp) {
        mCurrentRequest = createRequestData(currentTimestamp, null);
        getBackoffScheduler().resetFailedAttempts();
        mTimestampForNextPostAttempt = currentTimestamp;

        // Tentatively set the timestamp for a new request.  This will be updated when the server
        // is successfully contacted.
        mTimestampForNewRequest = currentTimestamp + MS_BETWEEN_REQUESTS;

        mDelegate.onRegisterNewRequestDone(mTimestampForNewRequest, mTimestampForNextPostAttempt);
    }

    private RequestData createRequestData(long currentTimestamp, String persistedID) {
        // If we're sending a persisted event, keep trying to send the same request ID.
        String requestID;
        if (persistedID == null || INVALID_REQUEST_ID.equals(persistedID)) {
            requestID = mDelegate.generateUUID();
        } else {
            requestID = persistedID;
        }
        return new RequestData(mSendInstallEvent, currentTimestamp, requestID, mInstallSource);
    }

    private boolean hasRequest() {
        return mCurrentRequest != null;
    }

    /**
     * Posts the request to the Omaha server.
     * @return the XML response as a String.
     * @throws RequestFailureException if the request fails.
     */
    private String postRequest(long timestamp, String xml) throws RequestFailureException {
        String response = null;

        HttpURLConnection urlConnection = null;
        try {
            urlConnection = createConnection();

            // Prepare the HTTP header.
            urlConnection.setDoOutput(true);
            urlConnection.setFixedLengthStreamingMode(xml.getBytes().length);
            if (mSendInstallEvent && getBackoffScheduler().getNumFailedAttempts() > 0) {
                String age = Long.toString(mCurrentRequest.getAgeInSeconds(timestamp));
                urlConnection.addRequestProperty("X-RequestAge", age);
            }

            response = OmahaBase.sendRequestToServer(urlConnection, xml);
        } catch (IllegalAccessError e) {
            throw new RequestFailureException("Caught an IllegalAccessError:", e);
        } catch (IllegalArgumentException e) {
            throw new RequestFailureException("Caught an IllegalArgumentException:", e);
        } catch (IllegalStateException e) {
            throw new RequestFailureException("Caught an IllegalStateException:", e);
        } finally {
            if (urlConnection != null) {
                urlConnection.disconnect();
            }
        }

        return response;
    }

    /**
     * Returns a HttpURLConnection to the server.
     */
    @VisibleForTesting
    protected HttpURLConnection createConnection() throws RequestFailureException {
        try {
            URL url = new URL(getRequestGenerator().getServerUrl());
            HttpURLConnection connection = (HttpURLConnection) url.openConnection();
            connection.setConnectTimeout(MS_CONNECTION_TIMEOUT);
            connection.setReadTimeout(MS_CONNECTION_TIMEOUT);
            return connection;
        } catch (MalformedURLException e) {
            throw new RequestFailureException("Caught a malformed URL exception.", e);
        } catch (IOException e) {
            throw new RequestFailureException("Failed to open connection to URL", e);
        }
    }

    /**
     * Reads the data back from the file it was saved to.  Uses SharedPreferences to handle I/O.
     * Sanity checks are performed on the timestamps to guard against clock changing.
     */
    private void restoreState(Context context) {
        if (mStateHasBeenRestored) return;

        String installSource =
                mDelegate.isInSystemImage() ? INSTALL_SOURCE_SYSTEM : INSTALL_SOURCE_ORGANIC;
        ExponentialBackoffScheduler scheduler = getBackoffScheduler();
        long currentTime = scheduler.getCurrentTime();

        SharedPreferences preferences = OmahaBase.getSharedPreferences(context);
        mTimestampForNewRequest =
                preferences.getLong(OmahaBase.PREF_TIMESTAMP_FOR_NEW_REQUEST, currentTime);
        mTimestampForNextPostAttempt =
                preferences.getLong(OmahaBase.PREF_TIMESTAMP_FOR_NEXT_POST_ATTEMPT, currentTime);
        mTimestampOfInstall = preferences.getLong(OmahaBase.PREF_TIMESTAMP_OF_INSTALL, currentTime);
        mSendInstallEvent = preferences.getBoolean(OmahaBase.PREF_SEND_INSTALL_EVENT, true);
        mInstallSource = preferences.getString(OmahaBase.PREF_INSTALL_SOURCE, installSource);
        mLatestVersion = preferences.getString(OmahaBase.PREF_LATEST_VERSION, "");
        mMarketURL = preferences.getString(OmahaBase.PREF_MARKET_URL, "");

        // If we're not sending an install event, don't bother restoring the request ID:
        // the server does not expect to have persisted request IDs for pings or update checks.
        String persistedRequestId = mSendInstallEvent
                ? preferences.getString(OmahaBase.PREF_PERSISTED_REQUEST_ID, INVALID_REQUEST_ID)
                : INVALID_REQUEST_ID;
        long requestTimestamp =
                preferences.getLong(OmahaBase.PREF_TIMESTAMP_OF_REQUEST, INVALID_TIMESTAMP);
        mCurrentRequest = requestTimestamp == INVALID_TIMESTAMP
                ? null
                : createRequestData(requestTimestamp, persistedRequestId);

        // Confirm that the timestamp for the next request is less than the base delay.
        long delayToNewRequest = mTimestampForNewRequest - currentTime;
        if (delayToNewRequest > MS_BETWEEN_REQUESTS) {
            Log.w(TAG,
                    "Delay to next request (" + delayToNewRequest
                            + ") is longer than expected.  Resetting to now.");
            mTimestampForNewRequest = currentTime;
        }

        // Confirm that the timestamp for the next POST is less than the current delay.
        long delayToNextPost = mTimestampForNextPostAttempt - currentTime;
        long lastGeneratedDelay = scheduler.getGeneratedDelay();
        if (delayToNextPost > lastGeneratedDelay) {
            Log.w(TAG,
                    "Delay to next post attempt (" + delayToNextPost
                            + ") is greater than expected (" + lastGeneratedDelay
                            + ").  Resetting to now.");
            mTimestampForNextPostAttempt = currentTime;
        }

        migrateToNewerChromeVersions();
        mStateHasBeenRestored = true;
    }

    /**
     * Writes out the current state to a file.
     */
    private void saveState(Context context) {
        SharedPreferences prefs = OmahaBase.getSharedPreferences(context);
        SharedPreferences.Editor editor = prefs.edit();
        editor.putBoolean(OmahaBase.PREF_SEND_INSTALL_EVENT, mSendInstallEvent);
        editor.putLong(OmahaBase.PREF_TIMESTAMP_OF_INSTALL, mTimestampOfInstall);
        editor.putLong(
                OmahaBase.PREF_TIMESTAMP_FOR_NEXT_POST_ATTEMPT, mTimestampForNextPostAttempt);
        editor.putLong(OmahaBase.PREF_TIMESTAMP_FOR_NEW_REQUEST, mTimestampForNewRequest);
        editor.putLong(OmahaBase.PREF_TIMESTAMP_OF_REQUEST,
                hasRequest() ? mCurrentRequest.getCreationTimestamp() : INVALID_TIMESTAMP);
        editor.putString(OmahaBase.PREF_PERSISTED_REQUEST_ID,
                hasRequest() ? mCurrentRequest.getRequestID() : INVALID_REQUEST_ID);
        editor.putString(
                OmahaBase.PREF_LATEST_VERSION, mLatestVersion == null ? "" : mLatestVersion);
        editor.putString(OmahaBase.PREF_MARKET_URL, mMarketURL == null ? "" : mMarketURL);
        editor.putString(OmahaBase.PREF_INSTALL_SOURCE, mInstallSource);
        editor.apply();

        mDelegate.onSaveStateDone(mTimestampForNewRequest, mTimestampForNextPostAttempt);
    }

    private void migrateToNewerChromeVersions() {
        // Remove any repeating alarms in favor of the new scheduling setup on M58 and up.
        // Seems cheaper to cancel the alarm repeatedly than to store a SharedPreference and never
        // do it again.
        Intent intent = new Intent(getContext(), OmahaClient.class);
        intent.setAction(ACTION_REGISTER_REQUEST);
        getBackoffScheduler().cancelAlarm(intent);
    }

    private Context getContext() {
        return mDelegate.getContext();
    }

    private RequestGenerator getRequestGenerator() {
        return mDelegate.getRequestGenerator();
    }

    private ExponentialBackoffScheduler getBackoffScheduler() {
        return mDelegate.getScheduler();
    }

    /** Begin communicating with the Omaha Update Server. */
    public static void onForegroundSessionStart(Context context) {
        if (!ChromeVersionInfo.isOfficialBuild() || isDisabled()) return;
        OmahaService.startServiceImmediately(context);
    }

    /** Checks whether Chrome has ever tried contacting Omaha before. */
    public static boolean isProbablyFreshInstall(Context context) {
        SharedPreferences prefs = getSharedPreferences(context);
        return prefs.getLong(PREF_TIMESTAMP_OF_INSTALL, -1) == -1;
    }

    /** Sends the request to the server and returns the response. */
    static String sendRequestToServer(HttpURLConnection urlConnection, String request)
            throws RequestFailureException {
        try {
            OutputStream out = new BufferedOutputStream(urlConnection.getOutputStream());
            OutputStreamWriter writer = new OutputStreamWriter(out);
            writer.write(request, 0, request.length());
            StreamUtil.closeQuietly(writer);
            checkServerResponseCode(urlConnection);
        } catch (IOException | SecurityException e) {
            throw new RequestFailureException("Failed to write request to server: ", e);
        }

        try {
            InputStreamReader reader = new InputStreamReader(urlConnection.getInputStream());
            BufferedReader in = new BufferedReader(reader);
            try {
                StringBuilder response = new StringBuilder();
                for (String line = in.readLine(); line != null; line = in.readLine()) {
                    response.append(line);
                }
                checkServerResponseCode(urlConnection);
                return response.toString();
            } finally {
                StreamUtil.closeQuietly(in);
            }
        } catch (IOException e) {
            throw new RequestFailureException("Failed when reading response from server: ", e);
        }
    }

    /** Confirms that the Omaha server sent back an "OK" code. */
    private static void checkServerResponseCode(HttpURLConnection urlConnection)
            throws RequestFailureException {
        try {
            if (urlConnection.getResponseCode() != 200) {
                throw new RequestFailureException("Received " + urlConnection.getResponseCode()
                        + " code instead of 200 (OK) from the server.  Aborting.");
            }
        } catch (IOException e) {
            throw new RequestFailureException("Failed to read response code from server: ", e);
        }
    }

    /** Returns the Omaha SharedPreferences. */
    static SharedPreferences getSharedPreferences(Context context) {
        return context.getSharedPreferences(PREF_PACKAGE, Context.MODE_PRIVATE);
    }
}
