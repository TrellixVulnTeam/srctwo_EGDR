// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.android_webview.test;

import android.support.test.InstrumentationRegistry;
import android.support.test.filters.SmallTest;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.chromium.android_webview.AwFormDatabase;
import org.chromium.base.annotations.SuppressFBWarnings;

/** AwFormDatabaseTest. */
@RunWith(AwJUnit4ClassRunner.class)
public class AwFormDatabaseTest {
    @SuppressFBWarnings("URF_UNREAD_PUBLIC_OR_PROTECTED_FIELD")
    @Rule
    public AwActivityTestRule mActivityTestRule = new AwActivityTestRule();

    @Test
    @SmallTest
    public void testSmoke() throws Throwable {
        InstrumentationRegistry.getInstrumentation().runOnMainSync(() -> {
            AwFormDatabase.clearFormData();
            Assert.assertFalse(AwFormDatabase.hasFormData());
        });
    }
}
