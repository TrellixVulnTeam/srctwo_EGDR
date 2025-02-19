// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/arc/bluetooth/bluetooth_struct_traits.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "base/memory/ptr_util.h"
#include "base/stl_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "device/bluetooth/bluetooth_advertisement.h"
#include "device/bluetooth/bluetooth_uuid.h"

namespace {

// BluetoothUUID helpers.
constexpr size_t kUUIDSize = 16;

bool IsNonHex(char c) {
  return !isxdigit(c);
}

std::string StripNonHex(const std::string& str) {
  std::string result = str;
  base::EraseIf(result, IsNonHex);
  return result;
}

// BluetoothAdvertisement helpers.
struct AdvertisementEntry {
  virtual ~AdvertisementEntry() {}
  virtual void AddTo(device::BluetoothAdvertisement::Data* data) {}
};

struct ServiceUUIDEntry : public AdvertisementEntry {
  std::vector<device::BluetoothUUID> service_uuids;

  ~ServiceUUIDEntry() override {}

  void AddTo(device::BluetoothAdvertisement::Data* data) override {
    auto string_uuids = base::MakeUnique<std::vector<std::string>>();
    for (const auto& uuid : service_uuids) {
      string_uuids->emplace_back(uuid.value());
    }
    data->set_service_uuids(std::move(string_uuids));
  }
};

struct ServiceDataEntry : public AdvertisementEntry {
  uint16_t service_uuid;
  std::vector<uint8_t> service_data;

  ~ServiceDataEntry() override {}

  void AddTo(device::BluetoothAdvertisement::Data* data) override {
    std::string string_uuid = base::StringPrintf("%04x", service_uuid);
    data->set_service_data(
        base::WrapUnique(new std::map<std::string, std::vector<uint8_t>>{
            {string_uuid, service_data}}));
  }
};

struct ManufacturerDataEntry : public AdvertisementEntry {
  uint16_t company_id_code;
  std::vector<uint8_t> blob;

  ~ManufacturerDataEntry() override {}

  void AddTo(device::BluetoothAdvertisement::Data* data) override {
    data->set_manufacturer_data(base::WrapUnique(
        new std::map<uint16_t, std::vector<uint8_t>>{{company_id_code, blob}}));
  }
};

uint16_t ExtractCompanyIdentifierCode(std::vector<uint8_t>* blob) {
  // The company identifier code is in little-endian.
  uint16_t company_id_code = (*blob)[1] << 8 | (*blob)[0];
  blob->erase(blob->begin(), blob->begin() + sizeof(uint16_t));
  return company_id_code;
}

}  // namespace

namespace mojo {

// static
std::vector<uint8_t>
StructTraits<arc::mojom::BluetoothUUIDDataView, device::BluetoothUUID>::uuid(
    const device::BluetoothUUID& input) {
  // TODO(dcheng): Figure out what to do here, this is called twice on
  // serialization. Building a vector is a little inefficient.
  std::string uuid_str = StripNonHex(input.canonical_value());

  std::vector<uint8_t> address_bytes;
  base::HexStringToBytes(uuid_str, &address_bytes);
  return address_bytes;
}

// static
bool StructTraits<arc::mojom::BluetoothUUIDDataView,
                  device::BluetoothUUID>::Read(
    arc::mojom::BluetoothUUIDDataView data,
    device::BluetoothUUID* output) {
  std::vector<uint8_t> address_bytes;
  if (!data.ReadUuid(&address_bytes))
    return false;

  if (address_bytes.size() != kUUIDSize)
    return false;

  // BluetoothUUID expects the format below with the dashes inserted.
  // xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
  std::string uuid_str =
      base::HexEncode(address_bytes.data(), address_bytes.size());
  constexpr size_t kUuidDashPos[] = {8, 13, 18, 23};
  for (auto pos : kUuidDashPos)
    uuid_str = uuid_str.insert(pos, "-");

  device::BluetoothUUID result(uuid_str);

  DCHECK(result.IsValid());
  *output = result;
  return true;
}

template <>
struct EnumTraits<arc::mojom::BluetoothAdvertisementType,
                  device::BluetoothAdvertisement::AdvertisementType> {
  static bool FromMojom(
      arc::mojom::BluetoothAdvertisementType mojom_type,
      device::BluetoothAdvertisement::AdvertisementType* type) {
    switch (mojom_type) {
      case arc::mojom::BluetoothAdvertisementType::ADV_TYPE_CONNECTABLE:
      case arc::mojom::BluetoothAdvertisementType::ADV_TYPE_SCANNABLE:
        *type = device::BluetoothAdvertisement::ADVERTISEMENT_TYPE_PERIPHERAL;
        return true;
      case arc::mojom::BluetoothAdvertisementType::ADV_TYPE_NON_CONNECTABLE:
        *type = device::BluetoothAdvertisement::ADVERTISEMENT_TYPE_BROADCAST;
        return true;
    }
    NOTREACHED() << "Invalid type: " << static_cast<uint32_t>(mojom_type);
    return false;
  }
};

template <>
struct StructTraits<arc::mojom::BluetoothServiceDataDataView,
                    ServiceDataEntry> {
  static bool Read(arc::mojom::BluetoothServiceDataDataView data,
                   ServiceDataEntry* output) {
    output->service_uuid = data.uuid_16bit();
    return data.ReadData(&output->service_data);
  }
};

template <>
struct UnionTraits<arc::mojom::BluetoothAdvertisingDataDataView,
                   std::unique_ptr<AdvertisementEntry>> {
  static bool Read(arc::mojom::BluetoothAdvertisingDataDataView data,
                   std::unique_ptr<AdvertisementEntry>* output) {
    switch (data.tag()) {
      case arc::mojom::BluetoothAdvertisingDataDataView::Tag::SERVICE_UUIDS: {
        std::unique_ptr<ServiceUUIDEntry> service_uuids =
            base::MakeUnique<ServiceUUIDEntry>();
        if (!data.ReadServiceUuids(&service_uuids->service_uuids))
          return false;
        *output = std::move(service_uuids);
        break;
      }
      case arc::mojom::BluetoothAdvertisingDataDataView::Tag::SERVICE_DATA: {
        std::unique_ptr<ServiceDataEntry> service_data =
            base::MakeUnique<ServiceDataEntry>();
        if (!data.ReadServiceData(service_data.get()))
          return false;
        *output = std::move(service_data);
        break;
      }
      case arc::mojom::BluetoothAdvertisingDataDataView::Tag::
          MANUFACTURER_DATA: {
        std::unique_ptr<ManufacturerDataEntry> manufacturer_data =
            base::MakeUnique<ManufacturerDataEntry>();
        // We get manufacturer data as a big blob. The first two bytes
        // should be a company identifier code and the rest is manufacturer-
        // specific.
        std::vector<uint8_t> blob;
        if (!data.ReadManufacturerData(&blob))
          return false;
        if (blob.size() < sizeof(uint16_t)) {
          LOG(WARNING) << "Advertisement had malformed manufacturer data";
          return false;
        }

        manufacturer_data->company_id_code =
            ExtractCompanyIdentifierCode(&blob);
        manufacturer_data->blob = std::move(blob);
        *output = std::move(manufacturer_data);
        break;
      }
      default: {
        LOG(WARNING) << "Bluetooth advertising data case not implemented";
        // Default AdvertisementEntry does nothing when added to the
        // device::BluetoothAdvertisement::AdvertisementData, so data we
        // don't know how to handle yet will be dropped but won't cause a
        // failure to deserialize.
        *output = base::MakeUnique<AdvertisementEntry>();
        break;
      }
    }
    return true;
  }
};

// static
bool StructTraits<arc::mojom::BluetoothAdvertisementDataView,
                  std::unique_ptr<device::BluetoothAdvertisement::Data>>::
    Read(arc::mojom::BluetoothAdvertisementDataView advertisement,
         std::unique_ptr<device::BluetoothAdvertisement::Data>* output) {
  device::BluetoothAdvertisement::AdvertisementType adv_type;
  if (!advertisement.ReadType(&adv_type))
    return false;
  auto data = base::MakeUnique<device::BluetoothAdvertisement::Data>(adv_type);

  std::vector<std::unique_ptr<AdvertisementEntry>> adv_entries;
  if (!advertisement.ReadData(&adv_entries))
    return false;
  for (const auto& adv_entry : adv_entries)
    adv_entry->AddTo(data.get());

  data->set_include_tx_power(advertisement.include_tx_power());

  *output = std::move(data);
  return true;
}

}  // namespace mojo
