// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.android_webview.unittest;

import org.chromium.base.annotations.CalledByNative;

import java.io.IOException;
import java.io.InputStream;

class InputStreamUnittest {
    private InputStreamUnittest() {
    }

    @CalledByNative
    static InputStream getEmptyStream() {
        return new InputStream() {
            @Override
            public int read() {
                return -1;
            }
        };
    }

    @CalledByNative
    static InputStream getThrowingStream() {
        return new InputStream() {
            @Override
            public int available() throws IOException {
                throw new IOException();
            }

            @Override
            public void close() throws IOException {
                throw new IOException();
            }

            @Override
            public long skip(long n) throws IOException {
                throw new IOException();
            }

            @Override
            public int read() throws IOException {
                throw new IOException();
            }
        };
    }

    @CalledByNative
    static InputStream getCountingStream(final int size) {
        return new InputStream() {
            private int mCount = 0;

            @Override
            public int read() {
                if (mCount < size) {
                    return mCount++ % 256;
                } else {
                    return -1;
                }
            }
        };
    }
}
