// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_PUBLIC_CPP_BINDINGS_SCOPED_INTERFACE_ENDPOINT_HANDLE_H_
#define MOJO_PUBLIC_CPP_BINDINGS_SCOPED_INTERFACE_ENDPOINT_HANDLE_H_

#include <string>

#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/optional.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/thread_task_runner_handle.h"
#include "mojo/public/cpp/bindings/bindings_export.h"
#include "mojo/public/cpp/bindings/disconnect_reason.h"
#include "mojo/public/cpp/bindings/interface_id.h"

namespace mojo {

class AssociatedGroupController;

// ScopedInterfaceEndpointHandle refers to one end of an interface, either the
// implementation side or the client side.
// Threading: At any given time, a ScopedInterfaceEndpointHandle should only
// be accessed from a single sequence.
class MOJO_CPP_BINDINGS_EXPORT ScopedInterfaceEndpointHandle {
 public:
  // Creates a pair of handles representing the two endpoints of an interface,
  // which are not yet associated with a message pipe.
  static void CreatePairPendingAssociation(
      ScopedInterfaceEndpointHandle* handle0,
      ScopedInterfaceEndpointHandle* handle1);

  // Creates an invalid endpoint handle.
  ScopedInterfaceEndpointHandle();

  ScopedInterfaceEndpointHandle(ScopedInterfaceEndpointHandle&& other);

  ~ScopedInterfaceEndpointHandle();

  ScopedInterfaceEndpointHandle& operator=(
      ScopedInterfaceEndpointHandle&& other);

  bool is_valid() const;

  // Returns true if the interface hasn't associated with a message pipe.
  bool pending_association() const;

  // Returns kInvalidInterfaceId when in pending association state or the handle
  // is invalid.
  InterfaceId id() const;

  // Returns null when in pending association state or the handle is invalid.
  AssociatedGroupController* group_controller() const;

  // Returns the disconnect reason if the peer handle is closed before
  // association and specifies a custom disconnect reason.
  const base::Optional<DisconnectReason>& disconnect_reason() const;

  enum AssociationEvent {
    // The interface has been associated with a message pipe.
    ASSOCIATED,
    // The peer of this object has been closed before association.
    PEER_CLOSED_BEFORE_ASSOCIATION
  };

  using AssociationEventCallback = base::OnceCallback<void(AssociationEvent)>;
  // Note:
  // - |handler| won't run if the handle is invalid. Otherwise, |handler| is run
  //   on the calling sequence asynchronously, even if the interface has already
  //   been associated or the peer has been closed before association.
  // - |handler| won't be called after this object is destroyed or reset.
  // - A null |handler| can be used to cancel the previous callback.
  void SetAssociationEventHandler(AssociationEventCallback handler);

  void reset();
  void ResetWithReason(uint32_t custom_reason, const std::string& description);

 private:
  friend class AssociatedGroupController;
  friend class AssociatedGroup;

  class State;

  // Used by AssociatedGroupController.
  ScopedInterfaceEndpointHandle(
      InterfaceId id,
      scoped_refptr<AssociatedGroupController> group_controller);

  // Used by AssociatedGroupController.
  // The peer of this handle will join |peer_group_controller|.
  bool NotifyAssociation(
      InterfaceId id,
      scoped_refptr<AssociatedGroupController> peer_group_controller);

  void ResetInternal(const base::Optional<DisconnectReason>& reason);

  // Used by AssociatedGroup.
  // It is safe to run the returned callback on any sequence, or after this
  // handle is destroyed.
  // The return value of the getter:
  //   - If the getter is retrieved when the handle is invalid, the return value
  //     of the getter will always be null.
  //   - If the getter is retrieved when the handle is valid and non-pending,
  //     the return value of the getter will be non-null and remain unchanged
  //     even if the handle is later reset.
  //   - If the getter is retrieved when the handle is valid but pending
  //     asssociation, the return value of the getter will initially be null,
  //     change to non-null when the handle is associated, and remain unchanged
  //     ever since.
  base::Callback<AssociatedGroupController*()> CreateGroupControllerGetter()
      const;

  scoped_refptr<State> state_;

  DISALLOW_COPY_AND_ASSIGN(ScopedInterfaceEndpointHandle);
};

}  // namespace mojo

#endif  // MOJO_PUBLIC_CPP_BINDINGS_SCOPED_INTERFACE_ENDPOINT_HANDLE_H_
