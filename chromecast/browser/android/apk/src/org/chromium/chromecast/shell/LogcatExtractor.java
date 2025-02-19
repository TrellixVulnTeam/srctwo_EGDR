// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
package org.chromium.chromecast.shell;

import android.util.Patterns;

import org.chromium.base.Log;
import org.chromium.base.VisibleForTesting;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;
import java.util.regex.Pattern;
/**
 * Extracts logcat out of Android devices and elide PII sensitive info from it.
 *
 * <p>Elided information includes: Emails, IP address, MAC address, URL/domains as well as
 * Javascript console messages.
 *
 * <p>Caller provides a list of minidump files as well as an intent. This Callable will then extract
 * the most recent logcat and save a copy for each minidump.
 *
 * <p>Upon completion, each minidump + logcat pairs will be passed to the MinidumpPreparationService
 * along with the intent provided here.
 */
class LogcatExtractor {
    private static final String TAG = "LogcatExtractor";
    private static final long HALF_SECOND = 500;
    protected static final int LOGCAT_SIZE = 256; // Number of lines.
    protected static final String EMAIL_ELISION = "XXX@EMAIL.ELIDED";
    @VisibleForTesting
    protected static final String URL_ELISION = "HTTP://WEBADDRESS.ELIDED";
    @VisibleForTesting
    protected static final String IP_ELISION = "1.2.3.4";
    @VisibleForTesting
    protected static final String MAC_ELISION = "01:23:45:67:89:AB";

    @VisibleForTesting
    protected static final String CONSOLE_ELISION = "[ELIDED:CONSOLE(0)] ELIDED CONSOLE MESSAGE";

    private static final Pattern MAC_ADDRESS =
            Pattern.compile("([0-9a-fA-F]{2}[-:]+){5}[0-9a-fA-F]{2}");
    private static final Pattern CONSOLE_MSG = Pattern.compile("\\[\\w*:CONSOLE.*\\].*");

    public static String getElidedLogcat() throws IOException, InterruptedException {
        List<String> rawLogcat = getLogcat();
        List<String> elidedLogcat = Collections.unmodifiableList(processLogcat(rawLogcat));
        return joinLines(elidedLogcat);
    }

    private static List<String> getLogcat() throws IOException, InterruptedException {
        LinkedList<String> rawLogcat = new LinkedList<String>();
        String logLn = null;
        Integer exitValue = null;

        Process p = Runtime.getRuntime().exec("logcat -d");
        BufferedReader bReader = null;
        try {
            bReader = new BufferedReader(new InputStreamReader(p.getInputStream()));
            while (exitValue == null) {
                while ((logLn = bReader.readLine()) != null) {
                    // Add each new string to the end of the queue.
                    rawLogcat.add(logLn);
                    if (rawLogcat.size() > LOGCAT_SIZE) {
                        // Remove the head of the queue when it gets too large.
                        rawLogcat.remove();
                    }
                }
                try {
                    exitValue = p.exitValue();
                } catch (IllegalThreadStateException itse) {
                    Thread.sleep(HALF_SECOND);
                }
            }
            if (exitValue != 0) {
                String msg = "Logcat failed: " + exitValue;
                Log.w(TAG, msg);
                throw new IOException(msg);
            }
            return rawLogcat;
        } finally {
            if (bReader != null) {
                bReader.close();
            }
        }
    }

    private static String joinLines(List<String> elidedLogcat) {
        StringBuilder sBuilder = new StringBuilder();
        for (String ln : elidedLogcat) {
            sBuilder.append(ln);
            sBuilder.append("\n");
        }
        return sBuilder.toString();
    }

    @VisibleForTesting
    protected static List<String> processLogcat(List<String> rawLogcat) {
        List<String> out = new ArrayList<String>(rawLogcat.size());
        for (String ln : rawLogcat) {
            ln = elideEmail(ln);
            ln = elideUrl(ln);
            ln = elideIp(ln);
            ln = elideMac(ln);
            ln = elideConsole(ln);
            out.add(ln);
        }
        return out;
    }
    /**
     * Elides any emails in the specified {@link String} with {@link #EMAIL_ELISION}.
     *
     * @param original String potentially containing emails.
     * @return String with elided emails.
     */
    @VisibleForTesting
    protected static String elideEmail(String original) {
        return Patterns.EMAIL_ADDRESS.matcher(original).replaceAll(EMAIL_ELISION);
    }
    /**
     * Elides any URLs in the specified {@link String} with {@link #URL_ELISION}.
     *
     * @param original String potentially containing URLs.
     * @return String with elided URLs.
     */
    @VisibleForTesting
    protected static String elideUrl(String original) {
        return Patterns.WEB_URL.matcher(original).replaceAll(URL_ELISION);
    }
    /**
     * Elides any IP addresses in the specified {@link String} with {@link #IP_ELISION}.
     *
     * @param original String potentially containing IPs.
     * @return String with elided IPs.
     */
    @VisibleForTesting
    protected static String elideIp(String original) {
        return Patterns.IP_ADDRESS.matcher(original).replaceAll(IP_ELISION);
    }
    /**
     * Elides any MAC addresses in the specified {@link String} with {@link #MAC_ELISION}.
     *
     * @param original String potentially containing MACs.
     * @return String with elided MACs.
     */
    @VisibleForTesting
    protected static String elideMac(String original) {
        return MAC_ADDRESS.matcher(original).replaceAll(MAC_ELISION);
    }
    /**
     * Elides any console messages in the specified {@link String} with {@link #CONSOLE_ELISION}.
     *
     * @param original String potentially containing console messages.
     * @return String with elided console messages.
     */
    @VisibleForTesting
    protected static String elideConsole(String original) {
        return CONSOLE_MSG.matcher(original).replaceAll(CONSOLE_ELISION);
    }
}
