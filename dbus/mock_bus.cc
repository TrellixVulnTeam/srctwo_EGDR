// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "dbus/mock_bus.h"

#include "base/location.h"

namespace dbus {

MockBus::MockBus(const Bus::Options& options) : Bus(options) {
}

MockBus::~MockBus() {
}

}  // namespace dbus
