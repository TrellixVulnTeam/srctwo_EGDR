// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/resource_coordinator/public/cpp/coordination_unit_struct_traits.h"

namespace mojo {

// static
resource_coordinator::mojom::CoordinationUnitType
EnumTraits<resource_coordinator::mojom::CoordinationUnitType,
           resource_coordinator::CoordinationUnitType>::
    ToMojom(resource_coordinator::CoordinationUnitType type) {
  switch (type) {
    case resource_coordinator::CoordinationUnitType::kWebContents:
      return resource_coordinator::mojom::CoordinationUnitType::kWebContents;
    case resource_coordinator::CoordinationUnitType::kFrame:
      return resource_coordinator::mojom::CoordinationUnitType::kFrame;
    case resource_coordinator::CoordinationUnitType::kNavigation:
      return resource_coordinator::mojom::CoordinationUnitType::kNavigation;
    case resource_coordinator::CoordinationUnitType::kProcess:
      return resource_coordinator::mojom::CoordinationUnitType::kProcess;
    default:
      NOTREACHED() << "Invalid type: " << static_cast<uint8_t>(type);
      // This should not be reached. Just return a random value.
      return resource_coordinator::mojom::CoordinationUnitType::kWebContents;
  }
}

// static
bool EnumTraits<resource_coordinator::mojom::CoordinationUnitType,
                resource_coordinator::CoordinationUnitType>::
    FromMojom(resource_coordinator::mojom::CoordinationUnitType input,
              resource_coordinator::CoordinationUnitType* out) {
  switch (input) {
    case resource_coordinator::mojom::CoordinationUnitType::kWebContents:
      *out = resource_coordinator::CoordinationUnitType::kWebContents;
      break;
    case resource_coordinator::mojom::CoordinationUnitType::kFrame:
      *out = resource_coordinator::CoordinationUnitType::kFrame;
      break;
    case resource_coordinator::mojom::CoordinationUnitType::kNavigation:
      *out = resource_coordinator::CoordinationUnitType::kNavigation;
      break;
    case resource_coordinator::mojom::CoordinationUnitType::kProcess:
      *out = resource_coordinator::CoordinationUnitType::kProcess;
      break;
    default:
      NOTREACHED() << "Invalid type: " << static_cast<uint8_t>(input);
      return false;
  }
  return true;
}

// static
bool StructTraits<resource_coordinator::mojom::CoordinationUnitIDDataView,
                  resource_coordinator::CoordinationUnitID>::
    Read(resource_coordinator::mojom::CoordinationUnitIDDataView input,
         resource_coordinator::CoordinationUnitID* out) {
  out->id = input.id();
  if (!input.ReadType(&out->type))
    return false;
  return true;
}

}  // namespace mojo
