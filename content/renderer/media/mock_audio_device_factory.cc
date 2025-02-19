// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/media/mock_audio_device_factory.h"

namespace content {

MockCapturerSource::MockCapturerSource() {}

MockCapturerSource::~MockCapturerSource() {}

void MockCapturerSource::SetVolume(double volume) {}

MockAudioDeviceFactory::MockAudioDeviceFactory()
    : AudioDeviceFactory(), mock_capturer_source_(new MockCapturerSource()),
      did_create_once_(false) {}

MockAudioDeviceFactory::~MockAudioDeviceFactory() {}

scoped_refptr<media::AudioCapturerSource>
MockAudioDeviceFactory::CreateAudioCapturerSource(int render_frame_id) {
  CHECK(!did_create_once_);
  did_create_once_ = true;
  return scoped_refptr<media::AudioCapturerSource>(mock_capturer_source_);
}

}  // namespace content
