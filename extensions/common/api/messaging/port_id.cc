// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/common/api/messaging/port_id.h"

#include <tuple>

namespace extensions {

PortId::PortId() {}
PortId::PortId(const base::UnguessableToken& context_id,
               int port_number,
               bool is_opener)
    : context_id(context_id), port_number(port_number), is_opener(is_opener) {}
PortId::~PortId() {}
PortId::PortId(PortId&& other) = default;
PortId::PortId(const PortId& other) = default;
PortId& PortId::operator=(const PortId& other) = default;

bool PortId::operator==(const PortId& other) const {
  return context_id == other.context_id && port_number == other.port_number &&
         is_opener == other.is_opener;
}

bool PortId::operator<(const PortId& other) const {
  return std::tie(context_id, port_number, is_opener) <
         std::tie(other.context_id, other.port_number, other.is_opener);
}

}  // namespace extensions
