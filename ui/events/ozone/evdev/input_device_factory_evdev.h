// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_OZONE_EVDEV_INPUT_DEVICE_FACTORY_EVDEV_H_
#define UI_EVENTS_OZONE_EVDEV_INPUT_DEVICE_FACTORY_EVDEV_H_

#include <map>
#include <memory>
#include <set>
#include <vector>

#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/task_runner.h"
#include "ui/events/ozone/evdev/event_converter_evdev.h"
#include "ui/events/ozone/evdev/event_device_info.h"
#include "ui/events/ozone/evdev/events_ozone_evdev_export.h"
#include "ui/events/ozone/evdev/input_device_settings_evdev.h"
#include "ui/ozone/public/input_controller.h"

namespace ui {

class CursorDelegateEvdev;
class DeviceEventDispatcherEvdev;

#if !defined(USE_EVDEV)
#error Missing dependency on ui/events/ozone:events_ozone_evdev
#endif

#if defined(USE_EVDEV_GESTURES)
class GesturePropertyProvider;
#endif

// Manager for event device objects. All device I/O starts here.
class EVENTS_OZONE_EVDEV_EXPORT InputDeviceFactoryEvdev {
 public:
  InputDeviceFactoryEvdev(
      std::unique_ptr<DeviceEventDispatcherEvdev> dispatcher,
      CursorDelegateEvdev* cursor);
  ~InputDeviceFactoryEvdev();

  // Open & start reading a newly plugged-in input device.
  void AddInputDevice(int id, const base::FilePath& path);

  // Stop reading & close an unplugged input device.
  void RemoveInputDevice(const base::FilePath& path);

  // Device thread handler for initial scan completion.
  void OnStartupScanComplete();

  // LED state.
  void SetCapsLockLed(bool enabled);

  // Bits from InputController that have to be answered on IO.
  void UpdateInputDeviceSettings(const InputDeviceSettingsEvdev& settings);
  void GetTouchDeviceStatus(InputController::GetTouchDeviceStatusReply reply);
  void GetTouchEventLog(const base::FilePath& out_dir,
                        InputController::GetTouchEventLogReply reply);

  base::WeakPtr<InputDeviceFactoryEvdev> GetWeakPtr();

  void EnablePalmSuppression(bool enabled);

 private:
  // Open device at path & starting processing events.
  void AttachInputDevice(std::unique_ptr<EventConverterEvdev> converter);

  // Close device at path.
  void DetachInputDevice(const base::FilePath& file_path);

  // Sync input_device_settings_ to attached devices.
  void ApplyInputDeviceSettings();
  void ApplyCapsLockLed();

  // Policy for device enablement.
  bool IsDeviceEnabled(const EventConverterEvdev* converter);

  // Update observers on device changes.
  void UpdateDirtyFlags(const EventConverterEvdev* converter);
  void NotifyDevicesUpdated();
  void NotifyKeyboardsUpdated();
  void NotifyTouchscreensUpdated();
  void NotifyMouseDevicesUpdated();
  void NotifyTouchpadDevicesUpdated();
  void NotifyGamepadDevicesUpdated();

  void SetIntPropertyForOneType(const EventDeviceType type,
                                const std::string& name,
                                int value);
  void SetBoolPropertyForOneType(const EventDeviceType type,
                                 const std::string& name,
                                 bool value);

  // Task runner for our thread.
  scoped_refptr<base::TaskRunner> task_runner_;

  // Cursor movement.
  CursorDelegateEvdev* cursor_;

#if defined(USE_EVDEV_GESTURES)
  // Gesture library property provider (used by touchpads/mice).
  std::unique_ptr<GesturePropertyProvider> gesture_property_provider_;
#endif

  // Dispatcher for events.
  std::unique_ptr<DeviceEventDispatcherEvdev> dispatcher_;

  // Number of pending device additions & device classes.
  int pending_device_changes_ = 0;
  bool touchscreen_list_dirty_ = true;
  bool keyboard_list_dirty_ = true;
  bool mouse_list_dirty_ = true;
  bool touchpad_list_dirty_ = true;
  bool gamepad_list_dirty_ = true;

  // Whether we have a list of devices that were present at startup.
  bool startup_devices_enumerated_ = false;

  // Whether devices that were present at startup are open.
  bool startup_devices_opened_ = false;

  // LEDs.
  bool caps_lock_led_enabled_ = false;

  // Whether touch palm suppression is enabled.
  bool palm_suppression_enabled_ = false;

  // Device settings. These primarily affect libgestures behavior.
  InputDeviceSettingsEvdev input_device_settings_;

  // Owned per-device event converters (by path).
  // NB: This should be destroyed early, before any shared state.
  std::map<base::FilePath, std::unique_ptr<EventConverterEvdev>> converters_;

  // Support weak pointers for attach & detach callbacks.
  base::WeakPtrFactory<InputDeviceFactoryEvdev> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(InputDeviceFactoryEvdev);
};

}  // namespace ui

#endif  // UI_EVENTS_OZONE_EVDEV_INPUT_DEVICE_FACTORY_EVDEV_H_
