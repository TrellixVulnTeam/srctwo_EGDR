// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/metrics/gpu/gpu_metrics_provider.h"

#include "components/metrics/proto/system_profile.pb.h"
#include "content/public/browser/gpu_data_manager.h"
#include "gpu/config/gpu_info.h"

namespace metrics {

GPUMetricsProvider::GPUMetricsProvider() {
}

GPUMetricsProvider::~GPUMetricsProvider() {
}

void GPUMetricsProvider::ProvideSystemProfileMetrics(
    SystemProfileProto* system_profile_proto) {
  SystemProfileProto::Hardware* hardware =
      system_profile_proto->mutable_hardware();

  const gpu::GPUInfo& gpu_info =
      content::GpuDataManager::GetInstance()->GetGPUInfo();
  SystemProfileProto::Hardware::Graphics* gpu =
      hardware->mutable_gpu();
  gpu->set_vendor_id(gpu_info.gpu.vendor_id);
  gpu->set_device_id(gpu_info.gpu.device_id);
  gpu->set_driver_version(gpu_info.driver_version);
  gpu->set_driver_date(gpu_info.driver_date);
  gpu->set_gl_vendor(gpu_info.gl_vendor);
  gpu->set_gl_renderer(gpu_info.gl_renderer);
}

}  // namespace metrics
