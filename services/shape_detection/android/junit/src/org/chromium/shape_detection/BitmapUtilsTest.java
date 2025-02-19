// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.shape_detection;

import static org.junit.Assert.assertNull;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.MockitoAnnotations;
import org.robolectric.annotation.Config;
import org.robolectric.shadows.ShadowLog;

import org.chromium.base.test.util.Feature;
import org.chromium.skia.mojom.Bitmap;
import org.chromium.skia.mojom.ColorType;
import org.chromium.testing.local.LocalRobolectricTestRunner;

/**
 * Test suite for conversion-to-Frame utils.
 */
@RunWith(LocalRobolectricTestRunner.class)
@Config(manifest = Config.NONE)
public class BitmapUtilsTest {
    private static final int VALID_WIDTH = 1;
    private static final int VALID_HEIGHT = 1;
    private static final int INVALID_WIDTH = 0;
    private static final long NUM_BYTES = VALID_WIDTH * VALID_HEIGHT * 4;
    private static final byte[] EMPTY_DATA = new byte[0];

    public BitmapUtilsTest() {}

    @Before
    public void setUp() {
        ShadowLog.stream = System.out;
        MockitoAnnotations.initMocks(this);
    }

    /**
     * Verify conversion fails if the Bitmap is invalid.
     */
    @Test
    @Feature({"ShapeDetection"})
    public void testConversionFailsWithInvalidBitmap() {
        Bitmap bitmap = new Bitmap();
        bitmap.pixelData = null;

        assertNull(BitmapUtils.convertToFrame(bitmap));
    }

    /**
     * Verify conversion fails if the sent dimensions are ugly.
     */
    @Test
    @Feature({"ShapeDetection"})
    public void testConversionFailsWithInvalidDimensions() {
        Bitmap bitmap = new Bitmap();
        bitmap.pixelData = EMPTY_DATA;
        bitmap.width = INVALID_WIDTH;
        bitmap.height = VALID_HEIGHT;

        assertNull(BitmapUtils.convertToFrame(bitmap));
    }

    /**
     * Verify conversion fails if Bitmap fails to wrap().
     */
    @Test
    @Feature({"ShapeDetection"})
    public void testConversionFailsWithWronglyWrappedData() {
        Bitmap bitmap = new Bitmap();
        bitmap.pixelData = EMPTY_DATA;
        bitmap.width = VALID_WIDTH;
        bitmap.height = VALID_HEIGHT;
        bitmap.colorType = ColorType.RGBA_8888;

        assertNull(BitmapUtils.convertToFrame(bitmap));
    }
}
