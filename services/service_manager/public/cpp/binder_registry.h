// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_SERVICE_MANAGER_PUBLIC_CPP_BINDER_REGISTRY_H_
#define SERVICES_SERVICE_MANAGER_PUBLIC_CPP_BINDER_REGISTRY_H_

#include <map>
#include <memory>
#include <string>

#include "base/callback.h"
#include "base/memory/ptr_util.h"
#include "base/memory/weak_ptr.h"
#include "mojo/public/cpp/system/message_pipe.h"
#include "services/service_manager/public/cpp/export.h"
#include "services/service_manager/public/cpp/interface_binder.h"

namespace service_manager {

template <typename... BinderArgs>
class BinderRegistryWithArgs {
 public:
  using Binder = base::Callback<
      void(const std::string&, mojo::ScopedMessagePipeHandle, BinderArgs...)>;

  BinderRegistryWithArgs() : weak_factory_(this) {}
  ~BinderRegistryWithArgs() = default;

  // Adds an interface inferring the interface name via the templated
  // parameter Interface::Name_
  // Usage example: //services/service_manager/README.md#OnBindInterface
  template <typename Interface>
  void AddInterface(
      const base::Callback<void(mojo::InterfaceRequest<Interface>,
                                BinderArgs...)>& callback,
      const scoped_refptr<base::SequencedTaskRunner>& task_runner = nullptr) {
    SetInterfaceBinder(
        Interface::Name_,
        base::MakeUnique<CallbackBinder<Interface, BinderArgs...>>(
            callback, task_runner));
  }
  void AddInterface(
      const std::string& interface_name,
      const base::Callback<void(mojo::ScopedMessagePipeHandle, BinderArgs...)>&
          callback,
      const scoped_refptr<base::SequencedTaskRunner>& task_runner = nullptr) {
    SetInterfaceBinder(interface_name,
                       base::MakeUnique<GenericCallbackBinder<BinderArgs...>>(
                           callback, task_runner));
  }
  void AddInterface(
      const std::string& interface_name,
      const Binder& callback,
      const scoped_refptr<base::SequencedTaskRunner>& task_runner = nullptr) {
    SetInterfaceBinder(interface_name,
                       base::MakeUnique<GenericCallbackBinder<BinderArgs...>>(
                           callback, task_runner));
  }

  // Removes the specified interface from the registry. This has no effect on
  // bindings already completed.
  template <typename Interface>
  void RemoveInterface() {
    RemoveInterface(Interface::Name_);
  }
  void RemoveInterface(const std::string& interface_name) {
    auto it = binders_.find(interface_name);
    if (it != binders_.end())
      binders_.erase(it);
  }

  // Returns true if an InterfaceBinder is registered for |interface_name|.
  bool CanBindInterface(const std::string& interface_name) const {
    auto it = binders_.find(interface_name);
    return it != binders_.end();
  }

  // Completes binding the request for |interface_name| on |interface_pipe|, by
  // invoking the corresponding InterfaceBinder.
  void BindInterface(const std::string& interface_name,
                     mojo::ScopedMessagePipeHandle interface_pipe,
                     BinderArgs... args) {
    auto it = binders_.find(interface_name);
    if (it != binders_.end()) {
      it->second->BindInterface(interface_name, std::move(interface_pipe),
                                args...);
    } else {
      LOG(ERROR) << "Failed to locate a binder for interface: "
                 << interface_name;
    }
  }

  // Attempts to bind a request for |interface_name| on |interface_pipe|.
  // If the request can be bound, |interface_pipe| is taken and this function
  // returns true. If the request cannot be bound, |interface_pipe| is
  // unmodified and this function returns false.
  bool TryBindInterface(const std::string& interface_name,
                        mojo::ScopedMessagePipeHandle* interface_pipe,
                        BinderArgs... args) {
    bool can_bind = CanBindInterface(interface_name);
    if (can_bind)
      BindInterface(interface_name, std::move(*interface_pipe), args...);
    return can_bind;
  }

  base::WeakPtr<BinderRegistryWithArgs> GetWeakPtr() {
    return weak_factory_.GetWeakPtr();
  }

 private:
  using InterfaceNameToBinderMap =
      std::map<std::string, std::unique_ptr<InterfaceBinder<BinderArgs...>>>;

  // Adds |binder| to the internal map.
  void SetInterfaceBinder(
      const std::string& interface_name,
      std::unique_ptr<InterfaceBinder<BinderArgs...>> binder) {
    RemoveInterface(interface_name);
    binders_[interface_name] = std::move(binder);
  }

  InterfaceNameToBinderMap binders_;

  base::WeakPtrFactory<BinderRegistryWithArgs> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(BinderRegistryWithArgs);
};

using BinderRegistry = BinderRegistryWithArgs<>;

}  // namespace service_manager

#endif  // SERVICES_SERVICE_MANAGER_PUBLIC_CPP_BINDER_REGISTRY_H_
