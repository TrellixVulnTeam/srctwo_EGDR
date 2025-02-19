// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.ui.display;

import android.graphics.Point;

/**
 * An instance of DisplayAndroid not associated with any physical display.
 */
public class VirtualDisplayAndroid extends DisplayAndroid {
    public static VirtualDisplayAndroid createVirtualDisplay() {
        return getManager().addVirtualDisplay();
    }

    /* package */ VirtualDisplayAndroid(int displayId) {
        super(displayId);
    }

    /**
     * @param other Sets the properties of this display to those of the other display.
     */
    public void setTo(DisplayAndroid other) {
        update(new Point(other.getDisplayWidth(), other.getDisplayHeight()), other.getDipScale(),
                other.getBitsPerPixel(), other.getBitsPerComponent(), other.getRotation(),
                other.mIsDisplayWideColorGamut, other.mIsDisplayServerWideColorGamut);
    }

    @Override
    public void update(Point size, Float dipScale, Integer bitsPerPixel, Integer bitsPerComponent,
            Integer rotation, Boolean isDisplayWideColorGamut,
            Boolean isDisplayServerWideColorGamut) {
        super.update(size, dipScale, bitsPerPixel, bitsPerComponent, rotation,
                isDisplayWideColorGamut, isDisplayServerWideColorGamut);
    }

    /**
     * Removes this Virtual Display from the DisplayManger.
     */
    public void destroy() {
        getManager().removeVirtualDisplay(this);
    }
}
