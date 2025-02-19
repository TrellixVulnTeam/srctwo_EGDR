// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ppapi/shared_impl/ppb_message_loop_shared.h"

namespace ppapi {

MessageLoopShared::MessageLoopShared(PP_Instance instance)
    : Resource(OBJECT_IS_PROXY, instance) {}

MessageLoopShared::MessageLoopShared(ForMainThread)
    : Resource(Resource::Untracked()) {}

MessageLoopShared::~MessageLoopShared() {}

}  // namespace ppapi
