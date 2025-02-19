// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_BLUETOOTH_BLUETOOTH_ADAPTER_MAC_METRICS_H_
#define DEVICE_BLUETOOTH_BLUETOOTH_ADAPTER_MAC_METRICS_H_

@class NSError;

enum class MacOSBluetoothOperationsResult : int {
  UNKNOWN_ERROR_DOMAIN = -2,
  NO_ERROR = -1,
  CBATT_ERROR_SUCCESS = 0,
  CBATT_ERROR_INVALID_HANDLE = 1,
  CBATT_ERROR_READ_NOT_PERMITTED = 2,
  CBATT_ERROR_WRITE_NOT_PERMITTED = 3,
  CBATT_ERROR_INVALID_PDU = 4,
  CBATT_ERROR_INSUFFICIENT_AUTHENTICATION = 5,
  CBATT_ERROR_REQUEST_NOT_SUPPORTED = 6,
  CBATT_ERROR_INVALID_OFFSET = 7,
  CBATT_ERROR_INSUFFICIENT_AUTHORIZATION = 8,
  CBATT_ERROR_PREPARE_QUEUE_FULL = 9,
  CBATT_ERROR_ATTRIBUTE_NOT_FOUND = 10,
  CBATT_ERROR_ATTRIBUTE_NOT_LONG = 11,
  CBATT_ERROR_INSUFFICIENT_ENCRYPTION_KEY_SIZE = 12,
  CBATT_ERROR_INVALID_ATTRIBUTE_VALUE_LENGTH = 13,
  CBATT_ERROR_UNLIKELY_ERROR = 14,
  CBATT_ERROR_INSUFFICIENT_ENCRYPTION = 15,
  CBATT_ERROR_UNSUPPORTED_GROUP_TYPE = 16,
  CBATT_ERROR_INSUFFICIENT_RESOURCES = 17,
  CBATT_ERROR_UNKNOWN_ERROR_CODE = 999,
  CBERROR_UNKNOWN = 1000,
  CBERROR_INVALID_PARAMETERS = 1001,
  CBERROR_INVALID_HANDLE = 1002,
  CBERROR_NOT_CONNECTED = 1003,
  CBERROR_OUT_OF_SPACE = 1004,
  CBERROR_OPERATION_CANCELLED = 1005,
  CBERROR_CONNECTION_TIMEOUT = 1006,
  CBERROR_PERIPHERAL_DISCONNECTED = 1007,
  CBERROR_UUID_NOT_ALLOWED = 1008,
  CBERROR_ALREADY_ADVERTISING = 1009,
  CBERROR_MAX_CONNECTION = 1010,
  CBERROR_UNKNOWN_ERROR_CODE = 1999,
  MAX,
};

void RecordDidFailToConnectPeripheralResult(NSError* error);
void RecordDidDisconnectPeripheralResult(NSError* error);
void RecordDidDiscoverPrimaryServicesResult(NSError* error);
void RecordDidDiscoverCharacteristicsResult(NSError* error);
void RecordDidUpdateValueResult(NSError* error);
void RecordDidWriteValueResult(NSError* error);
void RecordDidUpdateNotificationStateResult(NSError* error);
void RecordDidDiscoverDescriptorsResult(NSError* error);
void RecordDidUpdateValueForDescriptorResult(NSError* error);
void RecordDidWriteValueForDescriptorResult(NSError* error);

#endif  // DEVICE_BLUETOOTH_BLUETOOTH_ADAPTER_MAC_METRICS_H_
