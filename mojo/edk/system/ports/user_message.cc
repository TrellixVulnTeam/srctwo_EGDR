// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/edk/system/ports/user_message.h"

namespace mojo {
namespace edk {
namespace ports {

UserMessage::UserMessage(const TypeInfo* type_info) : type_info_(type_info) {}

UserMessage::~UserMessage() = default;

bool UserMessage::WillBeRoutedExternally() {
  return true;
}

}  // namespace ports
}  // namespace edk
}  // namespace mojo
