// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_AUDIO_AUDIO_SYSTEM_IMPL_H_
#define MEDIA_AUDIO_AUDIO_SYSTEM_IMPL_H_

#include "media/audio/audio_system.h"
#include "media/audio/audio_system_helper.h"

namespace media {
class AudioManager;

class MEDIA_EXPORT AudioSystemImpl : public AudioSystem {
 public:
  static std::unique_ptr<AudioSystem> Create(AudioManager* audio_manager);

  ~AudioSystemImpl() override;

  // AudioSystem implementation.
  void GetInputStreamParameters(const std::string& device_id,
                                OnAudioParamsCallback on_params_cb) override;

  void GetOutputStreamParameters(const std::string& device_id,
                                 OnAudioParamsCallback on_params_cb) override;

  void HasInputDevices(OnBoolCallback on_has_devices_cb) override;

  void HasOutputDevices(OnBoolCallback on_has_devices_cb) override;

  void GetDeviceDescriptions(
      bool for_input,
      OnDeviceDescriptionsCallback on_descriptions_cp) override;

  void GetAssociatedOutputDeviceID(const std::string& input_device_id,
                                   OnDeviceIdCallback on_device_id_cb) override;

  void GetInputDeviceInfo(
      const std::string& input_device_id,
      OnInputDeviceInfoCallback on_input_device_info_cb) override;

 private:
  AudioSystemImpl(AudioManager* audio_manager);

  // No-op if called on helper_.GetTaskRunner() thread, otherwise binds
  // |callback| to the current loop.
  template <typename... Args>
  base::OnceCallback<void(Args...)> MaybeBindToCurrentLoop(
      base::OnceCallback<void(Args...)> callback);

  AudioSystemHelper helper_;

  DISALLOW_COPY_AND_ASSIGN(AudioSystemImpl);
};

}  // namespace media

#endif  // MEDIA_AUDIO_AUDIO_SYSTEM_IMPL_H_
