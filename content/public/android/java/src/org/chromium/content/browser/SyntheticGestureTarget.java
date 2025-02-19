// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.content.browser;

import android.view.View;

import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;

/**
 * Fowards synthetic events to MotionEventSynthesizer. Owned by its native.
 */
@JNINamespace("content")
public class SyntheticGestureTarget {
    private final MotionEventSynthesizer mMotionEventSynthesizer;

    @CalledByNative
    private static SyntheticGestureTarget create(View target) {
        return new SyntheticGestureTarget(target);
    }

    private SyntheticGestureTarget(View target) {
        mMotionEventSynthesizer = new MotionEventSynthesizer(target);
    }

    @CalledByNative
    private void inject(int action, int pointerCount, long timeInMs) {
        mMotionEventSynthesizer.inject(action, pointerCount, timeInMs);
    }

    @CalledByNative
    private void setPointer(int index, int x, int y, int id) {
        mMotionEventSynthesizer.setPointer(index, x, y, id);
    }

    @CalledByNative
    private void setScrollDeltas(int x, int y, int dx, int dy) {
        mMotionEventSynthesizer.setScrollDeltas(x, y, dx, dy);
    }
}
