// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromecast/media/cma/base/buffering_frame_provider.h"

#include <utility>

#include "base/bind.h"
#include "base/callback_helpers.h"
#include "chromecast/media/cma/base/buffering_state.h"
#include "chromecast/media/cma/base/decoder_buffer_base.h"
#include "media/base/bind_to_current_loop.h"

namespace chromecast {
namespace media {

BufferingFrameProvider::BufferWithConfig::BufferWithConfig(
    const scoped_refptr<DecoderBufferBase>& buffer,
    const ::media::AudioDecoderConfig& audio_config,
    const ::media::VideoDecoderConfig& video_config)
    : buffer_(buffer),
      audio_config_(audio_config),
      video_config_(video_config) {
}

BufferingFrameProvider::BufferWithConfig::BufferWithConfig(
    const BufferWithConfig& other) = default;

BufferingFrameProvider::BufferWithConfig::~BufferWithConfig() {
}

BufferingFrameProvider::BufferingFrameProvider(
    std::unique_ptr<CodedFrameProvider> coded_frame_provider,
    size_t max_buffer_size,
    size_t max_frame_size,
    const FrameBufferedCB& frame_buffered_cb)
    : coded_frame_provider_(std::move(coded_frame_provider)),
      is_pending_request_(false),
      is_eos_(false),
      total_buffer_size_(0),
      max_buffer_size_(max_buffer_size),
      max_frame_size_(max_frame_size),
      frame_buffered_cb_(frame_buffered_cb),
      weak_factory_(this) {
  DCHECK_LE(max_frame_size, max_buffer_size);
  weak_this_ = weak_factory_.GetWeakPtr();
  thread_checker_.DetachFromThread();
}

BufferingFrameProvider::~BufferingFrameProvider() {
  // Required since some weak pointers might be released in the destructor.
  DCHECK(thread_checker_.CalledOnValidThread());
}

void BufferingFrameProvider::Read(const ReadCB& read_cb) {
  DCHECK(thread_checker_.CalledOnValidThread());

  DCHECK(!read_cb.is_null());
  read_cb_ = read_cb;

  CompleteReadIfNeeded();

  RequestBufferIfNeeded();
}

void BufferingFrameProvider::Flush(const base::Closure& flush_cb) {
  DCHECK(thread_checker_.CalledOnValidThread());

  // Invalidate all the buffers that belong to this media timeline.
  // This is needed since, even though |coded_frame_provider_| is flushed later
  // in this function, there might be a pending task holding onto a buffer.
  weak_factory_.InvalidateWeakPtrs();

  // Create a new valid weak pointer that is used for the next media timeline.
  weak_this_ = weak_factory_.GetWeakPtr();

  is_pending_request_ = false;
  is_eos_ = false;
  buffer_list_.clear();
  total_buffer_size_ = 0;
  read_cb_.Reset();
  coded_frame_provider_->Flush(flush_cb);
}

void BufferingFrameProvider::OnNewBuffer(
    const scoped_refptr<DecoderBufferBase>& buffer,
    const ::media::AudioDecoderConfig& audio_config,
    const ::media::VideoDecoderConfig& video_config) {
  is_pending_request_ = false;
  buffer_list_.push_back(
      BufferWithConfig(buffer, audio_config, video_config));

  if (buffer->end_of_stream()) {
    is_eos_ = true;
  } else {
    total_buffer_size_ += buffer->data_size();
  }

  if (!frame_buffered_cb_.is_null()) {
    // If the next upcoming frame is possibly filling the whole buffer,
    // then the buffer is considered as having reached its max capacity.
    bool max_capacity_flag =
        (total_buffer_size_ + max_frame_size_ >= max_buffer_size_);
    frame_buffered_cb_.Run(buffer, max_capacity_flag);
  }

  RequestBufferIfNeeded();

  CompleteReadIfNeeded();
}

void BufferingFrameProvider::RequestBufferIfNeeded() {
  if (is_pending_request_)
    return;

  if (is_eos_ || total_buffer_size_ >= max_buffer_size_)
    return;

  is_pending_request_ = true;
  coded_frame_provider_->Read(::media::BindToCurrentLoop(
      base::Bind(&BufferingFrameProvider::OnNewBuffer, weak_this_)));
}

void BufferingFrameProvider::CompleteReadIfNeeded() {
  if (read_cb_.is_null())
    return;

  if (buffer_list_.empty())
    return;

  BufferWithConfig buffer_with_config(buffer_list_.front());
  buffer_list_.pop_front();
  if (!buffer_with_config.buffer()->end_of_stream())
    total_buffer_size_ -= buffer_with_config.buffer()->data_size();

  base::ResetAndReturn(&read_cb_).Run(
      buffer_with_config.buffer(),
      buffer_with_config.audio_config(),
      buffer_with_config.video_config());
}

}  // namespace media
}  // namespace chromecast
