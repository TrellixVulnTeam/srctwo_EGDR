// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IPC_IPC_MOJO_HANDLE_ATTACHMENT_H_
#define IPC_IPC_MOJO_HANDLE_ATTACHMENT_H_

#include "base/files/file.h"
#include "base/macros.h"
#include "build/build_config.h"
#include "ipc/ipc_export.h"
#include "ipc/ipc_message_attachment.h"
#include "mojo/public/cpp/system/handle.h"

namespace IPC {

namespace internal {

// A MessageAttachment that holds a MojoHandle.
// This can hold any type of transferrable Mojo handle (i.e. message pipe, data
// pipe, etc), but the receiver is expected to know what type of handle to
// expect.
class IPC_EXPORT MojoHandleAttachment : public MessageAttachment {
 public:
  explicit MojoHandleAttachment(mojo::ScopedHandle handle);

  Type GetType() const override;

  // Returns the owning handle transferring the ownership.
  mojo::ScopedHandle TakeHandle();

 private:
  ~MojoHandleAttachment() override;
  mojo::ScopedHandle handle_;

  DISALLOW_COPY_AND_ASSIGN(MojoHandleAttachment);
};

}  // namespace internal
}  // namespace IPC

#endif  // IPC_IPC_MOJO_HANDLE_ATTACHMENT_H_
