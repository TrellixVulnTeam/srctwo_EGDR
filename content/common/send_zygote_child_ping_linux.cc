// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/common/send_zygote_child_ping_linux.h"

#include <vector>

#include "base/posix/unix_domain_socket.h"
#include "content/common/zygote_commands_linux.h"

namespace content {

bool SendZygoteChildPing(int fd) {
  return base::UnixDomainSocket::SendMsg(fd,
                                         kZygoteChildPingMessage,
                                         sizeof(kZygoteChildPingMessage),
                                         std::vector<int>());
}

}  // namespace content
