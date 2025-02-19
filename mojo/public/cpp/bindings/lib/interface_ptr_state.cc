// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/public/cpp/bindings/lib/interface_ptr_state.h"

#include "mojo/public/cpp/bindings/lib/task_runner_helper.h"

namespace mojo {
namespace internal {

InterfacePtrStateBase::InterfacePtrStateBase() = default;

InterfacePtrStateBase::~InterfacePtrStateBase() {
  endpoint_client_.reset();
  if (router_)
    router_->CloseMessagePipe();
}

void InterfacePtrStateBase::QueryVersion(
    const base::Callback<void(uint32_t)>& callback) {
  // It is safe to capture |this| because the callback won't be run after this
  // object goes away.
  endpoint_client_->QueryVersion(
      base::Bind(&InterfacePtrStateBase::OnQueryVersion, base::Unretained(this),
                 callback));
}

void InterfacePtrStateBase::RequireVersion(uint32_t version) {
  if (version <= version_)
    return;

  version_ = version;
  endpoint_client_->RequireVersion(version);
}

void InterfacePtrStateBase::Swap(InterfacePtrStateBase* other) {
  using std::swap;
  swap(other->router_, router_);
  swap(other->endpoint_client_, endpoint_client_);
  handle_.swap(other->handle_);
  runner_.swap(other->runner_);
  swap(other->version_, version_);
}

void InterfacePtrStateBase::Bind(
    ScopedMessagePipeHandle handle,
    uint32_t version,
    scoped_refptr<base::SingleThreadTaskRunner> task_runner) {
  DCHECK(!router_);
  DCHECK(!endpoint_client_);
  DCHECK(!handle_.is_valid());
  DCHECK_EQ(0u, version_);
  DCHECK(handle.is_valid());

  handle_ = std::move(handle);
  version_ = version;
  runner_ =
      GetTaskRunnerToUseFromUserProvidedTaskRunner(std::move(task_runner));
}

void InterfacePtrStateBase::OnQueryVersion(
    const base::Callback<void(uint32_t)>& callback,
    uint32_t version) {
  version_ = version;
  callback.Run(version);
}

bool InterfacePtrStateBase::InitializeEndpointClient(
    bool passes_associated_kinds,
    bool has_sync_methods,
    std::unique_ptr<MessageReceiver> payload_validator) {
  // The object hasn't been bound.
  if (!handle_.is_valid())
    return false;

  MultiplexRouter::Config config =
      passes_associated_kinds
          ? MultiplexRouter::MULTI_INTERFACE
          : (has_sync_methods
                 ? MultiplexRouter::SINGLE_INTERFACE_WITH_SYNC_METHODS
                 : MultiplexRouter::SINGLE_INTERFACE);
  router_ = new MultiplexRouter(std::move(handle_), config, true, runner_);
  endpoint_client_.reset(new InterfaceEndpointClient(
      router_->CreateLocalEndpointHandle(kMasterInterfaceId), nullptr,
      std::move(payload_validator), false, std::move(runner_),
      // The version is only queried from the client so the value passed here
      // will not be used.
      0u));
  return true;
}

}  // namespace internal
}  // namespace mojo
