// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.device.gamepad;

import android.annotation.SuppressLint;
import android.content.Context;
import android.hardware.input.InputManager;
import android.hardware.input.InputManager.InputDeviceListener;
import android.view.InputDevice;
import android.view.InputEvent;
import android.view.KeyEvent;
import android.view.MotionEvent;

import org.chromium.base.ThreadUtils;
import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;

/**
 * Class to manage connected gamepad devices list.
 *
 * It is a Java counterpart of GamepadPlatformDataFetcherAndroid and feeds Gamepad API with input
 * data.
 */
@JNINamespace("device")
public class GamepadList {
    private static final int MAX_GAMEPADS = 4;

    private final Object mLock = new Object();

    private final GamepadDevice[] mGamepadDevices = new GamepadDevice[MAX_GAMEPADS];
    private InputManager mInputManager;
    private int mAttachedToWindowCounter;
    private boolean mIsGamepadAPIActive;
    private InputDeviceListener mInputDeviceListener;

    private GamepadList() {
        mInputDeviceListener = new InputDeviceListener() {
            // Override InputDeviceListener methods
            @Override
            public void onInputDeviceChanged(int deviceId) {
                onInputDeviceChangedImpl(deviceId);
            }

            @Override
            public void onInputDeviceRemoved(int deviceId) {
                onInputDeviceRemovedImpl(deviceId);
            }

            @Override
            public void onInputDeviceAdded(int deviceId) {
                onInputDeviceAddedImpl(deviceId);
            }
        };
    }

    private void initializeDevices() {
        // Get list of all the attached input devices.
        int[] deviceIds = mInputManager.getInputDeviceIds();
        for (int i = 0; i < deviceIds.length; i++) {
            InputDevice inputDevice = InputDevice.getDevice(deviceIds[i]);
            // Check for gamepad device
            if (isGamepadDevice(inputDevice)) {
                // Register a new gamepad device.
                registerGamepad(inputDevice);
            }
        }
    }

    /**
     * Notifies the GamepadList that a {@link ContentView} is attached to a window and it should
     * prepare itself for gamepad input. It must be called before {@link onGenericMotionEvent} and
     * {@link dispatchKeyEvent}.
     */
    public static void onAttachedToWindow(Context context) {
        assert ThreadUtils.runningOnUiThread();
        getInstance().attachedToWindow(context);
    }

    private void attachedToWindow(Context context) {
        if (mAttachedToWindowCounter++ == 0) {
            mInputManager = (InputManager) context.getSystemService(Context.INPUT_SERVICE);
            synchronized (mLock) {
                initializeDevices();
            }
            // Register an input device listener.
            mInputManager.registerInputDeviceListener(mInputDeviceListener, null);
        }
    }

    /**
     * Notifies the GamepadList that a {@link ContentView} is detached from it's window.
     */
    @SuppressLint("MissingSuperCall")
    public static void onDetachedFromWindow() {
        assert ThreadUtils.runningOnUiThread();
        getInstance().detachedFromWindow();
    }

    private void detachedFromWindow() {
        if (--mAttachedToWindowCounter == 0) {
            synchronized (mLock) {
                for (int i = 0; i < MAX_GAMEPADS; ++i) {
                    mGamepadDevices[i] = null;
                }
            }
            mInputManager.unregisterInputDeviceListener(mInputDeviceListener);
            mInputManager = null;
        }
    }

    // ------------------------------------------------------------

    private void onInputDeviceChangedImpl(int deviceId) {}

    private void onInputDeviceRemovedImpl(int deviceId) {
        synchronized (mLock) {
            unregisterGamepad(deviceId);
        }
    }

    private void onInputDeviceAddedImpl(int deviceId) {
        InputDevice inputDevice = InputDevice.getDevice(deviceId);
        if (!isGamepadDevice(inputDevice)) return;
        synchronized (mLock) {
            registerGamepad(inputDevice);
        }
    }

    // ------------------------------------------------------------

    private static GamepadList getInstance() {
        return LazyHolder.INSTANCE;
    }

    private int getDeviceCount() {
        int count = 0;
        for (int i = 0; i < MAX_GAMEPADS; i++) {
            if (getDevice(i) != null) {
                count++;
            }
        }
        return count;
    }

    private boolean isDeviceConnected(int index) {
        if (index < MAX_GAMEPADS && getDevice(index) != null) {
            return true;
        }
        return false;
    }

    private GamepadDevice getDeviceById(int deviceId) {
        for (int i = 0; i < MAX_GAMEPADS; i++) {
            GamepadDevice gamepad = mGamepadDevices[i];
            if (gamepad != null && gamepad.getId() == deviceId) {
                return gamepad;
            }
        }
        return null;
    }

    private GamepadDevice getDevice(int index) {
        // Maximum 4 Gamepads can be connected at a time starting at index zero.
        assert index >= 0 && index < MAX_GAMEPADS;
        return mGamepadDevices[index];
    }

    /**
     * Handles key events from the gamepad devices.
     * @return True if the event has been consumed.
     */
    public static boolean dispatchKeyEvent(KeyEvent event) {
        if (!isGamepadEvent(event)) return false;
        return getInstance().handleKeyEvent(event);
    }

    private boolean handleKeyEvent(KeyEvent event) {
        synchronized (mLock) {
            if (!mIsGamepadAPIActive) return false;
            GamepadDevice gamepad = getGamepadForEvent(event);
            if (gamepad == null) return false;
            return gamepad.handleKeyEvent(event);
        }
    }

    /**
     * Handles motion events from the gamepad devices.
     * @return True if the event has been consumed.
     */
    public static boolean onGenericMotionEvent(MotionEvent event) {
        if (!isGamepadEvent(event)) return false;
        return getInstance().handleMotionEvent(event);
    }

    private boolean handleMotionEvent(MotionEvent event) {
        synchronized (mLock) {
            if (!mIsGamepadAPIActive) return false;
            GamepadDevice gamepad = getGamepadForEvent(event);
            if (gamepad == null) return false;
            return gamepad.handleMotionEvent(event);
        }
    }

    private int getNextAvailableIndex() {
        // When multiple gamepads are connected to a user agent, indices must be assigned on a
        // first-come first-serve basis, starting at zero. If a gamepad is disconnected, previously
        // assigned indices must not be reassigned to gamepads that continue to be connected.
        // However, if a gamepad is disconnected, and subsequently the same or a different
        // gamepad is then connected, index entries must be reused.

        for (int i = 0; i < MAX_GAMEPADS; ++i) {
            if (getDevice(i) == null) {
                return i;
            }
        }
        // Reached maximum gamepads limit.
        return -1;
    }

    private boolean registerGamepad(InputDevice inputDevice) {
        int index = getNextAvailableIndex();
        if (index == -1) return false; // invalid index

        GamepadDevice gamepad = new GamepadDevice(index, inputDevice);
        mGamepadDevices[index] = gamepad;
        return true;
    }

    private void unregisterGamepad(int deviceId) {
        GamepadDevice gamepadDevice = getDeviceById(deviceId);
        if (gamepadDevice == null) return; // Not a registered device.
        int index = gamepadDevice.getIndex();
        mGamepadDevices[index] = null;
    }

    private static boolean isGamepadDevice(InputDevice inputDevice) {
        if (inputDevice == null) return false;
        return ((inputDevice.getSources() & InputDevice.SOURCE_JOYSTICK)
                == InputDevice.SOURCE_JOYSTICK);
    }

    private GamepadDevice getGamepadForEvent(InputEvent event) {
        return getDeviceById(event.getDeviceId());
    }

    /**
     * @return True if HTML5 gamepad API is active.
     */
    public static boolean isGamepadAPIActive() {
        return getInstance().mIsGamepadAPIActive;
    }

    /**
     * @return True if the motion event corresponds to a gamepad event.
     */
    public static boolean isGamepadEvent(MotionEvent event) {
        return ((event.getSource() & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK);
    }

    /**
     * @return True if event's keycode corresponds to a gamepad key.
     */
    public static boolean isGamepadEvent(KeyEvent event) {
        int keyCode = event.getKeyCode();
        switch (keyCode) {
            // Specific handling for dpad keys is required because
            // KeyEvent.isGamepadButton doesn't consider dpad keys.
            case KeyEvent.KEYCODE_DPAD_UP:
            case KeyEvent.KEYCODE_DPAD_DOWN:
            case KeyEvent.KEYCODE_DPAD_LEFT:
            case KeyEvent.KEYCODE_DPAD_RIGHT:
                return true;
            default:
                return KeyEvent.isGamepadButton(keyCode);
        }
    }

    @CalledByNative
    static void updateGamepadData(long webGamepadsPtr) {
        getInstance().grabGamepadData(webGamepadsPtr);
    }

    private void grabGamepadData(long webGamepadsPtr) {
        synchronized (mLock) {
            for (int i = 0; i < MAX_GAMEPADS; i++) {
                final GamepadDevice device = getDevice(i);
                if (device != null) {
                    device.updateButtonsAndAxesMapping();
                    nativeSetGamepadData(webGamepadsPtr, i, device.isStandardGamepad(), true,
                            device.getName(), device.getTimestamp(), device.getAxes(),
                            device.getButtons());
                } else {
                    nativeSetGamepadData(webGamepadsPtr, i, false, false, null, 0, null, null);
                }
            }
        }
    }

    @CalledByNative
    static void setGamepadAPIActive(boolean isActive) {
        getInstance().setIsGamepadActive(isActive);
    }

    private void setIsGamepadActive(boolean isGamepadActive) {
        synchronized (mLock) {
            mIsGamepadAPIActive = isGamepadActive;
            if (isGamepadActive) {
                for (int i = 0; i < MAX_GAMEPADS; i++) {
                    GamepadDevice gamepadDevice = getDevice(i);
                    if (gamepadDevice == null) continue;
                    gamepadDevice.clearData();
                }
            }
        }
    }

    private native void nativeSetGamepadData(long webGamepadsPtr, int index, boolean mapping,
            boolean connected, String devicename, long timestamp, float[] axes, float[] buttons);

    private static class LazyHolder {
        private static final GamepadList INSTANCE = new GamepadList();
    }
}
