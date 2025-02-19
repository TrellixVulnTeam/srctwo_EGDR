// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// IPC messages for extensions GuestViews.
// Multiply-included message file, hence no include guard.

#include <string>

#include "ipc/ipc_message_macros.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/ipc/gfx_param_traits.h"

#define IPC_MESSAGE_START ExtensionsGuestViewMsgStart
// Messages sent from the browser to the renderer.

// The ACK for GuestViewHostMsg_CreateMimeHandlerViewGuest.
IPC_MESSAGE_CONTROL1(ExtensionsGuestViewMsg_CreateMimeHandlerViewGuestACK,
                     int /* element_instance_id */)

// Once a MimeHandlerView guest's JavaScript onload function has been called,
// this IPC is sent to the container to notify it.
IPC_MESSAGE_CONTROL1(ExtensionsGuestViewMsg_MimeHandlerViewGuestOnLoadCompleted,
                     int /* element_instance_id */)

// Messages sent from the renderer to the browser.

// Queries whether the RenderView of the provided |routing_id| is allowed to
// inject the script with the provided |script_id|.
IPC_SYNC_MESSAGE_CONTROL2_1(
    ExtensionsGuestViewHostMsg_CanExecuteContentScriptSync,
    int /* routing_id */,
    int /* script_id */,
    bool /* allowed */)

// Tells the browser to create a mime handler guest view for a plugin.
IPC_MESSAGE_CONTROL4(ExtensionsGuestViewHostMsg_CreateMimeHandlerViewGuest,
                     int /* render_frame_id */,
                     std::string /* view_id */,
                     int /* element_instance_id */,
                     gfx::Size /* element_size */)

// A renderer sends this message when it wants to resize a guest.
IPC_MESSAGE_CONTROL3(ExtensionsGuestViewHostMsg_ResizeGuest,
                     int /* routing_id */,
                     int /* element_instance_id*/,
                     gfx::Size /* new_size */)
