// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/mojo/services/mojo_video_decoder_service.h"

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/logging.h"
#include "base/optional.h"
#include "base/threading/thread_task_runner_handle.h"
#include "media/base/decoder_buffer.h"
#include "media/base/video_decoder.h"
#include "media/base/video_decoder_config.h"
#include "media/base/video_frame.h"
#include "media/mojo/common/media_type_converters.h"
#include "media/mojo/common/mojo_decoder_buffer_converter.h"
#include "media/mojo/services/mojo_media_client.h"
#include "media/mojo/services/mojo_media_log.h"
#include "mojo/public/c/system/types.h"
#include "mojo/public/cpp/system/buffer.h"
#include "mojo/public/cpp/system/handle.h"

namespace media {

MojoVideoDecoderService::MojoVideoDecoderService(
    MojoMediaClient* mojo_media_client)
    : mojo_media_client_(mojo_media_client), weak_factory_(this) {
  weak_this_ = weak_factory_.GetWeakPtr();
}

MojoVideoDecoderService::~MojoVideoDecoderService() {}

void MojoVideoDecoderService::Construct(
    mojom::VideoDecoderClientAssociatedPtrInfo client,
    mojom::MediaLogAssociatedPtrInfo media_log,
    mojo::ScopedDataPipeConsumerHandle decoder_buffer_pipe,
    mojom::CommandBufferIdPtr command_buffer_id) {
  DVLOG(1) << __func__;

  if (decoder_) {
    // TODO(sandersd): Close the channel.
    return;
  }

  client_.Bind(std::move(client));

  mojom::MediaLogAssociatedPtr media_log_ptr;
  media_log_ptr.Bind(std::move(media_log));
  media_log_ = base::MakeUnique<MojoMediaLog>(std::move(media_log_ptr));

  mojo_decoder_buffer_reader_.reset(
      new MojoDecoderBufferReader(std::move(decoder_buffer_pipe)));

  decoder_ = mojo_media_client_->CreateVideoDecoder(
      base::ThreadTaskRunnerHandle::Get(), media_log_.get(),
      std::move(command_buffer_id),
      base::Bind(&MojoVideoDecoderService::OnDecoderOutput, weak_this_));
}

void MojoVideoDecoderService::Initialize(const VideoDecoderConfig& config,
                                         bool low_delay,
                                         InitializeCallback callback) {
  DVLOG(1) << __func__;

  if (!decoder_) {
    std::move(callback).Run(false, false, 1);
    return;
  }

  decoder_->Initialize(
      config, low_delay, nullptr,
      base::Bind(&MojoVideoDecoderService::OnDecoderInitialized, weak_this_,
                 base::Passed(&callback)),
      base::Bind(&MojoVideoDecoderService::OnDecoderOutput, weak_this_,
                 MojoMediaClient::ReleaseMailboxCB()));
}

void MojoVideoDecoderService::Decode(mojom::DecoderBufferPtr buffer,
                                     DecodeCallback callback) {
  DVLOG(2) << __func__ << " pts=" << buffer->timestamp.InMilliseconds();
  if (!decoder_) {
    std::move(callback).Run(DecodeStatus::DECODE_ERROR);
    return;
  }

  mojo_decoder_buffer_reader_->ReadDecoderBuffer(
      std::move(buffer), base::BindOnce(&MojoVideoDecoderService::OnDecoderRead,
                                        weak_this_, std::move(callback)));
}

void MojoVideoDecoderService::Reset(ResetCallback callback) {
  DVLOG(1) << __func__;

  if (!decoder_) {
    std::move(callback).Run();
    return;
  }

  decoder_->Reset(base::Bind(&MojoVideoDecoderService::OnDecoderReset,
                             weak_this_, base::Passed(&callback)));
}

void MojoVideoDecoderService::OnDecoderInitialized(InitializeCallback callback,
                                                   bool success) {
  DVLOG(1) << __func__;
  DCHECK(decoder_);
  std::move(callback).Run(success, decoder_->NeedsBitstreamConversion(),
                          decoder_->GetMaxDecodeRequests());
}

void MojoVideoDecoderService::OnDecoderRead(
    DecodeCallback callback,
    scoped_refptr<DecoderBuffer> buffer) {
  if (!buffer) {
    // TODO(sandersd): Close the channel.
    std::move(callback).Run(DecodeStatus::DECODE_ERROR);
    return;
  }

  decoder_->Decode(
      buffer, base::Bind(&MojoVideoDecoderService::OnDecoderDecoded, weak_this_,
                         base::Passed(&callback)));
}

void MojoVideoDecoderService::OnDecoderDecoded(DecodeCallback callback,
                                               DecodeStatus status) {
  DVLOG(2) << __func__;
  std::move(callback).Run(status);
}

void MojoVideoDecoderService::OnDecoderReset(ResetCallback callback) {
  DVLOG(1) << __func__;
  std::move(callback).Run();
}

void MojoVideoDecoderService::OnDecoderOutput(
    MojoMediaClient::ReleaseMailboxCB release_cb,
    const scoped_refptr<VideoFrame>& frame) {
  DVLOG(2) << __func__ << " pts=" << frame->timestamp().InMilliseconds();
  DCHECK(client_);
  DCHECK(decoder_);

  base::Optional<base::UnguessableToken> release_token;
  if (release_cb) {
    release_token = base::UnguessableToken::Create();
    release_mailbox_cbs_[*release_token] = std::move(release_cb);
  }

  client_->OnVideoFrameDecoded(frame, decoder_->CanReadWithoutStalling(),
                               std::move(release_token));
}

void MojoVideoDecoderService::OnReleaseMailbox(
    const base::UnguessableToken& release_token,
    const gpu::SyncToken& release_sync_token) {
  DVLOG(2) << __func__;

  // TODO(sandersd): Is it a serious error for the client to call
  // OnReleaseMailbox() with an invalid |release_token|?
  auto it = release_mailbox_cbs_.find(release_token);
  if (it == release_mailbox_cbs_.end())
    return;

  MojoMediaClient::ReleaseMailboxCB cb = std::move(it->second);
  release_mailbox_cbs_.erase(it);
  std::move(cb).Run(release_sync_token);
}

}  // namespace media
