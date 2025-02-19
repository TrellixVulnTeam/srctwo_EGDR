// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_PEPPER_PEPPER_VIDEO_DECODER_HOST_H_
#define CONTENT_RENDERER_PEPPER_PEPPER_VIDEO_DECODER_HOST_H_

#include <stdint.h>

#include <list>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include "base/containers/hash_tables.h"
#include "base/macros.h"
#include "content/common/content_export.h"
#include "media/video/video_decode_accelerator.h"
#include "ppapi/c/pp_codecs.h"
#include "ppapi/host/host_message_context.h"
#include "ppapi/host/resource_host.h"
#include "ppapi/proxy/resource_message_params.h"

namespace base {
class SharedMemory;
}

namespace content {

class RendererPpapiHost;
class VideoDecoderShim;

class CONTENT_EXPORT PepperVideoDecoderHost
    : public ppapi::host::ResourceHost,
      public media::VideoDecodeAccelerator::Client {
 public:
  PepperVideoDecoderHost(RendererPpapiHost* host,
                         PP_Instance instance,
                         PP_Resource resource);
  ~PepperVideoDecoderHost() override;

 private:
  enum class PictureBufferState {
    ASSIGNED,
    IN_USE,
    DISMISSED,
  };

  struct PendingDecode {
    PendingDecode(int32_t decode_id,
                  uint32_t shm_id,
                  uint32_t size,
                  const ppapi::host::ReplyMessageContext& reply_context);
    ~PendingDecode();

    const int32_t decode_id;
    const uint32_t shm_id;
    const uint32_t size;
    const ppapi::host::ReplyMessageContext reply_context;
  };
  typedef std::list<PendingDecode> PendingDecodeList;

  friend class VideoDecoderShim;

  // ResourceHost implementation.
  int32_t OnResourceMessageReceived(
      const IPC::Message& msg,
      ppapi::host::HostMessageContext* context) override;

  // media::VideoDecodeAccelerator::Client implementation.
  void ProvidePictureBuffers(uint32_t requested_num_of_buffers,
                             media::VideoPixelFormat format,
                             uint32_t textures_per_buffer,
                             const gfx::Size& dimensions,
                             uint32_t texture_target) override;
  void DismissPictureBuffer(int32_t picture_buffer_id) override;
  void PictureReady(const media::Picture& picture) override;
  void NotifyEndOfBitstreamBuffer(int32_t bitstream_buffer_id) override;
  void NotifyFlushDone() override;
  void NotifyResetDone() override;
  void NotifyError(media::VideoDecodeAccelerator::Error error) override;

  int32_t OnHostMsgInitialize(ppapi::host::HostMessageContext* context,
                              const ppapi::HostResource& graphics_context,
                              PP_VideoProfile profile,
                              PP_HardwareAcceleration acceleration,
                              uint32_t min_picture_count);
  int32_t OnHostMsgGetShm(ppapi::host::HostMessageContext* context,
                          uint32_t shm_id,
                          uint32_t shm_size);
  int32_t OnHostMsgDecode(ppapi::host::HostMessageContext* context,
                          uint32_t shm_id,
                          uint32_t size,
                          int32_t decode_id);
  int32_t OnHostMsgAssignTextures(ppapi::host::HostMessageContext* context,
                                  const PP_Size& size,
                                  const std::vector<uint32_t>& texture_ids);
  int32_t OnHostMsgRecyclePicture(ppapi::host::HostMessageContext* context,
                                  uint32_t picture_id);
  int32_t OnHostMsgFlush(ppapi::host::HostMessageContext* context);
  int32_t OnHostMsgReset(ppapi::host::HostMessageContext* context);

  // These methods are needed by VideoDecodeShim, to look like a
  // VideoDecodeAccelerator.
  const uint8_t* DecodeIdToAddress(uint32_t decode_id);
  void RequestTextures(uint32_t requested_num_of_buffers,
                       const gfx::Size& dimensions,
                       uint32_t texture_target,
                       const std::vector<gpu::Mailbox>& mailboxes);

  // Tries to initialize software decoder. Returns true on success.
  bool TryFallbackToSoftwareDecoder();

  PendingDecodeList::iterator GetPendingDecodeById(int32_t decode_id);

  // Non-owning pointer.
  RendererPpapiHost* renderer_ppapi_host_;

  media::VideoCodecProfile profile_;

  std::unique_ptr<media::VideoDecodeAccelerator> decoder_;

  bool software_fallback_allowed_ = false;
  bool software_fallback_used_ = false;

  int pending_texture_requests_ = 0;

  // Set after software decoder fallback to dismiss all outstanding texture
  // requests.
  int assign_textures_messages_to_dismiss_ = 0;

  // A vector holding our shm buffers, in sync with a similar vector in the
  // resource. We use a buffer's index in these vectors as its id on both sides
  // of the proxy. Only add buffers or update them in place so as not to
  // invalidate these ids.
  std::vector<std::unique_ptr<base::SharedMemory>> shm_buffers_;
  // A vector of true/false indicating if a shm buffer is in use by the decoder.
  // This is parallel to |shm_buffers_|.
  std::vector<uint8_t> shm_buffer_busy_;

  uint32_t min_picture_count_;
  typedef std::map<uint32_t, PictureBufferState> PictureBufferMap;
  PictureBufferMap picture_buffer_map_;

  // Keeps list of pending decodes.
  PendingDecodeList pending_decodes_;

  ppapi::host::ReplyMessageContext flush_reply_context_;
  ppapi::host::ReplyMessageContext reset_reply_context_;

  bool initialized_ = false;

  DISALLOW_COPY_AND_ASSIGN(PepperVideoDecoderHost);
};

}  // namespace content

#endif  // CONTENT_RENDERER_PEPPER_PEPPER_VIDEO_DECODER_HOST_H_
