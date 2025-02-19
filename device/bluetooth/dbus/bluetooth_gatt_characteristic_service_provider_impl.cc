// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/bluetooth/dbus/bluetooth_gatt_characteristic_service_provider_impl.h"

#include <cstddef>

#include "base/bind.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "device/bluetooth/dbus/bluetooth_gatt_attribute_helpers.h"
#include "third_party/cros_system_api/dbus/service_constants.h"

namespace bluez {

namespace {

const char kErrorInvalidArgs[] = "org.freedesktop.DBus.Error.InvalidArgs";
const char kErrorPropertyReadOnly[] =
    "org.freedesktop.DBus.Error.PropertyReadOnly";
const char kErrorFailed[] = "org.freedesktop.DBus.Error.Failed";

}  // namespace

BluetoothGattCharacteristicServiceProviderImpl::
    BluetoothGattCharacteristicServiceProviderImpl(
        dbus::Bus* bus,
        const dbus::ObjectPath& object_path,
        std::unique_ptr<BluetoothGattAttributeValueDelegate> delegate,
        const std::string& uuid,
        const std::vector<std::string>& flags,
        const dbus::ObjectPath& service_path)
    : origin_thread_id_(base::PlatformThread::CurrentId()),
      uuid_(uuid),
      flags_(flags),
      bus_(bus),
      delegate_(std::move(delegate)),
      object_path_(object_path),
      service_path_(service_path),
      weak_ptr_factory_(this) {
  VLOG(1) << "Created Bluetooth GATT characteristic: " << object_path.value()
          << " UUID: " << uuid;
  if (!bus_)
    return;

  DCHECK(delegate_);
  DCHECK(!uuid_.empty());
  DCHECK(object_path_.IsValid());
  DCHECK(service_path_.IsValid());
  DCHECK(base::StartsWith(object_path_.value(), service_path_.value() + "/",
                          base::CompareCase::SENSITIVE));

  exported_object_ = bus_->GetExportedObject(object_path_);

  // org.freedesktop.DBus.Properties interface:
  exported_object_->ExportMethod(
      dbus::kDBusPropertiesInterface, dbus::kDBusPropertiesGet,
      base::Bind(&BluetoothGattCharacteristicServiceProviderImpl::Get,
                 weak_ptr_factory_.GetWeakPtr()),
      base::Bind(&BluetoothGattCharacteristicServiceProviderImpl::OnExported,
                 weak_ptr_factory_.GetWeakPtr()));
  exported_object_->ExportMethod(
      dbus::kDBusPropertiesInterface, dbus::kDBusPropertiesSet,
      base::Bind(&BluetoothGattCharacteristicServiceProviderImpl::Set,
                 weak_ptr_factory_.GetWeakPtr()),
      base::Bind(&BluetoothGattCharacteristicServiceProviderImpl::OnExported,
                 weak_ptr_factory_.GetWeakPtr()));
  exported_object_->ExportMethod(
      dbus::kDBusPropertiesInterface, dbus::kDBusPropertiesGetAll,
      base::Bind(&BluetoothGattCharacteristicServiceProviderImpl::GetAll,
                 weak_ptr_factory_.GetWeakPtr()),
      base::Bind(&BluetoothGattCharacteristicServiceProviderImpl::OnExported,
                 weak_ptr_factory_.GetWeakPtr()));

  // org.bluez.GattCharacteristic1 interface:
  exported_object_->ExportMethod(
      bluetooth_gatt_characteristic::kBluetoothGattCharacteristicInterface,
      bluetooth_gatt_characteristic::kReadValue,
      base::Bind(&BluetoothGattCharacteristicServiceProviderImpl::ReadValue,
                 weak_ptr_factory_.GetWeakPtr()),
      base::Bind(&BluetoothGattCharacteristicServiceProviderImpl::OnExported,
                 weak_ptr_factory_.GetWeakPtr()));
  exported_object_->ExportMethod(
      bluetooth_gatt_characteristic::kBluetoothGattCharacteristicInterface,
      bluetooth_gatt_characteristic::kWriteValue,
      base::Bind(&BluetoothGattCharacteristicServiceProviderImpl::WriteValue,
                 weak_ptr_factory_.GetWeakPtr()),
      base::Bind(&BluetoothGattCharacteristicServiceProviderImpl::OnExported,
                 weak_ptr_factory_.GetWeakPtr()));
  exported_object_->ExportMethod(
      bluetooth_gatt_characteristic::kBluetoothGattCharacteristicInterface,
      bluetooth_gatt_characteristic::kStartNotify,
      base::Bind(&BluetoothGattCharacteristicServiceProviderImpl::StartNotify,
                 weak_ptr_factory_.GetWeakPtr()),
      base::Bind(&BluetoothGattCharacteristicServiceProviderImpl::OnExported,
                 weak_ptr_factory_.GetWeakPtr()));
  exported_object_->ExportMethod(
      bluetooth_gatt_characteristic::kBluetoothGattCharacteristicInterface,
      bluetooth_gatt_characteristic::kStopNotify,
      base::Bind(&BluetoothGattCharacteristicServiceProviderImpl::StopNotify,
                 weak_ptr_factory_.GetWeakPtr()),
      base::Bind(&BluetoothGattCharacteristicServiceProviderImpl::OnExported,
                 weak_ptr_factory_.GetWeakPtr()));
}

BluetoothGattCharacteristicServiceProviderImpl::
    ~BluetoothGattCharacteristicServiceProviderImpl() {
  VLOG(1) << "Cleaning up Bluetooth GATT characteristic: "
          << object_path_.value();
  if (bus_)
    bus_->UnregisterExportedObject(object_path_);
}

void BluetoothGattCharacteristicServiceProviderImpl::SendValueChanged(
    const std::vector<uint8_t>& value) {
  // Running a test, don't actually try to write to use DBus.
  if (!bus_)
    return;

  VLOG(2) << "Emitting a PropertiesChanged signal for characteristic value.";
  dbus::Signal signal(dbus::kDBusPropertiesInterface,
                      dbus::kDBusPropertiesChangedSignal);
  dbus::MessageWriter writer(&signal);
  dbus::MessageWriter array_writer(NULL);
  dbus::MessageWriter dict_entry_writer(NULL);
  dbus::MessageWriter variant_writer(NULL);

  // interface_name
  writer.AppendString(
      bluetooth_gatt_characteristic::kBluetoothGattCharacteristicInterface);

  // changed_properties
  writer.OpenArray("{sv}", &array_writer);
  array_writer.OpenDictEntry(&dict_entry_writer);
  dict_entry_writer.AppendString(bluetooth_gatt_characteristic::kValueProperty);
  dict_entry_writer.OpenVariant("ay", &variant_writer);
  variant_writer.AppendArrayOfBytes(value.data(), value.size());
  dict_entry_writer.CloseContainer(&variant_writer);
  array_writer.CloseContainer(&dict_entry_writer);
  writer.CloseContainer(&array_writer);

  // invalidated_properties.
  writer.OpenArray("s", &array_writer);
  writer.CloseContainer(&array_writer);

  exported_object_->SendSignal(&signal);
}

// Returns true if the current thread is on the origin thread.
bool BluetoothGattCharacteristicServiceProviderImpl::OnOriginThread() {
  return base::PlatformThread::CurrentId() == origin_thread_id_;
}

void BluetoothGattCharacteristicServiceProviderImpl::Get(
    dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender) {
  VLOG(2) << "BluetoothGattCharacteristicServiceProvider::Get: "
          << object_path_.value();
  DCHECK(OnOriginThread());

  dbus::MessageReader reader(method_call);

  std::string interface_name;
  std::string property_name;
  if (!reader.PopString(&interface_name) || !reader.PopString(&property_name) ||
      reader.HasMoreData()) {
    std::unique_ptr<dbus::ErrorResponse> error_response =
        dbus::ErrorResponse::FromMethodCall(method_call, kErrorInvalidArgs,
                                            "Expected 'ss'.");
    response_sender.Run(std::move(error_response));
    return;
  }

  // Only the GATT characteristic interface is supported.
  if (interface_name !=
      bluetooth_gatt_characteristic::kBluetoothGattCharacteristicInterface) {
    std::unique_ptr<dbus::ErrorResponse> error_response =
        dbus::ErrorResponse::FromMethodCall(
            method_call, kErrorInvalidArgs,
            "No such interface: '" + interface_name + "'.");
    response_sender.Run(std::move(error_response));
    return;
  }

  std::unique_ptr<dbus::Response> response =
      dbus::Response::FromMethodCall(method_call);
  dbus::MessageWriter writer(response.get());
  dbus::MessageWriter variant_writer(NULL);

  if (property_name == bluetooth_gatt_characteristic::kUUIDProperty) {
    writer.OpenVariant("s", &variant_writer);
    variant_writer.AppendString(uuid_);
    writer.CloseContainer(&variant_writer);
  } else if (property_name == bluetooth_gatt_characteristic::kServiceProperty) {
    writer.OpenVariant("o", &variant_writer);
    variant_writer.AppendObjectPath(service_path_);
    writer.CloseContainer(&variant_writer);
  } else if (property_name == bluetooth_gatt_characteristic::kFlagsProperty) {
    writer.OpenVariant("as", &variant_writer);
    variant_writer.AppendArrayOfStrings(flags_);
    writer.CloseContainer(&variant_writer);
  } else {
    response = dbus::ErrorResponse::FromMethodCall(
        method_call, kErrorInvalidArgs,
        "No such property: '" + property_name + "'.");
  }

  response_sender.Run(std::move(response));
}

void BluetoothGattCharacteristicServiceProviderImpl::Set(
    dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender) {
  VLOG(2) << "BluetoothGattCharacteristicServiceProviderImpl::Set: "
          << object_path_.value();
  DCHECK(OnOriginThread());
  // All of the properties on this interface are read-only, so just return
  // error.
  std::unique_ptr<dbus::ErrorResponse> error_response =
      dbus::ErrorResponse::FromMethodCall(method_call, kErrorPropertyReadOnly,
                                          "All properties are read-only.");
  response_sender.Run(std::move(error_response));
}

void BluetoothGattCharacteristicServiceProviderImpl::GetAll(
    dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender) {
  VLOG(2) << "BluetoothGattCharacteristicServiceProvider::GetAll: "
          << object_path_.value();
  DCHECK(OnOriginThread());

  dbus::MessageReader reader(method_call);

  std::string interface_name;
  if (!reader.PopString(&interface_name) || reader.HasMoreData()) {
    std::unique_ptr<dbus::ErrorResponse> error_response =
        dbus::ErrorResponse::FromMethodCall(method_call, kErrorInvalidArgs,
                                            "Expected 's'.");
    response_sender.Run(std::move(error_response));
    return;
  }

  // Only the GATT characteristic interface is supported.
  if (interface_name !=
      bluetooth_gatt_characteristic::kBluetoothGattCharacteristicInterface) {
    std::unique_ptr<dbus::ErrorResponse> error_response =
        dbus::ErrorResponse::FromMethodCall(
            method_call, kErrorInvalidArgs,
            "No such interface: '" + interface_name + "'.");
    response_sender.Run(std::move(error_response));
    return;
  }

  std::unique_ptr<dbus::Response> response =
      dbus::Response::FromMethodCall(method_call);
  dbus::MessageWriter writer(response.get());
  WriteProperties(&writer);
  response_sender.Run(std::move(response));
}

void BluetoothGattCharacteristicServiceProviderImpl::ReadValue(
    dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender) {
  VLOG(3) << "BluetoothGattCharacteristicServiceProvider::ReadValue: "
          << object_path_.value();
  DCHECK(OnOriginThread());

  dbus::MessageReader reader(method_call);
  dbus::ObjectPath device_path = ReadDevicePath(&reader);
  if (device_path.value().empty()) {
    LOG(WARNING) << "ReadValue called with incorrect parameters: "
                 << method_call->ToString();
    // Continue on with an empty device path. This will return a null device to
    // the delegate, which should know how to handle it.
  }

  DCHECK(delegate_);
  delegate_->GetValue(
      device_path,
      base::Bind(&BluetoothGattCharacteristicServiceProviderImpl::OnReadValue,
                 weak_ptr_factory_.GetWeakPtr(), method_call, response_sender),
      base::Bind(&BluetoothGattCharacteristicServiceProviderImpl::OnFailure,
                 weak_ptr_factory_.GetWeakPtr(), method_call, response_sender));
}

void BluetoothGattCharacteristicServiceProviderImpl::WriteValue(
    dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender) {
  VLOG(3) << "BluetoothGattCharacteristicServiceProvider::WriteValue: "
          << object_path_.value();
  DCHECK(OnOriginThread());

  dbus::MessageReader reader(method_call);
  const uint8_t* bytes = NULL;
  size_t length = 0;

  std::vector<uint8_t> value;
  if (!reader.PopArrayOfBytes(&bytes, &length)) {
    LOG(WARNING) << "Error reading value parameter. WriteValue called with "
                    "incorrect parameters: "
                 << method_call->ToString();
  }
  if (bytes)
    value.assign(bytes, bytes + length);

  dbus::ObjectPath device_path = ReadDevicePath(&reader);
  if (device_path.value().empty()) {
    LOG(WARNING) << "WriteValue called with incorrect parameters: "
                 << method_call->ToString();
    // Continue on with an empty device path. This will return a null device to
    // the delegate, which should know how to handle it.
  }

  DCHECK(delegate_);
  delegate_->SetValue(
      device_path, value,
      base::Bind(&BluetoothGattCharacteristicServiceProviderImpl::OnWriteValue,
                 weak_ptr_factory_.GetWeakPtr(), method_call, response_sender),
      base::Bind(&BluetoothGattCharacteristicServiceProviderImpl::OnFailure,
                 weak_ptr_factory_.GetWeakPtr(), method_call, response_sender));
}

void BluetoothGattCharacteristicServiceProviderImpl::StartNotify(
    dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender) {
  VLOG(3) << "BluetoothGattCharacteristicServiceProvider::StartNotify: "
          << object_path_.value();
  DCHECK(OnOriginThread());
  DCHECK(delegate_);
  delegate_->StartNotifications();
}

void BluetoothGattCharacteristicServiceProviderImpl::StopNotify(
    dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender) {
  VLOG(3) << "BluetoothGattCharacteristicServiceProvider::StopNotify: "
          << object_path_.value();
  DCHECK(OnOriginThread());

  DCHECK(delegate_);
  delegate_->StopNotifications();
}

void BluetoothGattCharacteristicServiceProviderImpl::OnExported(
    const std::string& interface_name,
    const std::string& method_name,
    bool success) {
  LOG_IF(WARNING, !success) << "Failed to export " << interface_name << "."
                            << method_name;
}

void BluetoothGattCharacteristicServiceProviderImpl::OnReadValue(
    dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender,
    const std::vector<uint8_t>& value) {
  VLOG(3) << "Characteristic value obtained from delegate. Responding to "
             "ReadValue.";

  std::unique_ptr<dbus::Response> response =
      dbus::Response::FromMethodCall(method_call);
  dbus::MessageWriter writer(response.get());
  writer.AppendArrayOfBytes(value.data(), value.size());
  response_sender.Run(std::move(response));
}

void BluetoothGattCharacteristicServiceProviderImpl::OnWriteValue(
    dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender) {
  VLOG(3) << "Responding to WriteValue.";

  std::unique_ptr<dbus::Response> response =
      dbus::Response::FromMethodCall(method_call);
  response_sender.Run(std::move(response));
}

void BluetoothGattCharacteristicServiceProviderImpl::WriteProperties(
    dbus::MessageWriter* writer) {
  dbus::MessageWriter array_writer(NULL);
  dbus::MessageWriter dict_entry_writer(NULL);
  dbus::MessageWriter variant_writer(NULL);

  writer->OpenArray("{sv}", &array_writer);

  // UUID:
  array_writer.OpenDictEntry(&dict_entry_writer);
  dict_entry_writer.AppendString(bluetooth_gatt_characteristic::kUUIDProperty);
  dict_entry_writer.AppendVariantOfString(uuid_);
  array_writer.CloseContainer(&dict_entry_writer);

  // Service:
  array_writer.OpenDictEntry(&dict_entry_writer);
  dict_entry_writer.AppendString(
      bluetooth_gatt_characteristic::kServiceProperty);
  dict_entry_writer.AppendVariantOfObjectPath(service_path_);
  array_writer.CloseContainer(&dict_entry_writer);

  // Flags:
  array_writer.OpenDictEntry(&dict_entry_writer);
  dict_entry_writer.AppendString(bluetooth_gatt_characteristic::kFlagsProperty);
  dict_entry_writer.OpenVariant("as", &variant_writer);
  variant_writer.AppendArrayOfStrings(flags_);
  dict_entry_writer.CloseContainer(&variant_writer);
  array_writer.CloseContainer(&dict_entry_writer);

  writer->CloseContainer(&array_writer);
}

void BluetoothGattCharacteristicServiceProviderImpl::OnFailure(
    dbus::MethodCall* method_call,
    dbus::ExportedObject::ResponseSender response_sender) {
  VLOG(2) << "Failed to get/set characteristic value. Report error.";
  std::unique_ptr<dbus::ErrorResponse> error_response =
      dbus::ErrorResponse::FromMethodCall(
          method_call, kErrorFailed, "Failed to get/set characteristic value.");
  response_sender.Run(std::move(error_response));
}

const dbus::ObjectPath&
BluetoothGattCharacteristicServiceProviderImpl::object_path() const {
  return object_path_;
}

}  // namespace bluez
