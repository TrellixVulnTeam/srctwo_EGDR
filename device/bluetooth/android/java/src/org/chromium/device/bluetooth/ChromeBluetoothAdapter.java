// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.device.bluetooth;

import android.annotation.TargetApi;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.le.ScanSettings;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Build;
import android.os.ParcelUuid;

import org.chromium.base.Log;
import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;
import org.chromium.components.location.LocationUtils;

import java.util.List;

/**
 * Exposes android.bluetooth.BluetoothAdapter as necessary for C++
 * device::BluetoothAdapterAndroid, which implements the cross platform
 * device::BluetoothAdapter.
 *
 * Lifetime is controlled by device::BluetoothAdapterAndroid.
 */
@JNINamespace("device")
@TargetApi(Build.VERSION_CODES.M)
final class ChromeBluetoothAdapter extends BroadcastReceiver {
    private static final String TAG = "Bluetooth";

    private long mNativeBluetoothAdapterAndroid;
    // mAdapter is final to ensure registerReceiver is followed by unregisterReceiver.
    private final Wrappers.BluetoothAdapterWrapper mAdapter;
    private ScanCallback mScanCallback;

    // ---------------------------------------------------------------------------------------------
    // Construction and handler for C++ object destruction.

    /**
     * Constructs a ChromeBluetoothAdapter.
     * @param nativeBluetoothAdapterAndroid Is the associated C++
     *                                      BluetoothAdapterAndroid pointer value.
     * @param adapterWrapper Wraps the default android.bluetooth.BluetoothAdapter,
     *                       but may be either null if an adapter is not available
     *                       or a fake for testing.
     */
    public ChromeBluetoothAdapter(
            long nativeBluetoothAdapterAndroid, Wrappers.BluetoothAdapterWrapper adapterWrapper) {
        mNativeBluetoothAdapterAndroid = nativeBluetoothAdapterAndroid;
        mAdapter = adapterWrapper;
        registerBroadcastReceiver();
        if (adapterWrapper == null) {
            Log.i(TAG, "ChromeBluetoothAdapter created with no adapterWrapper.");
        } else {
            Log.i(TAG, "ChromeBluetoothAdapter created with provided adapterWrapper.");
        }
    }

    /**
     * Handles C++ object being destroyed.
     */
    @CalledByNative
    private void onBluetoothAdapterAndroidDestruction() {
        stopScan();
        mNativeBluetoothAdapterAndroid = 0;
        unregisterBroadcastReceiver();
    }

    // ---------------------------------------------------------------------------------------------
    // BluetoothAdapterAndroid methods implemented in java:

    // Implements BluetoothAdapterAndroid::Create.
    // 'Object' type must be used for |adapterWrapper| because inner class
    // Wrappers.BluetoothAdapterWrapper reference is not handled by jni_generator.py JavaToJni.
    // http://crbug.com/505554
    @CalledByNative
    private static ChromeBluetoothAdapter create(
            long nativeBluetoothAdapterAndroid, Object adapterWrapper) {
        return new ChromeBluetoothAdapter(
                nativeBluetoothAdapterAndroid, (Wrappers.BluetoothAdapterWrapper) adapterWrapper);
    }

    // Implements BluetoothAdapterAndroid::GetAddress.
    @CalledByNative
    private String getAddress() {
        if (isPresent()) {
            return mAdapter.getAddress();
        } else {
            return "";
        }
    }

    // Implements BluetoothAdapterAndroid::GetName.
    @CalledByNative
    private String getName() {
        if (isPresent()) {
            return mAdapter.getName();
        } else {
            return "";
        }
    }

    // Implements BluetoothAdapterAndroid::IsPresent.
    @CalledByNative
    private boolean isPresent() {
        return mAdapter != null;
    }

    // Implements BluetoothAdapterAndroid::IsPowered.
    @CalledByNative
    private boolean isPowered() {
        return isPresent() && mAdapter.isEnabled();
    }

    // Implements BluetoothAdapterAndroid::SetPowered.
    @CalledByNative
    private boolean setPowered(boolean powered) {
        if (powered) {
            return isPresent() && mAdapter.enable();
        } else {
            return isPresent() && mAdapter.disable();
        }
    }

    // Implements BluetoothAdapterAndroid::IsDiscoverable.
    @CalledByNative
    private boolean isDiscoverable() {
        return isPresent()
                && mAdapter.getScanMode() == BluetoothAdapter.SCAN_MODE_CONNECTABLE_DISCOVERABLE;
    }

    // Implements BluetoothAdapterAndroid::IsDiscovering.
    @CalledByNative
    private boolean isDiscovering() {
        return isPresent() && (mAdapter.isDiscovering() || mScanCallback != null);
    }

    /**
     * Starts a Low Energy scan.
     * @return True on success.
     */
    @CalledByNative
    private boolean startScan() {
        Wrappers.BluetoothLeScannerWrapper scanner = mAdapter.getBluetoothLeScanner();

        if (scanner == null) {
            return false;
        }

        if (!canScan()) {
            return false;
        }

        // scanMode note: SCAN_FAILED_FEATURE_UNSUPPORTED is caused (at least on some devices) if
        // setReportDelay() is used or if SCAN_MODE_LOW_LATENCY isn't used.
        int scanMode = ScanSettings.SCAN_MODE_LOW_LATENCY;

        assert mScanCallback == null;
        mScanCallback = new ScanCallback();

        try {
            scanner.startScan(null /* filters */, scanMode, mScanCallback);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "Cannot start scan: " + e);
            mScanCallback = null;
            return false;
        } catch (IllegalStateException e) {
            Log.e(TAG, "Adapter is off. Cannot start scan: " + e);
            mScanCallback = null;
            return false;
        }
        return true;
    }

    /**
     * Stops the Low Energy scan.
     * @return True if a scan was in progress.
     */
    @CalledByNative
    private boolean stopScan() {
        if (mScanCallback == null) {
            return false;
        }

        try {
            Wrappers.BluetoothLeScannerWrapper scanner = mAdapter.getBluetoothLeScanner();
            if (scanner != null) {
                scanner.stopScan(mScanCallback);
            }
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "Cannot stop scan: " + e);
        } catch (IllegalStateException e) {
            Log.e(TAG, "Adapter is off. Cannot stop scan: " + e);
        }
        mScanCallback = null;
        return true;
    }

    // ---------------------------------------------------------------------------------------------
    // Implementation details:

    /**
     * @return true if Chromium has permission to scan for Bluetooth devices and location services
     * are on.
     */
    private boolean canScan() {
        LocationUtils locationUtils = LocationUtils.getInstance();
        return locationUtils.hasAndroidLocationPermission()
                && locationUtils.isSystemLocationSettingEnabled();
    }

    private void registerBroadcastReceiver() {
        if (mAdapter != null) {
            mAdapter.getContext().registerReceiver(
                    this, new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED));
        }
    }

    private void unregisterBroadcastReceiver() {
        if (mAdapter != null) {
            mAdapter.getContext().unregisterReceiver(this);
        }
    }

    /**
     * Implements callbacks used during a Low Energy scan by notifying upon
     * devices discovered or detecting a scan failure.
     */
    private class ScanCallback extends Wrappers.ScanCallbackWrapper {
        @Override
        public void onBatchScanResult(List<Wrappers.ScanResultWrapper> results) {
            Log.v(TAG, "onBatchScanResults");
        }

        @Override
        public void onScanResult(int callbackType, Wrappers.ScanResultWrapper result) {
            Log.v(TAG, "onScanResult %d %s %s", callbackType, result.getDevice().getAddress(),
                    result.getDevice().getName());

            String[] uuid_strings;
            List<ParcelUuid> uuids = result.getScanRecord_getServiceUuids();

            if (uuids == null) {
                uuid_strings = new String[] {};
            } else {
                uuid_strings = new String[uuids.size()];
                for (int i = 0; i < uuids.size(); i++) {
                    uuid_strings[i] = uuids.get(i).toString();
                }
            }

            nativeCreateOrUpdateDeviceOnScan(mNativeBluetoothAdapterAndroid,
                    result.getDevice().getAddress(), result.getDevice(), result.getRssi(),
                    uuid_strings, result.getScanRecord_getTxPowerLevel());
        }

        @Override
        public void onScanFailed(int errorCode) {
            Log.w(TAG, "onScanFailed: %d", errorCode);
            nativeOnScanFailed(mNativeBluetoothAdapterAndroid);
        }
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();

        if (isPresent() && BluetoothAdapter.ACTION_STATE_CHANGED.equals(action)) {
            int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR);

            Log.w(TAG, "onReceive: BluetoothAdapter.ACTION_STATE_CHANGED: %s",
                    getBluetoothStateString(state));

            switch (state) {
                case BluetoothAdapter.STATE_ON:
                    nativeOnAdapterStateChanged(mNativeBluetoothAdapterAndroid, true);
                    break;
                case BluetoothAdapter.STATE_OFF:
                    nativeOnAdapterStateChanged(mNativeBluetoothAdapterAndroid, false);
                    break;
                default:
                    // do nothing
            }
        }
    }

    private String getBluetoothStateString(int state) {
        switch (state) {
            case BluetoothAdapter.STATE_OFF:
                return "STATE_OFF";
            case BluetoothAdapter.STATE_ON:
                return "STATE_ON";
            case BluetoothAdapter.STATE_TURNING_OFF:
                return "STATE_TURNING_OFF";
            case BluetoothAdapter.STATE_TURNING_ON:
                return "STATE_TURNING_ON";
            default:
                assert false;
                return "illegal state: " + state;
        }
    }

    // ---------------------------------------------------------------------------------------------
    // BluetoothAdapterAndroid C++ methods declared for access from java:

    // Binds to BluetoothAdapterAndroid::OnScanFailed.
    private native void nativeOnScanFailed(long nativeBluetoothAdapterAndroid);

    // Binds to BluetoothAdapterAndroid::CreateOrUpdateDeviceOnScan.
    // 'Object' type must be used for |bluetoothDeviceWrapper| because inner class
    // Wrappers.BluetoothDeviceWrapper reference is not handled by jni_generator.py JavaToJni.
    // http://crbug.com/505554
    private native void nativeCreateOrUpdateDeviceOnScan(long nativeBluetoothAdapterAndroid,
            String address, Object bluetoothDeviceWrapper, int rssi, String[] advertisedUuids,
            int txPower);

    // Binds to BluetoothAdapterAndroid::nativeOnAdapterStateChanged
    private native void nativeOnAdapterStateChanged(
            long nativeBluetoothAdapterAndroid, boolean powered);
}
