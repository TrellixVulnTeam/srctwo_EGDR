// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PermissionUtils_h
#define PermissionUtils_h

#include "public/platform/modules/permissions/permission.mojom-blink.h"

namespace blink {

class ExecutionContext;

bool ConnectToPermissionService(ExecutionContext*,
                                mojom::blink::PermissionServiceRequest);

mojom::blink::PermissionDescriptorPtr CreatePermissionDescriptor(
    mojom::blink::PermissionName);

mojom::blink::PermissionDescriptorPtr CreateMidiPermissionDescriptor(
    bool sysex);

}  // namespace blink

#endif  // PermissionUtils_h
