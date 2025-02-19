// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/pepper/pepper_audio_input_host.h"

#include "base/logging.h"
#include "build/build_config.h"
#include "content/common/pepper_file_util.h"
#include "content/renderer/pepper/pepper_media_device_manager.h"
#include "content/renderer/pepper/pepper_platform_audio_input.h"
#include "content/renderer/pepper/pepper_plugin_instance_impl.h"
#include "content/renderer/pepper/renderer_ppapi_host_impl.h"
#include "content/renderer/render_frame_impl.h"
#include "ipc/ipc_message.h"
#include "ppapi/c/pp_errors.h"
#include "ppapi/host/dispatch_host_message.h"
#include "ppapi/host/ppapi_host.h"
#include "ppapi/proxy/ppapi_messages.h"
#include "ppapi/proxy/serialized_structs.h"

namespace content {

namespace {

base::PlatformFile ConvertSyncSocketHandle(const base::SyncSocket& socket) {
  return socket.handle();
}

}  // namespace

PepperAudioInputHost::PepperAudioInputHost(RendererPpapiHostImpl* host,
                                           PP_Instance instance,
                                           PP_Resource resource)
    : ResourceHost(host->GetPpapiHost(), instance, resource),
      renderer_ppapi_host_(host),
      audio_input_(NULL),
      enumeration_helper_(this,
                          PepperMediaDeviceManager::GetForRenderFrame(
                              host->GetRenderFrameForInstance(pp_instance())),
                          PP_DEVICETYPE_DEV_AUDIOCAPTURE,
                          host->GetDocumentURL(instance)) {}

PepperAudioInputHost::~PepperAudioInputHost() { Close(); }

int32_t PepperAudioInputHost::OnResourceMessageReceived(
    const IPC::Message& msg,
    ppapi::host::HostMessageContext* context) {
  int32_t result = PP_ERROR_FAILED;
  if (enumeration_helper_.HandleResourceMessage(msg, context, &result))
    return result;

  PPAPI_BEGIN_MESSAGE_MAP(PepperAudioInputHost, msg)
    PPAPI_DISPATCH_HOST_RESOURCE_CALL(PpapiHostMsg_AudioInput_Open, OnOpen)
    PPAPI_DISPATCH_HOST_RESOURCE_CALL(PpapiHostMsg_AudioInput_StartOrStop,
                                      OnStartOrStop)
    PPAPI_DISPATCH_HOST_RESOURCE_CALL_0(PpapiHostMsg_AudioInput_Close, OnClose)
  PPAPI_END_MESSAGE_MAP()
  return PP_ERROR_FAILED;
}

void PepperAudioInputHost::StreamCreated(
    base::SharedMemoryHandle shared_memory_handle,
    size_t shared_memory_size,
    base::SyncSocket::Handle socket) {
  OnOpenComplete(PP_OK, shared_memory_handle, shared_memory_size, socket);
}

void PepperAudioInputHost::StreamCreationFailed() {
  OnOpenComplete(PP_ERROR_FAILED, base::SharedMemoryHandle(), 0,
                 base::SyncSocket::kInvalidHandle);
}

int32_t PepperAudioInputHost::OnOpen(ppapi::host::HostMessageContext* context,
                                     const std::string& device_id,
                                     PP_AudioSampleRate sample_rate,
                                     uint32_t sample_frame_count) {
  if (open_context_.is_valid())
    return PP_ERROR_INPROGRESS;
  if (audio_input_)
    return PP_ERROR_FAILED;

  GURL document_url = renderer_ppapi_host_->GetDocumentURL(pp_instance());
  if (!document_url.is_valid())
    return PP_ERROR_FAILED;

  // When it is done, we'll get called back on StreamCreated() or
  // StreamCreationFailed().
  audio_input_ = PepperPlatformAudioInput::Create(
      renderer_ppapi_host_->GetRenderFrameForInstance(pp_instance())->
          GetRoutingID(),
      device_id,
      static_cast<int>(sample_rate),
      static_cast<int>(sample_frame_count),
      this);
  if (audio_input_) {
    open_context_ = context->MakeReplyMessageContext();
    return PP_OK_COMPLETIONPENDING;
  } else {
    return PP_ERROR_FAILED;
  }
}

int32_t PepperAudioInputHost::OnStartOrStop(
    ppapi::host::HostMessageContext* /* context */,
    bool capture) {
  if (!audio_input_)
    return PP_ERROR_FAILED;
  if (capture)
    audio_input_->StartCapture();
  else
    audio_input_->StopCapture();
  return PP_OK;
}

int32_t PepperAudioInputHost::OnClose(
    ppapi::host::HostMessageContext* /* context */) {
  Close();
  return PP_OK;
}

void PepperAudioInputHost::OnOpenComplete(
    int32_t result,
    base::SharedMemoryHandle shared_memory_handle,
    size_t shared_memory_size,
    base::SyncSocket::Handle socket_handle) {
  // Make sure the handles are cleaned up.
  base::SyncSocket scoped_socket(socket_handle);
  base::SharedMemory scoped_shared_memory(shared_memory_handle, false);

  if (!open_context_.is_valid()) {
    NOTREACHED();
    return;
  }

  ppapi::proxy::SerializedHandle serialized_socket_handle(
      ppapi::proxy::SerializedHandle::SOCKET);
  ppapi::proxy::SerializedHandle serialized_shared_memory_handle(
      ppapi::proxy::SerializedHandle::SHARED_MEMORY);

  if (result == PP_OK) {
    IPC::PlatformFileForTransit temp_socket =
        IPC::InvalidPlatformFileForTransit();
    base::SharedMemoryHandle temp_shmem;
    result = GetRemoteHandles(
        scoped_socket, scoped_shared_memory, &temp_socket, &temp_shmem);

    serialized_socket_handle.set_socket(temp_socket);
    serialized_shared_memory_handle.set_shmem(temp_shmem, shared_memory_size);
  }

  // Send all the values, even on error. This simplifies some of our cleanup
  // code since the handles will be in the other process and could be
  // inconvenient to clean up. Our IPC code will automatically handle this for
  // us, as long as the remote side always closes the handles it receives, even
  // in the failure case.
  open_context_.params.AppendHandle(serialized_socket_handle);
  open_context_.params.AppendHandle(serialized_shared_memory_handle);
  SendOpenReply(result);
}

int32_t PepperAudioInputHost::GetRemoteHandles(
    const base::SyncSocket& socket,
    const base::SharedMemory& shared_memory,
    IPC::PlatformFileForTransit* remote_socket_handle,
    base::SharedMemoryHandle* remote_shared_memory_handle) {
  *remote_socket_handle = renderer_ppapi_host_->ShareHandleWithRemote(
      ConvertSyncSocketHandle(socket), false);
  if (*remote_socket_handle == IPC::InvalidPlatformFileForTransit())
    return PP_ERROR_FAILED;

  *remote_shared_memory_handle =
      renderer_ppapi_host_->ShareSharedMemoryHandleWithRemote(
          shared_memory.handle());
  if (!base::SharedMemory::IsHandleValid(*remote_shared_memory_handle))
    return PP_ERROR_FAILED;

  return PP_OK;
}

void PepperAudioInputHost::Close() {
  if (!audio_input_)
    return;

  audio_input_->ShutDown();
  audio_input_ = NULL;

  if (open_context_.is_valid())
    SendOpenReply(PP_ERROR_ABORTED);
}

void PepperAudioInputHost::SendOpenReply(int32_t result) {
  open_context_.params.set_result(result);
  host()->SendReply(open_context_, PpapiPluginMsg_AudioInput_OpenReply());
  open_context_ = ppapi::host::ReplyMessageContext();
}

}  // namespace content
