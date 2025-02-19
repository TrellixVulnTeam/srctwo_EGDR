// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/host/test_echo_extension.h"

#include "base/memory/ptr_util.h"
#include "remoting/host/test_echo_extension_session.h"

namespace {
constexpr char kCapability[] = "";
}

namespace remoting {

TestEchoExtension::TestEchoExtension() = default;

TestEchoExtension::~TestEchoExtension() = default;

std::string TestEchoExtension::capability() const {
  return kCapability;
}

std::unique_ptr<HostExtensionSession> TestEchoExtension::CreateExtensionSession(
    ClientSessionDetails* details,
    protocol::ClientStub* client_stub) {
  return base::MakeUnique<TestEchoExtensionSession>();
}

}  // namespace remoting
