// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/renderer_host/pepper/pepper_gamepad_host.h"

#include "base/bind.h"
#include "content/public/browser/browser_ppapi_host.h"
#include "device/gamepad/gamepad_service.h"
#include "ppapi/c/pp_errors.h"
#include "ppapi/host/dispatch_host_message.h"
#include "ppapi/host/host_message_context.h"
#include "ppapi/host/ppapi_host.h"
#include "ppapi/proxy/ppapi_messages.h"
#include "ppapi/shared_impl/ppb_gamepad_shared.h"

namespace content {

PepperGamepadHost::PepperGamepadHost(BrowserPpapiHost* host,
                                     PP_Instance instance,
                                     PP_Resource resource)
    : ResourceHost(host->GetPpapiHost(), instance, resource),
      gamepad_service_(device::GamepadService::GetInstance()),
      is_started_(false),
      weak_factory_(this) {}

PepperGamepadHost::PepperGamepadHost(device::GamepadService* gamepad_service,
                                     BrowserPpapiHost* host,
                                     PP_Instance instance,
                                     PP_Resource resource)
    : ResourceHost(host->GetPpapiHost(), instance, resource),
      gamepad_service_(gamepad_service),
      is_started_(false),
      weak_factory_(this) {}

PepperGamepadHost::~PepperGamepadHost() {
  if (is_started_)
    gamepad_service_->RemoveConsumer(this);
}

int32_t PepperGamepadHost::OnResourceMessageReceived(
    const IPC::Message& msg,
    ppapi::host::HostMessageContext* context) {
  PPAPI_BEGIN_MESSAGE_MAP(PepperGamepadHost, msg)
    PPAPI_DISPATCH_HOST_RESOURCE_CALL_0(PpapiHostMsg_Gamepad_RequestMemory,
                                        OnRequestMemory)
  PPAPI_END_MESSAGE_MAP()
  return PP_ERROR_FAILED;
}

int32_t PepperGamepadHost::OnRequestMemory(
    ppapi::host::HostMessageContext* context) {
  if (is_started_)
    return PP_ERROR_FAILED;

  gamepad_service_->ConsumerBecameActive(this);
  is_started_ = true;

  // Don't send the shared memory back until the user has interacted with the
  // gamepad. This is to prevent fingerprinting and matches what the web
  // platform does.
  gamepad_service_->RegisterForUserGesture(
      base::Bind(&PepperGamepadHost::GotUserGesture,
                 weak_factory_.GetWeakPtr(),
                 context->MakeReplyMessageContext()));
  return PP_OK_COMPLETIONPENDING;
}

void PepperGamepadHost::GotUserGesture(
    const ppapi::host::ReplyMessageContext& context) {
  base::SharedMemoryHandle handle =
      gamepad_service_->DuplicateSharedMemoryHandle();

  context.params.AppendHandle(ppapi::proxy::SerializedHandle(
      handle, sizeof(ppapi::ContentGamepadHardwareBuffer)));
  host()->SendReply(context, PpapiPluginMsg_Gamepad_SendMemory());
}

}  // namespace content
