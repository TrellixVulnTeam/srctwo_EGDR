// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/device/fingerprint/fingerprint.h"
#include "services/device/public/interfaces/fingerprint.mojom.h"

namespace device {

namespace {

// An empty implementation of Fingerprint.
// Used on platforms where a real implementation is not available.
class FingerprintDefault : public Fingerprint {
 public:
  explicit FingerprintDefault() {}
  ~FingerprintDefault() {}
};

}  // namespace

// static
void Fingerprint::Create(device::mojom::FingerprintRequest request) {
  // Do nothing.
}

}  // namespace device
