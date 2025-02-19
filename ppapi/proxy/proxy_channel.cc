// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ppapi/proxy/proxy_channel.h"

#include "base/logging.h"
#include "build/build_config.h"
#include "ipc/ipc_platform_file.h"
#include "ipc/ipc_test_sink.h"

#if defined(OS_NACL)
#include <unistd.h>
#endif

namespace ppapi {
namespace proxy {

ProxyChannel::ProxyChannel()
    : delegate_(NULL),
      peer_pid_(base::kNullProcessId),
      test_sink_(NULL) {
}

ProxyChannel::~ProxyChannel() {
  DVLOG(1) << "ProxyChannel::~ProxyChannel()";
}

bool ProxyChannel::InitWithChannel(Delegate* delegate,
                                   base::ProcessId peer_pid,
                                   const IPC::ChannelHandle& channel_handle,
                                   bool is_client) {
  delegate_ = delegate;
  peer_pid_ = peer_pid;
  IPC::Channel::Mode mode = is_client
      ? IPC::Channel::MODE_CLIENT
      : IPC::Channel::MODE_SERVER;
  channel_ = IPC::SyncChannel::Create(channel_handle, mode, this,
                                      delegate->GetIPCTaskRunner(), true,
                                      delegate->GetShutdownEvent());
  return true;
}

void ProxyChannel::InitWithTestSink(IPC::TestSink* test_sink) {
  DCHECK(!test_sink_);
  test_sink_ = test_sink;
#if !defined(OS_NACL)
  peer_pid_ = base::GetCurrentProcId();
#endif
}

void ProxyChannel::OnChannelError() {
  channel_.reset();
}

IPC::PlatformFileForTransit ProxyChannel::ShareHandleWithRemote(
      base::PlatformFile handle,
      bool should_close_source) {
  // Channel could be closed if the plugin crashes.
  if (!channel_.get()) {
    if (should_close_source) {
      base::File file_closer(handle);
    }
    return IPC::InvalidPlatformFileForTransit();
  }
  DCHECK(peer_pid_ != base::kNullProcessId);
  return delegate_->ShareHandleWithRemote(handle, peer_pid_,
                                          should_close_source);
}

base::SharedMemoryHandle ProxyChannel::ShareSharedMemoryHandleWithRemote(
    const base::SharedMemoryHandle& handle) {
  if (!channel_.get())
    return base::SharedMemoryHandle();

  DCHECK(peer_pid_ != base::kNullProcessId);
  return delegate_->ShareSharedMemoryHandleWithRemote(handle, peer_pid_);
}

bool ProxyChannel::Send(IPC::Message* msg) {
  if (test_sink_)
    return test_sink_->Send(msg);
  if (channel_.get())
    return channel_->Send(msg);

  // Remote side crashed, drop this message.
  delete msg;
  return false;
}

}  // namespace proxy
}  // namespace ppapi
