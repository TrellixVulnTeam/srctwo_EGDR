// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.device.bluetooth;

import org.chromium.base.Log;
import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;

import java.util.List;

/**
 * Exposes android.bluetooth.BluetoothGattService as necessary
 * for C++ device::BluetoothRemoteGattServiceAndroid.
 *
 * Lifetime is controlled by
 * device::BluetoothRemoteGattServiceAndroid.
 */
@JNINamespace("device")
final class ChromeBluetoothRemoteGattService {
    private static final String TAG = "Bluetooth";

    private long mNativeBluetoothRemoteGattServiceAndroid;
    final Wrappers.BluetoothGattServiceWrapper mService;
    final String mInstanceId;
    ChromeBluetoothDevice mChromeDevice;

    private ChromeBluetoothRemoteGattService(long nativeBluetoothRemoteGattServiceAndroid,
            Wrappers.BluetoothGattServiceWrapper serviceWrapper, String instanceId,
            ChromeBluetoothDevice chromeDevice) {
        mNativeBluetoothRemoteGattServiceAndroid = nativeBluetoothRemoteGattServiceAndroid;
        mService = serviceWrapper;
        mInstanceId = instanceId;
        mChromeDevice = chromeDevice;
        Log.v(TAG, "ChromeBluetoothRemoteGattService created.");
    }

    /**
     * Handles C++ object being destroyed.
     */
    @CalledByNative
    private void onBluetoothRemoteGattServiceAndroidDestruction() {
        mNativeBluetoothRemoteGattServiceAndroid = 0;
    }

    // ---------------------------------------------------------------------------------------------
    // BluetoothRemoteGattServiceAndroid methods implemented in java:

    // Implements BluetoothRemoteGattServiceAndroid::Create.
    // TODO(http://crbug.com/505554): Replace 'Object' with specific type when JNI fixed.
    @CalledByNative
    private static ChromeBluetoothRemoteGattService create(
            long nativeBluetoothRemoteGattServiceAndroid, Object bluetoothGattServiceWrapper,
            String instanceId, ChromeBluetoothDevice chromeDevice) {
        return new ChromeBluetoothRemoteGattService(nativeBluetoothRemoteGattServiceAndroid,
                (Wrappers.BluetoothGattServiceWrapper) bluetoothGattServiceWrapper, instanceId,
                chromeDevice);
    }

    // Implements BluetoothRemoteGattServiceAndroid::GetUUID.
    @CalledByNative
    private String getUUID() {
        return mService.getUuid().toString();
    }

    // Creates objects for all characteristics. Designed only to be called by
    // BluetoothRemoteGattServiceAndroid::EnsureCharacteristicsCreated.
    @CalledByNative
    private void createCharacteristics() {
        List<Wrappers.BluetoothGattCharacteristicWrapper> characteristics =
                mService.getCharacteristics();
        for (Wrappers.BluetoothGattCharacteristicWrapper characteristic : characteristics) {
            // Create an adapter unique characteristic ID. getInstanceId only differs between
            // characteristic instances with the same UUID on this service.
            String characteristicInstanceId = mInstanceId + "/"
                    + characteristic.getUuid().toString() + "," + characteristic.getInstanceId();
            nativeCreateGattRemoteCharacteristic(mNativeBluetoothRemoteGattServiceAndroid,
                    characteristicInstanceId, characteristic, mChromeDevice);
        }
    }

    // ---------------------------------------------------------------------------------------------
    // BluetoothAdapterDevice C++ methods declared for access from java:

    // Binds to BluetoothRemoteGattServiceAndroid::CreateGattRemoteCharacteristic.
    // TODO(http://crbug.com/505554): Replace 'Object' with specific type when JNI fixed.
    private native void nativeCreateGattRemoteCharacteristic(
            long nativeBluetoothRemoteGattServiceAndroid, String instanceId,
            Object bluetoothGattCharacteristicWrapper, Object chromeBluetoothDevice);
}
