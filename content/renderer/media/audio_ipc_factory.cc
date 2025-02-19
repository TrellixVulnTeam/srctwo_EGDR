// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/media/audio_ipc_factory.h"

#include <utility>

#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/single_thread_task_runner.h"
#include "content/renderer/media/audio_message_filter.h"
#include "content/renderer/media/mojo_audio_output_ipc.h"
#include "services/service_manager/public/cpp/interface_provider.h"

namespace content {

AudioIPCFactory* AudioIPCFactory::instance_ = nullptr;

AudioIPCFactory::AudioIPCFactory(
    scoped_refptr<AudioMessageFilter> audio_message_filter,
    scoped_refptr<base::SingleThreadTaskRunner> io_task_runner)
    : audio_message_filter_(std::move(audio_message_filter)),
      io_task_runner_(std::move(io_task_runner)) {
  DCHECK(!instance_);
  instance_ = this;
}

AudioIPCFactory::~AudioIPCFactory() {
  // Allow destruction in tests.
  DCHECK(factory_ptrs_.empty());
  DCHECK_EQ(instance_, this);
  instance_ = nullptr;
}

std::unique_ptr<media::AudioOutputIPC> AudioIPCFactory::CreateAudioOutputIPC(
    int frame_id) const {
  if (UsingMojoFactories()) {
    // Unretained is safe due to the contract at the top of the header file.
    return base::MakeUnique<MojoAudioOutputIPC>(base::BindRepeating(
        &AudioIPCFactory::GetRemoteFactory, base::Unretained(this), frame_id));
  }
  return audio_message_filter_->CreateAudioOutputIPC(frame_id);
}

void AudioIPCFactory::MaybeRegisterRemoteFactory(
    int frame_id,
    service_manager::InterfaceProvider* interface_provider) {
  if (!UsingMojoFactories())
    return;
  mojom::RendererAudioOutputStreamFactoryPtr factory_ptr;
  interface_provider->GetInterface(&factory_ptr);
  // PassInterface unbinds the message pipe from the current thread. This
  // allows us to bind it to the IO thread.
  // Unretained is safe due to the contract at the top of the header file.
  io_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&AudioIPCFactory::RegisterRemoteFactoryOnIOThread,
                     base::Unretained(this), frame_id,
                     factory_ptr.PassInterface()));
}

void AudioIPCFactory::MaybeDeregisterRemoteFactory(int frame_id) {
  if (!UsingMojoFactories())
    return;
  io_task_runner_->PostTask(
      FROM_HERE,
      base::BindOnce(&AudioIPCFactory::MaybeDeregisterRemoteFactoryOnIOThread,
                     base::Unretained(this), frame_id));
}

mojom::RendererAudioOutputStreamFactory* AudioIPCFactory::GetRemoteFactory(
    int frame_id) const {
  DCHECK(io_task_runner_->BelongsToCurrentThread());
  DCHECK(UsingMojoFactories());
  auto it = factory_ptrs_.find(frame_id);
  return it == factory_ptrs_.end() ? nullptr : it->second.get();
}

void AudioIPCFactory::RegisterRemoteFactoryOnIOThread(
    int frame_id,
    mojom::RendererAudioOutputStreamFactoryPtrInfo factory_ptr_info) {
  DCHECK(io_task_runner_->BelongsToCurrentThread());
  DCHECK(UsingMojoFactories());
  std::pair<StreamFactoryMap::iterator, bool> emplace_result =
      factory_ptrs_.emplace(frame_id,
                            mojo::MakeProxy(std::move(factory_ptr_info)));

  DCHECK(emplace_result.second) << "Attempt to register a factory for a "
                                   "frame which already has a factory "
                                   "registered.";

  auto& emplaced_factory = emplace_result.first->second;
  DCHECK(emplaced_factory.is_bound())
      << "Factory is not bound to a remote implementation.";

  // Unretained is safe because |this| owns the binding, so a connection error
  // cannot trigger after destruction.
  emplaced_factory.set_connection_error_handler(
      base::BindOnce(&AudioIPCFactory::MaybeDeregisterRemoteFactory,
                     base::Unretained(this), frame_id));
}

void AudioIPCFactory::MaybeDeregisterRemoteFactoryOnIOThread(int frame_id) {
  DCHECK(io_task_runner_->BelongsToCurrentThread());
  DCHECK(UsingMojoFactories());
  // This function can be called both by the frame and the connection error
  // handler of the factory pointer. Calling erase multiple times even though
  // there is nothing to erase is safe, so we don't have to handle this in any
  // particular way.
  factory_ptrs_.erase(frame_id);
}

bool AudioIPCFactory::UsingMojoFactories() const {
  return audio_message_filter_ == nullptr;
}

}  // namespace content
