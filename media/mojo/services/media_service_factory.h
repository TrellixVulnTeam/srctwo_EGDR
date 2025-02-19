// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_MOJO_SERVICES_MEDIA_SERVICE_FACTORY_H_
#define MEDIA_MOJO_SERVICES_MEDIA_SERVICE_FACTORY_H_

#include <memory>

#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/single_thread_task_runner.h"
#include "media/mojo/services/media_mojo_export.h"
#include "services/service_manager/public/cpp/service.h"

namespace media {

class MediaGpuChannelManager;

// Creates a MediaService instance using the default MojoMediaClient on each
// platform. Uses the TestMojoMediaClient if |enable_test_mojo_media_client| is
// true.
std::unique_ptr<service_manager::Service> MEDIA_MOJO_EXPORT
CreateMediaService();

// Creates a MediaService instance using the GpuMojoMediaClient.
// |media_gpu_channel_manager| must only be used on |task_runner|, which is
// expected to be the GPU main thread task runner.
std::unique_ptr<service_manager::Service> MEDIA_MOJO_EXPORT
CreateGpuMediaService(
    scoped_refptr<base::SingleThreadTaskRunner> task_runner,
    base::WeakPtr<MediaGpuChannelManager> media_gpu_channel_manager);

// Creates a MediaService instance using the TestMojoMediaClient.
std::unique_ptr<service_manager::Service> MEDIA_MOJO_EXPORT
CreateMediaServiceForTesting();

}  // namespace media

#endif  // MEDIA_MOJO_SERVICES_MEDIA_SERVICE_FACTORY_H_
