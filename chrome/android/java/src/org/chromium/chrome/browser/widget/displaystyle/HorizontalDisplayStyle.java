// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.widget.displaystyle;

import android.support.annotation.IntDef;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/**
 * The horizontal dimension groups.
 */
@Retention(RetentionPolicy.SOURCE)
@IntDef({HorizontalDisplayStyle.NARROW, HorizontalDisplayStyle.REGULAR,
        HorizontalDisplayStyle.WIDE})
public @interface HorizontalDisplayStyle {
    int NARROW = 0;
    int REGULAR = 1;
    int WIDE = 2;
}
