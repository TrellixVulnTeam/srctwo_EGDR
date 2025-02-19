// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/indexed_db/mock_mojo_indexed_db_callbacks.h"

namespace content {

MockMojoIndexedDBCallbacks::MockMojoIndexedDBCallbacks() : binding_(this) {}
MockMojoIndexedDBCallbacks::~MockMojoIndexedDBCallbacks() {}

::indexed_db::mojom::CallbacksAssociatedPtrInfo
MockMojoIndexedDBCallbacks::CreateInterfacePtrAndBind() {
  ::indexed_db::mojom::CallbacksAssociatedPtrInfo ptr_info;
  binding_.Bind(::mojo::MakeRequest(&ptr_info));
  return ptr_info;
}

}  // namespace content
