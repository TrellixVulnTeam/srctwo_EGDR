// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.android_webview.test;

import static org.junit.Assert.assertNotEquals;

import android.support.test.filters.SmallTest;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.chromium.android_webview.AwDebug;
import org.chromium.base.annotations.SuppressFBWarnings;
import org.chromium.base.test.util.Feature;
import org.chromium.base.test.util.parameter.SkipCommandLineParameterization;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Scanner;

/**
 * A test suite for AwDebug class.
 */
// Only works in single-process mode, crbug.com/568825.
@RunWith(AwJUnit4ClassRunner.class)
@SkipCommandLineParameterization
public class AwDebugTest {
    @SuppressFBWarnings("URF_UNREAD_PUBLIC_OR_PROTECTED_FIELD")
    @Rule
    public AwActivityTestRule mActivityTestRule = new AwActivityTestRule();

    private static final String TAG = "cr_AwDebugTest";
    private static final String WHITELISTED_DEBUG_KEY = "AW_WHITELISTED_DEBUG_KEY";
    private static final String NON_WHITELISTED_DEBUG_KEY = "AW_NONWHITELISTED_DEBUG_KEY";
    private static final String DEBUG_VALUE = "AW_DEBUG_VALUE";

    @Test
    @SmallTest
    @Feature({"AndroidWebView", "Debug"})
    public void testDump() throws Throwable {
        File f = File.createTempFile("dump", ".dmp");
        try {
            Assert.assertTrue(AwDebug.dumpWithoutCrashing(f));
            Assert.assertTrue(f.canRead());
            assertNotEquals(f.length(), 0);
        } finally {
            Assert.assertTrue(f.delete());
        }
    }

    @Test
    @SmallTest
    @Feature({"AndroidWebView", "Debug"})
    public void testDumpContainsWhitelistedKey() throws Throwable {
        File f = File.createTempFile("dump", ".dmp");
        try {
            AwDebug.initCrashKeysForTesting();
            AwDebug.setCrashKeyValue(WHITELISTED_DEBUG_KEY, DEBUG_VALUE);
            Assert.assertTrue(AwDebug.dumpWithoutCrashing(f));
            assertContainsCrashKeyValue(f, WHITELISTED_DEBUG_KEY, DEBUG_VALUE);
        } finally {
            Assert.assertTrue(f.delete());
        }
    }

    @Test
    @SmallTest
    @Feature({"AndroidWebView", "Debug"})
    public void testDumpDoesNotContainNonWhitelistedKey() throws Throwable {
        File f = File.createTempFile("dump", ".dmp");
        try {
            AwDebug.initCrashKeysForTesting();
            AwDebug.setCrashKeyValue(NON_WHITELISTED_DEBUG_KEY, DEBUG_VALUE);
            Assert.assertTrue(AwDebug.dumpWithoutCrashing(f));
            assertNotContainsCrashKeyValue(f, NON_WHITELISTED_DEBUG_KEY);
        } finally {
            Assert.assertTrue(f.delete());
        }
    }

    private void assertNotContainsCrashKeyValue(File dump, String key) throws IOException {
        String dumpContents = readEntireFile(dump);
        // We are expecting the following to NOT be in the dumpContents:
        //
        // Content-Disposition: form-data; name="AW_DEBUG_KEY"
        // AW_DEBUG_VALUE
        Assert.assertFalse(dumpContents.contains(getDebugKeyLine(key)));
    }

    private void assertContainsCrashKeyValue(File dump, String key, String expectedValue)
            throws IOException {
        String dumpContents = readEntireFile(dump);
        // We are expecting the following lines:
        //
        // Content-Disposition: form-data; name="AW_DEBUG_KEY"
        // AW_DEBUG_VALUE
        String debugKeyLine = getDebugKeyLine(key);
        Assert.assertTrue(dumpContents.contains(debugKeyLine));

        int debugKeyIndex = dumpContents.indexOf(debugKeyLine);
        // Read the word after the line containing the debug key
        Scanner debugValueScanner =
                new Scanner(dumpContents.substring(debugKeyIndex + debugKeyLine.length()));
        Assert.assertTrue(debugValueScanner.hasNext());
        Assert.assertEquals(expectedValue, debugValueScanner.next());
    }

    private static String getDebugKeyLine(String debugKey) {
        return "Content-Disposition: form-data; name=\"" + debugKey + "\"";
    }

    private static String readEntireFile(File file) throws IOException {
        FileInputStream fileInputStream = new FileInputStream(file);
        try {
            byte[] data = new byte[(int) file.length()];
            fileInputStream.read(data);
            return new String(data);
        } finally {
            fileInputStream.close();
        }
    }
}
