// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/mojo/services/mojo_decryptor_service.h"

#include <utility>

#include "base/bind.h"
#include "base/numerics/safe_conversions.h"
#include "media/base/audio_decoder_config.h"
#include "media/base/decoder_buffer.h"
#include "media/base/decryptor.h"
#include "media/base/video_decoder_config.h"
#include "media/base/video_frame.h"
#include "media/mojo/common/media_type_converters.h"
#include "media/mojo/common/mojo_decoder_buffer_converter.h"
#include "media/mojo/common/mojo_shared_buffer_video_frame.h"
#include "media/mojo/interfaces/demuxer_stream.mojom.h"
#include "mojo/public/cpp/bindings/strong_binding.h"

namespace media {

namespace {

// A mojom::FrameResourceReleaser implementation. This object is created when
// DecryptAndDecodeVideo() returns a shared memory video frame, and holds
// on to the local frame. When MojoDecryptor is done using the frame,
// the connection should be broken and this will free the shared resources
// associated with the frame.
class FrameResourceReleaserImpl final : public mojom::FrameResourceReleaser {
 public:
  explicit FrameResourceReleaserImpl(scoped_refptr<VideoFrame> frame)
      : frame_(std::move(frame)) {
    DVLOG(1) << __func__;
    DCHECK_EQ(VideoFrame::STORAGE_MOJO_SHARED_BUFFER, frame_->storage_type());
  }
  ~FrameResourceReleaserImpl() override { DVLOG(1) << __func__; }

 private:
  scoped_refptr<VideoFrame> frame_;

  DISALLOW_COPY_AND_ASSIGN(FrameResourceReleaserImpl);
};

}  // namespace

MojoDecryptorService::MojoDecryptorService(
    media::Decryptor* decryptor,
    mojo::InterfaceRequest<mojom::Decryptor> request,
    const base::Closure& error_handler)
    : binding_(this, std::move(request)),
      decryptor_(decryptor),
      weak_factory_(this) {
  DVLOG(1) << __func__;
  DCHECK(decryptor_);
  weak_this_ = weak_factory_.GetWeakPtr();
  binding_.set_connection_error_handler(error_handler);
}

MojoDecryptorService::~MojoDecryptorService() {}

void MojoDecryptorService::Initialize(
    mojo::ScopedDataPipeConsumerHandle receive_pipe,
    mojo::ScopedDataPipeProducerHandle transmit_pipe) {
  mojo_decoder_buffer_writer_.reset(
      new MojoDecoderBufferWriter(std::move(transmit_pipe)));
  mojo_decoder_buffer_reader_.reset(
      new MojoDecoderBufferReader(std::move(receive_pipe)));
}

void MojoDecryptorService::Decrypt(StreamType stream_type,
                                   mojom::DecoderBufferPtr encrypted,
                                   DecryptCallback callback) {
  DVLOG(3) << __func__;
  mojo_decoder_buffer_reader_->ReadDecoderBuffer(
      std::move(encrypted),
      base::BindOnce(&MojoDecryptorService::OnReadDone, weak_this_, stream_type,
                     std::move(callback)));
}

void MojoDecryptorService::CancelDecrypt(StreamType stream_type) {
  DVLOG(1) << __func__;
  decryptor_->CancelDecrypt(stream_type);
}

void MojoDecryptorService::InitializeAudioDecoder(
    const AudioDecoderConfig& config,
    InitializeAudioDecoderCallback callback) {
  DVLOG(1) << __func__;
  decryptor_->InitializeAudioDecoder(
      config, base::Bind(&MojoDecryptorService::OnAudioDecoderInitialized,
                         weak_this_, base::Passed(&callback)));
}

void MojoDecryptorService::InitializeVideoDecoder(
    const VideoDecoderConfig& config,
    InitializeVideoDecoderCallback callback) {
  DVLOG(1) << __func__;
  decryptor_->InitializeVideoDecoder(
      config, base::Bind(&MojoDecryptorService::OnVideoDecoderInitialized,
                         weak_this_, base::Passed(&callback)));
}

void MojoDecryptorService::DecryptAndDecodeAudio(
    mojom::DecoderBufferPtr encrypted,
    DecryptAndDecodeAudioCallback callback) {
  DVLOG(3) << __func__;
  mojo_decoder_buffer_reader_->ReadDecoderBuffer(
      std::move(encrypted), base::BindOnce(&MojoDecryptorService::OnAudioRead,
                                           weak_this_, std::move(callback)));
}

void MojoDecryptorService::DecryptAndDecodeVideo(
    mojom::DecoderBufferPtr encrypted,
    DecryptAndDecodeVideoCallback callback) {
  DVLOG(3) << __func__;
  mojo_decoder_buffer_reader_->ReadDecoderBuffer(
      std::move(encrypted), base::BindOnce(&MojoDecryptorService::OnVideoRead,
                                           weak_this_, std::move(callback)));
}

void MojoDecryptorService::ResetDecoder(StreamType stream_type) {
  DVLOG(1) << __func__;
  decryptor_->ResetDecoder(stream_type);
}

void MojoDecryptorService::DeinitializeDecoder(StreamType stream_type) {
  DVLOG(1) << __func__;
  decryptor_->DeinitializeDecoder(stream_type);
}

void MojoDecryptorService::OnReadDone(StreamType stream_type,
                                      DecryptCallback callback,
                                      scoped_refptr<DecoderBuffer> buffer) {
  if (!buffer) {
    std::move(callback).Run(Status::kError, nullptr);
    return;
  }

  decryptor_->Decrypt(stream_type, std::move(buffer),
                      base::Bind(&MojoDecryptorService::OnDecryptDone,
                                 weak_this_, base::Passed(&callback)));
}

void MojoDecryptorService::OnDecryptDone(
    DecryptCallback callback,
    Status status,
    const scoped_refptr<DecoderBuffer>& buffer) {
  DVLOG_IF(1, status != Status::kSuccess) << __func__ << "(" << status << ")";
  DVLOG_IF(3, status == Status::kSuccess) << __func__;

  if (!buffer) {
    DCHECK_NE(status, Status::kSuccess);
    std::move(callback).Run(status, nullptr);
    return;
  }

  mojom::DecoderBufferPtr mojo_buffer =
      mojo_decoder_buffer_writer_->WriteDecoderBuffer(buffer);
  if (!mojo_buffer) {
    std::move(callback).Run(Status::kError, nullptr);
    return;
  }

  std::move(callback).Run(status, std::move(mojo_buffer));
}

void MojoDecryptorService::OnAudioDecoderInitialized(
    InitializeAudioDecoderCallback callback,
    bool success) {
  DVLOG(1) << __func__ << "(" << success << ")";
  std::move(callback).Run(success);
}

void MojoDecryptorService::OnVideoDecoderInitialized(
    InitializeVideoDecoderCallback callback,
    bool success) {
  DVLOG(1) << __func__ << "(" << success << ")";
  std::move(callback).Run(success);
}

void MojoDecryptorService::OnAudioRead(DecryptAndDecodeAudioCallback callback,
                                       scoped_refptr<DecoderBuffer> buffer) {
  if (!buffer) {
    std::move(callback).Run(Status::kError,
                            std::vector<mojom::AudioBufferPtr>());
    return;
  }

  decryptor_->DecryptAndDecodeAudio(
      std::move(buffer), base::Bind(&MojoDecryptorService::OnAudioDecoded,
                                    weak_this_, base::Passed(&callback)));
}

void MojoDecryptorService::OnVideoRead(DecryptAndDecodeVideoCallback callback,
                                       scoped_refptr<DecoderBuffer> buffer) {
  if (!buffer) {
    std::move(callback).Run(Status::kError, nullptr, nullptr);
    return;
  }

  decryptor_->DecryptAndDecodeVideo(
      std::move(buffer), base::Bind(&MojoDecryptorService::OnVideoDecoded,
                                    weak_this_, base::Passed(&callback)));
}

void MojoDecryptorService::OnAudioDecoded(
    DecryptAndDecodeAudioCallback callback,
    Status status,
    const media::Decryptor::AudioFrames& frames) {
  DVLOG_IF(1, status != Status::kSuccess) << __func__ << "(" << status << ")";
  DVLOG_IF(3, status == Status::kSuccess) << __func__;

  // Note that the audio data is sent over the mojo pipe. This could be
  // improved to use shared memory (http://crbug.com/593896).
  std::vector<mojom::AudioBufferPtr> audio_buffers;
  for (const auto& frame : frames)
    audio_buffers.push_back(mojom::AudioBuffer::From(frame));

  std::move(callback).Run(status, std::move(audio_buffers));
}

void MojoDecryptorService::OnVideoDecoded(
    DecryptAndDecodeVideoCallback callback,
    Status status,
    const scoped_refptr<VideoFrame>& frame) {
  DVLOG_IF(1, status != Status::kSuccess)
      << __func__ << ": status = " << status;
  DVLOG_IF(3, status == Status::kSuccess) << __func__;

  if (!frame) {
    DCHECK_NE(status, Status::kSuccess);
    std::move(callback).Run(status, nullptr, nullptr);
    return;
  }

  // If |frame| has shared memory that will be passed back, keep the reference
  // to it until the other side is done with the memory.
  mojom::FrameResourceReleaserPtr releaser;
  if (frame->storage_type() == VideoFrame::STORAGE_MOJO_SHARED_BUFFER) {
    mojo::MakeStrongBinding(base::MakeUnique<FrameResourceReleaserImpl>(frame),
                            mojo::MakeRequest(&releaser));
  }

  std::move(callback).Run(status, std::move(frame), std::move(releaser));
}

}  // namespace media
