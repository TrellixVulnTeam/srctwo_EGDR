// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/protocol/chromium_port_allocator_factory.h"

#include "base/memory/ptr_util.h"
#include "remoting/protocol/chromium_socket_factory.h"
#include "remoting/protocol/port_allocator.h"
#include "remoting/protocol/transport_context.h"

namespace remoting {
namespace protocol {

ChromiumPortAllocatorFactory::ChromiumPortAllocatorFactory() {}
ChromiumPortAllocatorFactory::~ChromiumPortAllocatorFactory() {}

std::unique_ptr<cricket::PortAllocator>
ChromiumPortAllocatorFactory::CreatePortAllocator(
    scoped_refptr<TransportContext> transport_context) {
  return base::MakeUnique<PortAllocator>(
      base::WrapUnique(new rtc::BasicNetworkManager()),
      base::WrapUnique(new ChromiumPacketSocketFactory()), transport_context);
}

}  // namespace protocol
}  // namespace remoting
