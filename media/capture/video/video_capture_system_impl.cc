// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/capture/video/video_capture_system_impl.h"

#include "media/base/bind_to_current_loop.h"

namespace {

// Compares two VideoCaptureFormat by checking smallest frame_size area, then
// by width, and then by _largest_ frame_rate. Used to order a
// VideoCaptureFormats vector so that the first entry for a given resolution has
// the largest frame rate.
bool IsCaptureFormatSmaller(const media::VideoCaptureFormat& format1,
                            const media::VideoCaptureFormat& format2) {
  DCHECK(format1.frame_size.GetCheckedArea().IsValid());
  DCHECK(format2.frame_size.GetCheckedArea().IsValid());
  if (format1.frame_size.GetCheckedArea().ValueOrDefault(0) ==
      format2.frame_size.GetCheckedArea().ValueOrDefault(0)) {
    if (format1.frame_size.width() == format2.frame_size.width()) {
      return format1.frame_rate > format2.frame_rate;
    }
    return format1.frame_size.width() > format2.frame_size.width();
  }
  return format1.frame_size.GetCheckedArea().ValueOrDefault(0) <
         format2.frame_size.GetCheckedArea().ValueOrDefault(0);
}

bool IsCaptureFormatEqual(const media::VideoCaptureFormat& format1,
                          const media::VideoCaptureFormat& format2) {
  return format1.frame_size == format2.frame_size &&
         format1.frame_rate == format2.frame_rate &&
         format1.pixel_format == format2.pixel_format;
}

// This function receives a list of capture formats, sets all of them to I420
// (while keeping Y16 as is), and then removes duplicates.
void ConsolidateCaptureFormats(media::VideoCaptureFormats* formats) {
  if (formats->empty())
    return;
  // Mark all formats as I420, since this is what the renderer side will get
  // anyhow: the actual pixel format is decided at the device level.
  // Don't do this for Y16 format as it is handled separatelly.
  for (auto& format : *formats) {
    if (format.pixel_format != media::PIXEL_FORMAT_Y16)
      format.pixel_format = media::PIXEL_FORMAT_I420;
  }
  std::sort(formats->begin(), formats->end(), IsCaptureFormatSmaller);
  // Remove duplicates
  media::VideoCaptureFormats::iterator last =
      std::unique(formats->begin(), formats->end(), IsCaptureFormatEqual);
  formats->erase(last, formats->end());
}

}  // anonymous namespace

namespace media {

VideoCaptureSystemImpl::VideoCaptureSystemImpl(
    std::unique_ptr<VideoCaptureDeviceFactory> factory)
    : factory_(std::move(factory)) {
  thread_checker_.DetachFromThread();
}

VideoCaptureSystemImpl::~VideoCaptureSystemImpl() = default;

void VideoCaptureSystemImpl::GetDeviceInfosAsync(
    DeviceInfoCallback result_callback) {
  DCHECK(thread_checker_.CalledOnValidThread());
  VideoCaptureDeviceDescriptors descriptors;
  factory_->GetDeviceDescriptors(&descriptors);
  // For devices for which we already have an entry in |devices_info_cache_|,
  // we do not want to query the |factory_| for supported formats again. We
  // simply copy them from |devices_info_cache_|.
  std::vector<VideoCaptureDeviceInfo> new_devices_info_cache;
  new_devices_info_cache.reserve(descriptors.size());
  for (const auto& descriptor : descriptors) {
    if (auto* cached_info = LookupDeviceInfoFromId(descriptor.device_id)) {
      new_devices_info_cache.push_back(*cached_info);
    } else {
      // Query for supported formats in order to create the entry.
      VideoCaptureDeviceInfo device_info(descriptor);
      factory_->GetSupportedFormats(descriptor, &device_info.supported_formats);
      ConsolidateCaptureFormats(&device_info.supported_formats);
      new_devices_info_cache.push_back(device_info);
    }
  }

  devices_info_cache_.swap(new_devices_info_cache);
  base::ResetAndReturn(&result_callback).Run(devices_info_cache_);
}

// Creates a VideoCaptureDevice object. Returns NULL if something goes wrong.
std::unique_ptr<VideoCaptureDevice> VideoCaptureSystemImpl::CreateDevice(
    const std::string& device_id) {
  DCHECK(thread_checker_.CalledOnValidThread());
  const VideoCaptureDeviceInfo* device_info = LookupDeviceInfoFromId(device_id);
  if (!device_info)
    return nullptr;
  return factory_->CreateDevice(device_info->descriptor);
}

const VideoCaptureDeviceInfo* VideoCaptureSystemImpl::LookupDeviceInfoFromId(
    const std::string& device_id) {
  DCHECK(thread_checker_.CalledOnValidThread());
  auto iter = std::find_if(
      devices_info_cache_.begin(), devices_info_cache_.end(),
      [&device_id](const media::VideoCaptureDeviceInfo& device_info) {
        return device_info.descriptor.device_id == device_id;
      });
  if (iter == devices_info_cache_.end())
    return nullptr;
  return &(*iter);
}

}  // namespace media
