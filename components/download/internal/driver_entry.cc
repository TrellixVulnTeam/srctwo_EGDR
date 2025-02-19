// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/download/internal/driver_entry.h"

#include "net/http/http_response_headers.h"

namespace download {

DriverEntry::DriverEntry()
    : state(State::UNKNOWN),
      paused(false),
      bytes_downloaded(0u),
      expected_total_size(0u) {}

DriverEntry::DriverEntry(const DriverEntry& other) = default;

DriverEntry::~DriverEntry() = default;

}  // namespace download
