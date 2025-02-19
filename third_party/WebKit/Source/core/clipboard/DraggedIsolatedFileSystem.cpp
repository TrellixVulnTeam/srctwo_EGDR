// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/clipboard/DraggedIsolatedFileSystem.h"

namespace blink {

DraggedIsolatedFileSystem::FileSystemIdPreparationCallback
    DraggedIsolatedFileSystem::prepare_callback_ = nullptr;

void DraggedIsolatedFileSystem::Init(
    DraggedIsolatedFileSystem::FileSystemIdPreparationCallback callback) {
  DCHECK(!prepare_callback_);
  prepare_callback_ = callback;
}

void DraggedIsolatedFileSystem::PrepareForDataObject(DataObject* data_object) {
  DCHECK(prepare_callback_);
  (*prepare_callback_)(data_object);
}

}  // namespace blink
