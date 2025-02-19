// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "services/preferences/tracked/device_id.h"

MachineIdStatus GetDeterministicMachineSpecificId(std::string* machine_id) {
  DCHECK(machine_id);
  return MachineIdStatus::NOT_IMPLEMENTED;
}
