// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/audio/win/core_audio_util_win.h"

#include <stddef.h>
#include <stdint.h>

#include "base/macros.h"
#include "base/strings/utf_string_conversions.h"
#include "base/synchronization/waitable_event.h"
#include "base/win/scoped_co_mem.h"
#include "base/win/scoped_com_initializer.h"
#include "base/win/scoped_handle.h"
#include "media/audio/audio_device_description.h"
#include "media/audio/audio_unittest_util.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using base::win::ScopedCOMInitializer;

namespace media {

class CoreAudioUtilWinTest : public ::testing::Test {
 protected:
  // The tests must run on a COM thread.
  // If we don't initialize the COM library on a thread before using COM,
  // all function calls will return CO_E_NOTINITIALIZED.
  CoreAudioUtilWinTest() {
    DCHECK(com_init_.succeeded());
  }
  ~CoreAudioUtilWinTest() override {}

  bool DevicesAvailable() {
    return CoreAudioUtil::IsSupported() &&
           CoreAudioUtil::NumberOfActiveDevices(eCapture) > 0 &&
           CoreAudioUtil::NumberOfActiveDevices(eRender) > 0;
  }

  ScopedCOMInitializer com_init_;
};

TEST_F(CoreAudioUtilWinTest, GetDxDiagDetails) {
  ABORT_AUDIO_TEST_IF_NOT(DevicesAvailable());
  std::string name, version;
  ASSERT_TRUE(CoreAudioUtil::GetDxDiagDetails(&name, &version));
  EXPECT_TRUE(!name.empty());
  EXPECT_TRUE(!version.empty());
}

TEST_F(CoreAudioUtilWinTest, NumberOfActiveDevices) {
  ABORT_AUDIO_TEST_IF_NOT(DevicesAvailable());

  int render_devices = CoreAudioUtil::NumberOfActiveDevices(eRender);
  EXPECT_GT(render_devices, 0);
  int capture_devices = CoreAudioUtil::NumberOfActiveDevices(eCapture);
  EXPECT_GT(capture_devices, 0);
  int total_devices = CoreAudioUtil::NumberOfActiveDevices(eAll);
  EXPECT_EQ(total_devices, render_devices + capture_devices);
}

TEST_F(CoreAudioUtilWinTest, CreateDeviceEnumerator) {
  ABORT_AUDIO_TEST_IF_NOT(DevicesAvailable());

  ScopedComPtr<IMMDeviceEnumerator> enumerator =
      CoreAudioUtil::CreateDeviceEnumerator();
  EXPECT_TRUE(enumerator.Get());
}

TEST_F(CoreAudioUtilWinTest, CreateDefaultDevice) {
  ABORT_AUDIO_TEST_IF_NOT(DevicesAvailable());

  struct {
    EDataFlow flow;
    ERole role;
  } data[] = {
    {eRender, eConsole},
    {eRender, eCommunications},
    {eRender, eMultimedia},
    {eCapture, eConsole},
    {eCapture, eCommunications},
    {eCapture, eMultimedia}
  };

  // Create default devices for all flow/role combinations above.
  ScopedComPtr<IMMDevice> audio_device;
  for (size_t i = 0; i < arraysize(data); ++i) {
    audio_device =
        CoreAudioUtil::CreateDefaultDevice(data[i].flow, data[i].role);
    EXPECT_TRUE(audio_device.Get());
    EXPECT_EQ(data[i].flow, CoreAudioUtil::GetDataFlow(audio_device.Get()));
  }

  // Only eRender and eCapture are allowed as flow parameter.
  audio_device = CoreAudioUtil::CreateDefaultDevice(eAll, eConsole);
  EXPECT_FALSE(audio_device.Get());
}

TEST_F(CoreAudioUtilWinTest, CreateDevice) {
  ABORT_AUDIO_TEST_IF_NOT(DevicesAvailable());

  // Get name and ID of default device used for playback.
  ScopedComPtr<IMMDevice> default_render_device =
      CoreAudioUtil::CreateDefaultDevice(eRender, eConsole);
  AudioDeviceName default_render_name;
  EXPECT_TRUE(SUCCEEDED(CoreAudioUtil::GetDeviceName(
      default_render_device.Get(), &default_render_name)));

  // Use the uniqe ID as input to CreateDevice() and create a corresponding
  // IMMDevice.
  ScopedComPtr<IMMDevice> audio_device =
      CoreAudioUtil::CreateDevice(default_render_name.unique_id);
  EXPECT_TRUE(audio_device.Get());

  // Verify that the two IMMDevice interfaces represents the same endpoint
  // by comparing their unique IDs.
  AudioDeviceName device_name;
  EXPECT_TRUE(SUCCEEDED(
      CoreAudioUtil::GetDeviceName(audio_device.Get(), &device_name)));
  EXPECT_EQ(default_render_name.unique_id, device_name.unique_id);
}

TEST_F(CoreAudioUtilWinTest, GetDefaultDeviceName) {
  ABORT_AUDIO_TEST_IF_NOT(DevicesAvailable());

  struct {
    EDataFlow flow;
    ERole role;
  } data[] = {
    {eRender, eConsole},
    {eRender, eCommunications},
    {eCapture, eConsole},
    {eCapture, eCommunications}
  };

  // Get name and ID of default devices for all flow/role combinations above.
  ScopedComPtr<IMMDevice> audio_device;
  AudioDeviceName device_name;
  for (size_t i = 0; i < arraysize(data); ++i) {
    audio_device =
        CoreAudioUtil::CreateDefaultDevice(data[i].flow, data[i].role);
    EXPECT_TRUE(SUCCEEDED(
        CoreAudioUtil::GetDeviceName(audio_device.Get(), &device_name)));
    EXPECT_FALSE(device_name.device_name.empty());
    EXPECT_FALSE(device_name.unique_id.empty());
  }
}

TEST_F(CoreAudioUtilWinTest, GetAudioControllerID) {
  ABORT_AUDIO_TEST_IF_NOT(DevicesAvailable());

  ScopedComPtr<IMMDeviceEnumerator> enumerator(
      CoreAudioUtil::CreateDeviceEnumerator());
  ASSERT_TRUE(enumerator.Get());

  // Enumerate all active input and output devices and fetch the ID of
  // the associated device.
  EDataFlow flows[] = { eRender , eCapture };
  for (size_t i = 0; i < arraysize(flows); ++i) {
    ScopedComPtr<IMMDeviceCollection> collection;
    ASSERT_TRUE(SUCCEEDED(enumerator->EnumAudioEndpoints(
        flows[i], DEVICE_STATE_ACTIVE, collection.GetAddressOf())));
    UINT count = 0;
    collection->GetCount(&count);
    for (UINT j = 0; j < count; ++j) {
      ScopedComPtr<IMMDevice> device;
      collection->Item(j, device.GetAddressOf());
      std::string controller_id(
          CoreAudioUtil::GetAudioControllerID(device.Get(), enumerator.Get()));
      EXPECT_FALSE(controller_id.empty());
    }
  }
}

TEST_F(CoreAudioUtilWinTest, GetFriendlyName) {
  ABORT_AUDIO_TEST_IF_NOT(DevicesAvailable());

  // Get name and ID of default device used for recording.
  ScopedComPtr<IMMDevice> audio_device =
      CoreAudioUtil::CreateDefaultDevice(eCapture, eConsole);
  AudioDeviceName device_name;
  HRESULT hr = CoreAudioUtil::GetDeviceName(audio_device.Get(), &device_name);
  EXPECT_TRUE(SUCCEEDED(hr));

  // Use unique ID as input to GetFriendlyName() and compare the result
  // with the already obtained friendly name for the default capture device.
  std::string friendly_name = CoreAudioUtil::GetFriendlyName(
      device_name.unique_id);
  EXPECT_EQ(friendly_name, device_name.device_name);

  // Same test as above but for playback.
  audio_device = CoreAudioUtil::CreateDefaultDevice(eRender, eConsole);
  hr = CoreAudioUtil::GetDeviceName(audio_device.Get(), &device_name);
  EXPECT_TRUE(SUCCEEDED(hr));
  friendly_name = CoreAudioUtil::GetFriendlyName(device_name.unique_id);
  EXPECT_EQ(friendly_name, device_name.device_name);
}

TEST_F(CoreAudioUtilWinTest, DeviceIsDefault) {
  ABORT_AUDIO_TEST_IF_NOT(DevicesAvailable());

  // Verify that the default render device is correctly identified as a
  // default device.
  ScopedComPtr<IMMDevice> audio_device =
      CoreAudioUtil::CreateDefaultDevice(eRender, eConsole);
  AudioDeviceName name;
  EXPECT_TRUE(
      SUCCEEDED(CoreAudioUtil::GetDeviceName(audio_device.Get(), &name)));
  const std::string id = name.unique_id;
  EXPECT_TRUE(CoreAudioUtil::DeviceIsDefault(eRender, eConsole, id));
  EXPECT_FALSE(CoreAudioUtil::DeviceIsDefault(eCapture, eConsole, id));
}

TEST_F(CoreAudioUtilWinTest, CreateDefaultClient) {
  ABORT_AUDIO_TEST_IF_NOT(DevicesAvailable());

  EDataFlow data[] = {eRender, eCapture};

  for (size_t i = 0; i < arraysize(data); ++i) {
    ScopedComPtr<IAudioClient> client;
    client = CoreAudioUtil::CreateDefaultClient(data[i], eConsole);
    EXPECT_TRUE(client.Get());
  }
}

TEST_F(CoreAudioUtilWinTest, CreateClient) {
  ABORT_AUDIO_TEST_IF_NOT(DevicesAvailable());

  EDataFlow data[] = {eRender, eCapture};

  for (size_t i = 0; i < arraysize(data); ++i) {
    ScopedComPtr<IMMDevice> device;
    ScopedComPtr<IAudioClient> client;
    device = CoreAudioUtil::CreateDefaultDevice(data[i], eConsole);
    EXPECT_TRUE(device.Get());
    EXPECT_EQ(data[i], CoreAudioUtil::GetDataFlow(device.Get()));
    client = CoreAudioUtil::CreateClient(device.Get());
    EXPECT_TRUE(client.Get());
  }
}

TEST_F(CoreAudioUtilWinTest, GetSharedModeMixFormat) {
  ABORT_AUDIO_TEST_IF_NOT(DevicesAvailable());

  ScopedComPtr<IMMDevice> device;
  ScopedComPtr<IAudioClient> client;
  device = CoreAudioUtil::CreateDefaultDevice(eRender, eConsole);
  EXPECT_TRUE(device.Get());
  client = CoreAudioUtil::CreateClient(device.Get());
  EXPECT_TRUE(client.Get());

  // Perform a simple sanity test of the aquired format structure.
  WAVEFORMATPCMEX format;
  EXPECT_TRUE(
      SUCCEEDED(CoreAudioUtil::GetSharedModeMixFormat(client.Get(), &format)));
  EXPECT_GE(format.Format.nChannels, 1);
  EXPECT_GE(format.Format.nSamplesPerSec, 8000u);
  EXPECT_GE(format.Format.wBitsPerSample, 16);
  EXPECT_GE(format.Samples.wValidBitsPerSample, 16);
  EXPECT_EQ(format.Format.wFormatTag, WAVE_FORMAT_EXTENSIBLE);
}

TEST_F(CoreAudioUtilWinTest, IsChannelLayoutSupported) {
  ABORT_AUDIO_TEST_IF_NOT(DevicesAvailable());

  // The preferred channel layout should always be supported. Being supported
  // means that it is possible to initialize a shared mode stream with the
  // particular channel layout.
  AudioParameters mix_params;
  HRESULT hr = CoreAudioUtil::GetPreferredAudioParameters(
      AudioDeviceDescription::kDefaultDeviceId, true, &mix_params);
  EXPECT_TRUE(SUCCEEDED(hr));
  EXPECT_TRUE(mix_params.IsValid());
  EXPECT_TRUE(CoreAudioUtil::IsChannelLayoutSupported(
      std::string(), eRender, eConsole, mix_params.channel_layout()));

  // Check if it is possible to modify the channel layout to stereo for a
  // device which reports that it prefers to be openen up in an other
  // channel configuration.
  if (mix_params.channel_layout() != CHANNEL_LAYOUT_STEREO) {
    ChannelLayout channel_layout = CHANNEL_LAYOUT_STEREO;
    // TODO(henrika): it might be too pessimistic to assume false as return
    // value here.
    EXPECT_FALSE(CoreAudioUtil::IsChannelLayoutSupported(
        std::string(), eRender, eConsole, channel_layout));
  }
}

TEST_F(CoreAudioUtilWinTest, GetDevicePeriod) {
  ABORT_AUDIO_TEST_IF_NOT(DevicesAvailable());

  EDataFlow data[] = {eRender, eCapture};

  // Verify that the device periods are valid for the default render and
  // capture devices.
  for (size_t i = 0; i < arraysize(data); ++i) {
    ScopedComPtr<IAudioClient> client;
    REFERENCE_TIME shared_time_period = 0;
    REFERENCE_TIME exclusive_time_period = 0;
    client = CoreAudioUtil::CreateDefaultClient(data[i], eConsole);
    EXPECT_TRUE(client.Get());
    EXPECT_TRUE(SUCCEEDED(CoreAudioUtil::GetDevicePeriod(
        client.Get(), AUDCLNT_SHAREMODE_SHARED, &shared_time_period)));
    EXPECT_GT(shared_time_period, 0);
    EXPECT_TRUE(SUCCEEDED(CoreAudioUtil::GetDevicePeriod(
        client.Get(), AUDCLNT_SHAREMODE_EXCLUSIVE, &exclusive_time_period)));
    EXPECT_GT(exclusive_time_period, 0);
    EXPECT_LE(exclusive_time_period, shared_time_period);
  }
}

TEST_F(CoreAudioUtilWinTest, GetPreferredAudioParameters) {
  ABORT_AUDIO_TEST_IF_NOT(DevicesAvailable());

  EDataFlow data[] = {eRender, eCapture};

  // Verify that the preferred audio parameters are OK for the default render
  // and capture devices.
  for (size_t i = 0; i < arraysize(data); ++i) {
    ScopedComPtr<IAudioClient> client;
    AudioParameters params;
    client = CoreAudioUtil::CreateDefaultClient(data[i], eConsole);
    EXPECT_TRUE(client.Get());
    EXPECT_TRUE(SUCCEEDED(
        CoreAudioUtil::GetPreferredAudioParameters(client.Get(), &params)));
    EXPECT_TRUE(params.IsValid());
  }
}

TEST_F(CoreAudioUtilWinTest, SharedModeInitialize) {
  ABORT_AUDIO_TEST_IF_NOT(DevicesAvailable());

  ScopedComPtr<IAudioClient> client;
  client = CoreAudioUtil::CreateDefaultClient(eRender, eConsole);
  EXPECT_TRUE(client.Get());

  WAVEFORMATPCMEX format;
  EXPECT_TRUE(
      SUCCEEDED(CoreAudioUtil::GetSharedModeMixFormat(client.Get(), &format)));

  // Perform a shared-mode initialization without event-driven buffer handling.
  uint32_t endpoint_buffer_size = 0;
  HRESULT hr = CoreAudioUtil::SharedModeInitialize(client.Get(), &format, NULL,
                                                   &endpoint_buffer_size, NULL);
  EXPECT_TRUE(SUCCEEDED(hr));
  EXPECT_GT(endpoint_buffer_size, 0u);

  // It is only possible to create a client once.
  hr = CoreAudioUtil::SharedModeInitialize(client.Get(), &format, NULL,
                                           &endpoint_buffer_size, NULL);
  EXPECT_FALSE(SUCCEEDED(hr));
  EXPECT_EQ(hr, AUDCLNT_E_ALREADY_INITIALIZED);

  // Verify that it is possible to reinitialize the client after releasing it.
  client = CoreAudioUtil::CreateDefaultClient(eRender, eConsole);
  EXPECT_TRUE(client.Get());
  hr = CoreAudioUtil::SharedModeInitialize(client.Get(), &format, NULL,
                                           &endpoint_buffer_size, NULL);
  EXPECT_TRUE(SUCCEEDED(hr));
  EXPECT_GT(endpoint_buffer_size, 0u);

  // Use a non-supported format and verify that initialization fails.
  // A simple way to emulate an invalid format is to use the shared-mode
  // mixing format and modify the preferred sample.
  client = CoreAudioUtil::CreateDefaultClient(eRender, eConsole);
  EXPECT_TRUE(client.Get());
  format.Format.nSamplesPerSec = format.Format.nSamplesPerSec + 1;
  EXPECT_FALSE(CoreAudioUtil::IsFormatSupported(
      client.Get(), AUDCLNT_SHAREMODE_SHARED, &format));
  hr = CoreAudioUtil::SharedModeInitialize(client.Get(), &format, NULL,
                                           &endpoint_buffer_size, NULL);
  EXPECT_TRUE(FAILED(hr));
  EXPECT_EQ(hr, E_INVALIDARG);

  // Finally, perform a shared-mode initialization using event-driven buffer
  // handling. The event handle will be signaled when an audio buffer is ready
  // to be processed by the client (not verified here).
  // The event handle should be in the nonsignaled state.
  base::win::ScopedHandle event_handle(::CreateEvent(NULL, TRUE, FALSE, NULL));
  client = CoreAudioUtil::CreateDefaultClient(eRender, eConsole);
  EXPECT_TRUE(client.Get());
  EXPECT_TRUE(
      SUCCEEDED(CoreAudioUtil::GetSharedModeMixFormat(client.Get(), &format)));
  EXPECT_TRUE(CoreAudioUtil::IsFormatSupported(
      client.Get(), AUDCLNT_SHAREMODE_SHARED, &format));
  hr = CoreAudioUtil::SharedModeInitialize(
      client.Get(), &format, event_handle.Get(), &endpoint_buffer_size, NULL);
  EXPECT_TRUE(SUCCEEDED(hr));
  EXPECT_GT(endpoint_buffer_size, 0u);
}

TEST_F(CoreAudioUtilWinTest, CreateRenderAndCaptureClients) {
  ABORT_AUDIO_TEST_IF_NOT(DevicesAvailable());

  EDataFlow data[] = {eRender, eCapture};

  WAVEFORMATPCMEX format;
  uint32_t endpoint_buffer_size = 0;

  for (size_t i = 0; i < arraysize(data); ++i) {
    ScopedComPtr<IAudioClient> client;
    ScopedComPtr<IAudioRenderClient> render_client;
    ScopedComPtr<IAudioCaptureClient> capture_client;

    client = CoreAudioUtil::CreateDefaultClient(data[i], eConsole);
    EXPECT_TRUE(client.Get());
    EXPECT_TRUE(SUCCEEDED(
        CoreAudioUtil::GetSharedModeMixFormat(client.Get(), &format)));
    if (data[i] == eRender) {
      // It is not possible to create a render client using an unitialized
      // client interface.
      render_client = CoreAudioUtil::CreateRenderClient(client.Get());
      EXPECT_FALSE(render_client.Get());

      // Do a proper initialization and verify that it works this time.
      CoreAudioUtil::SharedModeInitialize(client.Get(), &format, NULL,
                                          &endpoint_buffer_size, NULL);
      render_client = CoreAudioUtil::CreateRenderClient(client.Get());
      EXPECT_TRUE(render_client.Get());
      EXPECT_GT(endpoint_buffer_size, 0u);
    } else if (data[i] == eCapture) {
      // It is not possible to create a capture client using an unitialized
      // client interface.
      capture_client = CoreAudioUtil::CreateCaptureClient(client.Get());
      EXPECT_FALSE(capture_client.Get());

      // Do a proper initialization and verify that it works this time.
      CoreAudioUtil::SharedModeInitialize(client.Get(), &format, NULL,
                                          &endpoint_buffer_size, NULL);
      capture_client = CoreAudioUtil::CreateCaptureClient(client.Get());
      EXPECT_TRUE(capture_client.Get());
      EXPECT_GT(endpoint_buffer_size, 0u);
    }
  }
}

TEST_F(CoreAudioUtilWinTest, FillRenderEndpointBufferWithSilence) {
  ABORT_AUDIO_TEST_IF_NOT(DevicesAvailable());

  // Create default clients using the default mixing format for shared mode.
  ScopedComPtr<IAudioClient> client(
      CoreAudioUtil::CreateDefaultClient(eRender, eConsole));
  EXPECT_TRUE(client.Get());

  WAVEFORMATPCMEX format;
  uint32_t endpoint_buffer_size = 0;
  EXPECT_TRUE(
      SUCCEEDED(CoreAudioUtil::GetSharedModeMixFormat(client.Get(), &format)));
  CoreAudioUtil::SharedModeInitialize(client.Get(), &format, NULL,
                                      &endpoint_buffer_size, NULL);
  EXPECT_GT(endpoint_buffer_size, 0u);

  ScopedComPtr<IAudioRenderClient> render_client(
      CoreAudioUtil::CreateRenderClient(client.Get()));
  EXPECT_TRUE(render_client.Get());

  // The endpoint audio buffer should not be filled up by default after being
  // created.
  UINT32 num_queued_frames = 0;
  client->GetCurrentPadding(&num_queued_frames);
  EXPECT_EQ(num_queued_frames, 0u);

  // Fill it up with zeros and verify that the buffer is full.
  // It is not possible to verify that the actual data consists of zeros
  // since we can't access data that has already been sent to the endpoint
  // buffer.
  EXPECT_TRUE(CoreAudioUtil::FillRenderEndpointBufferWithSilence(
      client.Get(), render_client.Get()));
  client->GetCurrentPadding(&num_queued_frames);
  EXPECT_EQ(num_queued_frames, endpoint_buffer_size);
}

// This test can only run on a machine that has audio hardware
// that has both input and output devices.
TEST_F(CoreAudioUtilWinTest, GetMatchingOutputDeviceID) {
  ABORT_AUDIO_TEST_IF_NOT(DevicesAvailable());

  bool found_a_pair = false;

  ScopedComPtr<IMMDeviceEnumerator> enumerator(
      CoreAudioUtil::CreateDeviceEnumerator());
  ASSERT_TRUE(enumerator.Get());

  // Enumerate all active input and output devices and fetch the ID of
  // the associated device.
  ScopedComPtr<IMMDeviceCollection> collection;
  ASSERT_TRUE(SUCCEEDED(enumerator->EnumAudioEndpoints(
      eCapture, DEVICE_STATE_ACTIVE, collection.GetAddressOf())));
  UINT count = 0;
  collection->GetCount(&count);
  for (UINT i = 0; i < count && !found_a_pair; ++i) {
    ScopedComPtr<IMMDevice> device;
    collection->Item(i, device.GetAddressOf());
    base::win::ScopedCoMem<WCHAR> wide_id;
    device->GetId(&wide_id);
    std::string id;
    base::WideToUTF8(wide_id, wcslen(wide_id), &id);
    found_a_pair = !CoreAudioUtil::GetMatchingOutputDeviceID(id).empty();
  }

  EXPECT_TRUE(found_a_pair);
}

TEST_F(CoreAudioUtilWinTest, GetDefaultOutputDeviceID) {
  ABORT_AUDIO_TEST_IF_NOT(DevicesAvailable());

  std::string default_device_id(CoreAudioUtil::GetDefaultOutputDeviceID());
  EXPECT_FALSE(default_device_id.empty());
}

}  // namespace media
