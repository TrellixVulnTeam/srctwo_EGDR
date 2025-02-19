// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "build/build_config.h"
#include "ipc/ipc_channel.h"
#include "ipc/ipc_channel_mojo.h"
#include "mojo/public/cpp/system/message_pipe.h"

namespace IPC {

#if defined(OS_LINUX)

namespace {
int g_global_pid = 0;
}

// static
void Channel::SetGlobalPid(int pid) {
  g_global_pid = pid;
}

// static
int Channel::GetGlobalPid() {
  return g_global_pid;
}

#endif  // defined(OS_LINUX)

// static
std::unique_ptr<Channel> Channel::CreateClient(
    const IPC::ChannelHandle& channel_handle,
    Listener* listener,
    const scoped_refptr<base::SingleThreadTaskRunner>& ipc_task_runner) {
#if defined(OS_NACL_SFI)
  return Channel::Create(channel_handle, Channel::MODE_CLIENT, listener);
#else
  DCHECK(channel_handle.is_mojo_channel_handle());
  return ChannelMojo::Create(
      mojo::ScopedMessagePipeHandle(channel_handle.mojo_handle),
      Channel::MODE_CLIENT, listener, ipc_task_runner);
#endif
}

// static
std::unique_ptr<Channel> Channel::CreateServer(
    const IPC::ChannelHandle& channel_handle,
    Listener* listener,
    const scoped_refptr<base::SingleThreadTaskRunner>& ipc_task_runner) {
#if defined(OS_NACL_SFI)
  return Channel::Create(channel_handle, Channel::MODE_SERVER, listener);
#else
  DCHECK(channel_handle.is_mojo_channel_handle());
  return ChannelMojo::Create(
      mojo::ScopedMessagePipeHandle(channel_handle.mojo_handle),
      Channel::MODE_SERVER, listener, ipc_task_runner);
#endif
}

Channel::~Channel() {
}

Channel::AssociatedInterfaceSupport* Channel::GetAssociatedInterfaceSupport() {
  return nullptr;
}

void Channel::Pause() { NOTREACHED(); }

void Channel::Unpause(bool flush) { NOTREACHED(); }

void Channel::Flush() { NOTREACHED(); }

void Channel::WillConnect() {
  did_start_connect_ = true;
}

}  // namespace IPC
