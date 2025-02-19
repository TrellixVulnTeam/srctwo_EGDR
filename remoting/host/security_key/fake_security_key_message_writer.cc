// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/host/security_key/fake_security_key_message_writer.h"

#include <string>
#include <utility>

#include "base/callback.h"
#include "base/location.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/threading/thread_task_runner_handle.h"
#include "remoting/host/security_key/security_key_message.h"

namespace remoting {

FakeSecurityKeyMessageWriter::FakeSecurityKeyMessageWriter(
    const base::Closure& write_callback)
    : write_callback_(write_callback), weak_factory_(this) {
  DCHECK(!write_callback_.is_null());
}

FakeSecurityKeyMessageWriter::~FakeSecurityKeyMessageWriter() {}

base::WeakPtr<FakeSecurityKeyMessageWriter>
FakeSecurityKeyMessageWriter::AsWeakPtr() {
  return weak_factory_.GetWeakPtr();
}

bool FakeSecurityKeyMessageWriter::WriteMessage(
    SecurityKeyMessageType message_type) {
  last_message_type_ = message_type;
  last_message_payload_.clear();

  base::ThreadTaskRunnerHandle::Get()->PostTask(FROM_HERE, write_callback_);

  return write_request_succeeded_;
}

bool FakeSecurityKeyMessageWriter::WriteMessageWithPayload(
    SecurityKeyMessageType message_type,
    const std::string& message_payload) {
  last_message_type_ = message_type;
  last_message_payload_ = message_payload;

  base::ThreadTaskRunnerHandle::Get()->PostTask(FROM_HERE, write_callback_);

  return write_request_succeeded_;
}

}  // namespace remoting
