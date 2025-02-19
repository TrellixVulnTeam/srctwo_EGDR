// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_EDK_SYSTEM_PORTS_USER_MESSAGE_H_
#define MOJO_EDK_SYSTEM_PORTS_USER_MESSAGE_H_

#include "base/macros.h"

namespace mojo {
namespace edk {
namespace ports {

// Base type to use for any embedder-defined user message implementation. This
// class is intentionally empty.
//
// Provides a bit of type-safety help to subclasses since by design downcasting
// from this type is a common operation in embedders.
//
// Each subclass should define a static const instance of TypeInfo named
// |kUserMessageTypeInfo| and pass its address down to the UserMessage
// constructor. The type of a UserMessage can then be dynamically inspected by
// comparing |type_info()| to any subclass's |&kUserMessageTypeInfo|.
class UserMessage {
 public:
  struct TypeInfo {};

  explicit UserMessage(const TypeInfo* type_info);
  virtual ~UserMessage();

  const TypeInfo* type_info() const { return type_info_; }

  // Invoked immediately before the system asks the embedder to forward this
  // message to an external node.
  //
  // Returns |true| if the message is OK to route externally, or |false|
  // otherwise. Returning |false| implies an unrecoverable condition, and the
  // message event will be destroyed without further routing.
  virtual bool WillBeRoutedExternally();

 private:
  const TypeInfo* const type_info_;

  DISALLOW_COPY_AND_ASSIGN(UserMessage);
};

}  // namespace ports
}  // namespace edk
}  // namespace mojo

#endif  // MOJO_EDK_SYSTEM_PORTS_USER_MESSAGE_H_
