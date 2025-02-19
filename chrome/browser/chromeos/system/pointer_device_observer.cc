// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/system/pointer_device_observer.h"

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "chrome/browser/chromeos/system/input_device_settings.h"
#include "content/public/browser/browser_thread.h"
#include "ui/events/devices/input_device_manager.h"

using content::BrowserThread;

namespace chromeos {
namespace system {

PointerDeviceObserver::PointerDeviceObserver()
    : weak_factory_(this) {
}

PointerDeviceObserver::~PointerDeviceObserver() {
  ui::InputDeviceManager::GetInstance()->RemoveObserver(this);
}

void PointerDeviceObserver::Init() {
  ui::InputDeviceManager::GetInstance()->AddObserver(this);
}

void PointerDeviceObserver::CheckDevices() {
  CheckMouseExists();
  CheckTouchpadExists();
}

void PointerDeviceObserver::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void PointerDeviceObserver::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

void PointerDeviceObserver::OnMouseDeviceConfigurationChanged() {
  CheckDevices();
}

void PointerDeviceObserver::OnTouchpadDeviceConfigurationChanged() {
  CheckDevices();
}

void PointerDeviceObserver::CheckTouchpadExists() {
  InputDeviceSettings::Get()->TouchpadExists(
      base::Bind(&PointerDeviceObserver::OnTouchpadExists,
                 weak_factory_.GetWeakPtr()));
}

void PointerDeviceObserver::CheckMouseExists() {
  InputDeviceSettings::Get()->MouseExists(
      base::Bind(&PointerDeviceObserver::OnMouseExists,
                 weak_factory_.GetWeakPtr()));
}

void PointerDeviceObserver::OnTouchpadExists(bool exists) {
  for (auto& observer : observers_)
    observer.TouchpadExists(exists);
}

void PointerDeviceObserver::OnMouseExists(bool exists) {
  for (auto& observer : observers_)
    observer.MouseExists(exists);
}

PointerDeviceObserver::Observer::~Observer() {
}

}  // namespace system
}  // namespace chromeos
