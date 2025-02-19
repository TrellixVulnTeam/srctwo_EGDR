// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.android_webview.crash;

interface ICrashReceiverService {
    void transmitCrashes(in ParcelFileDescriptor[] fileDescriptors);
}
