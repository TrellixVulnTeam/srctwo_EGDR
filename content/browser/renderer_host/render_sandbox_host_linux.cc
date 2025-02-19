// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/renderer_host/render_sandbox_host_linux.h"

#include <sys/socket.h>

#include "base/memory/singleton.h"
#include "base/posix/eintr_wrapper.h"

namespace content {

// Runs on the main thread at startup.
RenderSandboxHostLinux::RenderSandboxHostLinux()
    : initialized_(false), renderer_socket_(0), childs_lifeline_fd_(0) {
}

// static
RenderSandboxHostLinux* RenderSandboxHostLinux::GetInstance() {
  return base::Singleton<RenderSandboxHostLinux>::get();
}

void RenderSandboxHostLinux::Init() {
  DCHECK(!initialized_);
  initialized_ = true;

  int fds[2];
  // We use SOCK_SEQPACKET rather than SOCK_DGRAM to prevent the renderer from
  // sending datagrams to other sockets on the system. The sandbox may prevent
  // the renderer from calling socket() to create new sockets, but it'll still
  // inherit some sockets. With AF_UNIX+SOCK_DGRAM, it can call sendmsg to send
  // a datagram to any (abstract) socket on the same system. With
  // SOCK_SEQPACKET, this is prevented.
  CHECK(socketpair(AF_UNIX, SOCK_SEQPACKET, 0, fds) == 0);

  renderer_socket_ = fds[0];
  // The SandboxIPC client is not expected to read from |renderer_socket_|.
  // Instead, it reads from a temporary socket sent with the request.
  PCHECK(0 == shutdown(renderer_socket_, SHUT_RD)) << "shutdown";

  const int browser_socket = fds[1];
  // The SandboxIPC handler is not expected to write to |browser_socket|.
  // Instead, it replies on a temporary socket provided by the caller.
  PCHECK(0 == shutdown(browser_socket, SHUT_WR)) << "shutdown";

  int pipefds[2];
  CHECK(0 == pipe(pipefds));
  const int child_lifeline_fd = pipefds[0];
  childs_lifeline_fd_ = pipefds[1];

  ipc_handler_.reset(
      new SandboxIPCHandler(child_lifeline_fd, browser_socket));
  ipc_thread_.reset(
      new base::DelegateSimpleThread(ipc_handler_.get(), "sandbox_ipc_thread"));
  ipc_thread_->Start();
}

bool RenderSandboxHostLinux::ShutdownIPCChannel() {
  return IGNORE_EINTR(close(childs_lifeline_fd_)) == 0;
}

RenderSandboxHostLinux::~RenderSandboxHostLinux() {
  if (initialized_) {
    if (!ShutdownIPCChannel())
      LOG(ERROR) << "ShutdownIPCChannel failed";
    if (IGNORE_EINTR(close(renderer_socket_)) < 0)
      PLOG(ERROR) << "close";

    ipc_thread_->Join();
  }
}

}  // namespace content
