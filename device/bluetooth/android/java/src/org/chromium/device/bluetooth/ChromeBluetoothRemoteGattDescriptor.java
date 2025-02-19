// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.device.bluetooth;

import org.chromium.base.Log;
import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;

/**
 * Exposes android.bluetooth.BluetoothGattDescriptor as necessary
 * for C++ device::BluetoothRemoteGattDescriptorAndroid.
 *
 * Lifetime is controlled by device::BluetoothRemoteGattDescriptorAndroid.
 */
@JNINamespace("device")
final class ChromeBluetoothRemoteGattDescriptor {
    private static final String TAG = "Bluetooth";

    private long mNativeBluetoothRemoteGattDescriptorAndroid;
    final Wrappers.BluetoothGattDescriptorWrapper mDescriptor;
    final ChromeBluetoothDevice mChromeDevice;

    private ChromeBluetoothRemoteGattDescriptor(long nativeBluetoothRemoteGattDescriptorAndroid,
            Wrappers.BluetoothGattDescriptorWrapper descriptorWrapper,
            ChromeBluetoothDevice chromeDevice) {
        mNativeBluetoothRemoteGattDescriptorAndroid = nativeBluetoothRemoteGattDescriptorAndroid;
        mDescriptor = descriptorWrapper;
        mChromeDevice = chromeDevice;

        mChromeDevice.mWrapperToChromeDescriptorsMap.put(descriptorWrapper, this);

        Log.v(TAG, "ChromeBluetoothRemoteGattDescriptor created.");
    }

    /**
     * Handles C++ object being destroyed.
     */
    @CalledByNative
    private void onBluetoothRemoteGattDescriptorAndroidDestruction() {
        Log.v(TAG, "ChromeBluetoothRemoteGattDescriptor Destroyed.");
        mNativeBluetoothRemoteGattDescriptorAndroid = 0;
        mChromeDevice.mWrapperToChromeDescriptorsMap.remove(mDescriptor);
    }

    void onDescriptorRead(int status) {
        Log.i(TAG, "onDescriptorRead status:%d==%s", status,
                status == android.bluetooth.BluetoothGatt.GATT_SUCCESS ? "OK" : "Error");
        if (mNativeBluetoothRemoteGattDescriptorAndroid != 0) {
            nativeOnRead(
                    mNativeBluetoothRemoteGattDescriptorAndroid, status, mDescriptor.getValue());
        }
    }

    void onDescriptorWrite(int status) {
        Log.i(TAG, "onDescriptorWrite status:%d==%s", status,
                status == android.bluetooth.BluetoothGatt.GATT_SUCCESS ? "OK" : "Error");
        if (mNativeBluetoothRemoteGattDescriptorAndroid != 0) {
            nativeOnWrite(mNativeBluetoothRemoteGattDescriptorAndroid, status);
        }
    }

    // ---------------------------------------------------------------------------------------------
    // BluetoothRemoteGattDescriptorAndroid methods implemented in java:

    // Implements BluetoothRemoteGattDescriptorAndroid::Create.
    // TODO(http://crbug.com/505554): Replace 'Object' with specific type when JNI fixed.
    @CalledByNative
    private static ChromeBluetoothRemoteGattDescriptor create(
            long nativeBluetoothRemoteGattDescriptorAndroid, Object bluetoothGattDescriptorWrapper,
            ChromeBluetoothDevice chromeDevice) {
        return new ChromeBluetoothRemoteGattDescriptor(nativeBluetoothRemoteGattDescriptorAndroid,
                (Wrappers.BluetoothGattDescriptorWrapper) bluetoothGattDescriptorWrapper,
                chromeDevice);
    }

    // Implements BluetoothRemoteGattDescriptorAndroid::GetUUID.
    @CalledByNative
    private String getUUID() {
        return mDescriptor.getUuid().toString();
    }

    // Implements BluetoothRemoteGattDescriptorAndroid::ReadRemoteDescriptor.
    @CalledByNative
    private boolean readRemoteDescriptor() {
        if (!mChromeDevice.mBluetoothGatt.readDescriptor(mDescriptor)) {
            Log.i(TAG, "readRemoteDescriptor readDescriptor failed.");
            return false;
        }
        return true;
    }

    // Implements BluetoothRemoteGattDescriptorAndroid::WriteRemoteDescriptor.
    @CalledByNative
    private boolean writeRemoteDescriptor(byte[] value) {
        if (!mDescriptor.setValue(value)) {
            Log.i(TAG, "writeRemoteDescriptor setValue failed.");
            return false;
        }
        if (!mChromeDevice.mBluetoothGatt.writeDescriptor(mDescriptor)) {
            Log.i(TAG, "writeRemoteDescriptor writeDescriptor failed.");
            return false;
        }
        return true;
    }

    // ---------------------------------------------------------------------------------------------
    // BluetoothAdapterDevice C++ methods declared for access from java:

    // Binds to BluetoothRemoteGattDescriptorAndroid::OnRead.
    native void nativeOnRead(
            long nativeBluetoothRemoteGattDescriptorAndroid, int status, byte[] value);

    // Binds to BluetoothRemoteGattDescriptorAndroid::OnWrite.
    native void nativeOnWrite(long nativeBluetoothRemoteGattDescriptorAndroid, int status);
}
