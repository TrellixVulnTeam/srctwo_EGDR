// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ipc/ipc_security_test_util.h"

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/run_loop.h"
#include "ipc/ipc_channel_proxy.h"

namespace IPC {

void IpcSecurityTestUtil::PwnMessageReceived(ChannelProxy* channel,
                                             const IPC::Message& message) {
  base::RunLoop run_loop;
  base::Closure inject_message = base::Bind(
      base::IgnoreResult(&IPC::ChannelProxy::Context::OnMessageReceived),
      channel->context(), message);
  channel->context()->ipc_task_runner()->PostTaskAndReply(
      FROM_HERE, inject_message, run_loop.QuitClosure());
  run_loop.Run();
}

}  // namespace IPC
