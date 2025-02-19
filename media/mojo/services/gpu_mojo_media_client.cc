// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/mojo/services/gpu_mojo_media_client.h"

#include <utility>

#include "base/bind.h"
#include "gpu/ipc/service/gpu_channel.h"
#include "media/base/audio_decoder.h"
#include "media/base/cdm_factory.h"
#include "media/base/video_decoder.h"
#include "media/gpu/features.h"
#include "media/gpu/ipc/service/media_gpu_channel_manager.h"

#if defined(OS_ANDROID)
#include "base/memory/ptr_util.h"
#include "media/base/android/android_cdm_factory.h"
#include "media/filters/android/media_codec_audio_decoder.h"
#include "media/gpu/android/media_codec_video_decoder.h"
#include "media/gpu/android/video_frame_factory_impl.h"
#include "media/gpu/android_video_surface_chooser_impl.h"
#include "media/gpu/avda_codec_allocator.h"
#include "media/mojo/interfaces/media_drm_storage.mojom.h"
#include "media/mojo/interfaces/provision_fetcher.mojom.h"
#include "media/mojo/services/mojo_media_drm_storage.h"
#include "media/mojo/services/mojo_provision_fetcher.h"
#include "services/service_manager/public/cpp/connect.h"
#endif  // defined(OS_ANDROID)

namespace media {

namespace {

// TODO(xhwang): Remove the duplicate code between GpuMojoMediaClient and
// AndroidMojoMediaClient.

#if defined(OS_ANDROID)
std::unique_ptr<ProvisionFetcher> CreateProvisionFetcher(
    service_manager::mojom::InterfaceProvider* interface_provider) {
  mojom::ProvisionFetcherPtr provision_fetcher_ptr;
  service_manager::GetInterface(interface_provider, &provision_fetcher_ptr);
  return base::MakeUnique<MojoProvisionFetcher>(
      std::move(provision_fetcher_ptr));
}

std::unique_ptr<MediaDrmStorage> CreateMediaDrmStorage(
    service_manager::mojom::InterfaceProvider* host_interfaces) {
  DCHECK(host_interfaces);
  mojom::MediaDrmStoragePtr media_drm_storage_ptr;
  service_manager::GetInterface(host_interfaces, &media_drm_storage_ptr);
  return base::MakeUnique<MojoMediaDrmStorage>(
      std::move(media_drm_storage_ptr));
}

#if BUILDFLAG(ENABLE_MEDIA_CODEC_VIDEO_DECODER)
gpu::GpuCommandBufferStub* GetGpuCommandBufferStub(
    base::WeakPtr<MediaGpuChannelManager> media_gpu_channel_manager,
    base::UnguessableToken channel_token,
    int32_t route_id) {
  if (!media_gpu_channel_manager)
    return nullptr;

  gpu::GpuChannel* channel =
      media_gpu_channel_manager->LookupChannel(channel_token);
  if (!channel)
    return nullptr;

  return channel->LookupCommandBuffer(route_id);
}
#endif  // BUILDFLAG(ENABLE_MEDIA_CODEC_VIDEO_DECODER)
#endif  // defined(OS_ANDROID)

}  // namespace

GpuMojoMediaClient::GpuMojoMediaClient(
    scoped_refptr<base::SingleThreadTaskRunner> gpu_task_runner,
    base::WeakPtr<MediaGpuChannelManager> media_gpu_channel_manager)
    : gpu_task_runner_(std::move(gpu_task_runner)),
      media_gpu_channel_manager_(std::move(media_gpu_channel_manager)) {}

GpuMojoMediaClient::~GpuMojoMediaClient() {}

void GpuMojoMediaClient::Initialize(
    service_manager::Connector* connector,
    service_manager::ServiceContextRefFactory* context_ref_factory) {
  DCHECK(context_ref_factory);
  context_ref_factory_ = context_ref_factory;
}

std::unique_ptr<AudioDecoder> GpuMojoMediaClient::CreateAudioDecoder(
    scoped_refptr<base::SingleThreadTaskRunner> task_runner) {
#if defined(OS_ANDROID)
  return base::MakeUnique<MediaCodecAudioDecoder>(task_runner);
#else
  return nullptr;
#endif  // defined(OS_ANDROID)
}

std::unique_ptr<VideoDecoder> GpuMojoMediaClient::CreateVideoDecoder(
    scoped_refptr<base::SingleThreadTaskRunner> task_runner,
    MediaLog* media_log,
    mojom::CommandBufferIdPtr command_buffer_id,
    OutputWithReleaseMailboxCB output_cb) {
#if BUILDFLAG(ENABLE_MEDIA_CODEC_VIDEO_DECODER)
  return base::MakeUnique<MediaCodecVideoDecoder>(
      gpu_task_runner_,
      base::Bind(&GetGpuCommandBufferStub, media_gpu_channel_manager_,
                 command_buffer_id->channel_token, command_buffer_id->route_id),
      std::move(output_cb), DeviceInfo::GetInstance(),
      AVDACodecAllocator::GetInstance(),
      base::MakeUnique<AndroidVideoSurfaceChooserImpl>(
          DeviceInfo::GetInstance()->IsSetOutputSurfaceSupported()),
      base::MakeUnique<VideoFrameFactoryImpl>(),
      context_ref_factory_->CreateRef());
#else
  return nullptr;
#endif  // BUILDFLAG(ENABLE_MEDIA_CODEC_VIDEO_DECODER)
}

std::unique_ptr<CdmFactory> GpuMojoMediaClient::CreateCdmFactory(
    service_manager::mojom::InterfaceProvider* interface_provider) {
#if defined(OS_ANDROID)
  return base::MakeUnique<AndroidCdmFactory>(
      base::Bind(&CreateProvisionFetcher, interface_provider),
      base::Bind(&CreateMediaDrmStorage, interface_provider));
#else
  return nullptr;
#endif  // defined(OS_ANDROID)
}

}  // namespace media
