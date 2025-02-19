// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/edk/system/ports/port_ref.h"

#include "mojo/edk/system/ports/port.h"

namespace mojo {
namespace edk {
namespace ports {

PortRef::~PortRef() {
}

PortRef::PortRef() {
}

PortRef::PortRef(const PortName& name, scoped_refptr<Port> port)
    : name_(name), port_(std::move(port)) {}

PortRef::PortRef(const PortRef& other) = default;

PortRef::PortRef(PortRef&& other) = default;

PortRef& PortRef::operator=(const PortRef& other) = default;

PortRef& PortRef::operator=(PortRef&& other) = default;

}  // namespace ports
}  // namespace edk
}  // namespace mojo
