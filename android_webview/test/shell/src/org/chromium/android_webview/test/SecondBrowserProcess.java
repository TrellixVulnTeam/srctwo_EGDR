// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.android_webview.test;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.os.Parcel;
import android.os.Process;

import org.chromium.android_webview.AwBrowserProcess;
import org.chromium.android_webview.AwResource;
import org.chromium.android_webview.shell.R;
import org.chromium.base.CommandLine;
import org.chromium.base.ContextUtils;
import org.chromium.base.annotations.SuppressFBWarnings;

/**
 * This is a service for imitating a second browser process in the application.
 */
public class SecondBrowserProcess extends Service {
    public static final int CODE_START = IBinder.FIRST_CALL_TRANSACTION;

    private IBinder mBinder = new Binder() {
        @Override
        protected boolean onTransact(int code, Parcel data, Parcel reply, int flags) {
            switch (code) {
                case CODE_START:
                    reply.writeNoException();
                    try {
                        startBrowserProcess();
                        reply.writeInt(Process.myPid());
                    } catch (Exception e) {
                        reply.writeInt(0);
                    }
                    return true;
            }
            return false;
        }
    };

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return START_STICKY;
    }

    @SuppressFBWarnings("DMI_HARDCODED_ABSOLUTE_FILENAME")
    private void startBrowserProcess() throws Exception {
        CommandLine.initFromFile("/data/local/tmp/android-webview-command-line");
        AwResource.setResources(this.getResources());
        AwResource.setConfigKeySystemUuidMapping(R.array.config_key_system_uuid_mapping);
        ContextUtils.initApplicationContext(getApplicationContext());
        AwBrowserProcess.loadLibrary();
        AwBrowserProcess.start();
    }
}
