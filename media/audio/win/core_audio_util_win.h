// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Utility methods for the Core Audio API on Windows.
// Always ensure that Core Audio is supported before using these methods.
// Use media::CoreAudioUtil::IsSupported() for this purpose.
// Also, all methods must be called on a valid COM thread. This can be done
// by using the base::win::ScopedCOMInitializer helper class.

#ifndef MEDIA_AUDIO_WIN_CORE_AUDIO_UTIL_WIN_H_
#define MEDIA_AUDIO_WIN_CORE_AUDIO_UTIL_WIN_H_

#include <audioclient.h>
#include <mmdeviceapi.h>
#include <stdint.h>
#include <string>

#include "base/macros.h"
#include "base/time/time.h"
#include "base/win/scoped_comptr.h"
#include "media/audio/audio_device_name.h"
#include "media/base/audio_parameters.h"
#include "media/base/media_export.h"

using base::win::ScopedComPtr;

namespace media {


// Represents audio channel configuration constants as understood by Windows.
// E.g. KSAUDIO_SPEAKER_MONO.  For a list of possible values see:
// http://msdn.microsoft.com/en-us/library/windows/hardware/ff537083(v=vs.85).aspx
typedef uint32_t ChannelConfig;

class MEDIA_EXPORT CoreAudioUtil {
 public:
  // Returns true if Windows Core Audio is supported.
  // Always verify that this method returns true before using any of the
  // methods in this class.
  // WARNING: This function must be called once from the main thread before
  // it is safe to call from other threads.
  static bool IsSupported();

  // Converts between reference time to base::TimeDelta.
  // One reference-time unit is 100 nanoseconds.
  // Example: double s = RefererenceTimeToTimeDelta(t).InMillisecondsF();
  static base::TimeDelta RefererenceTimeToTimeDelta(REFERENCE_TIME time);

  // Returns AUDCLNT_SHAREMODE_EXCLUSIVE if --enable-exclusive-mode is used
  // as command-line flag and AUDCLNT_SHAREMODE_SHARED otherwise (default).
  static AUDCLNT_SHAREMODE GetShareMode();

  // The Windows Multimedia Device (MMDevice) API enables audio clients to
  // discover audio endpoint devices and determine their capabilities.

  // Number of active audio devices in the specified flow data flow direction.
  // Set |data_flow| to eAll to retrieve the total number of active audio
  // devices.
  static int NumberOfActiveDevices(EDataFlow data_flow);

  // Creates an IMMDeviceEnumerator interface which provides methods for
  // enumerating audio endpoint devices.
  static ScopedComPtr<IMMDeviceEnumerator> CreateDeviceEnumerator();

  // Creates a default endpoint device that is specified by a data-flow
  // direction and role, e.g. default render device.
  static ScopedComPtr<IMMDevice> CreateDefaultDevice(
      EDataFlow data_flow, ERole role);

  // Returns the device id of the default output device or an empty string
  // if no such device exists or if the default device has been disabled.
  static std::string GetDefaultOutputDeviceID();

  // Creates an endpoint device that is specified by a unique endpoint device-
  // identification string.
  static ScopedComPtr<IMMDevice> CreateDevice(const std::string& device_id);

  // Returns the unique ID and user-friendly name of a given endpoint device.
  // Example: "{0.0.1.00000000}.{8db6020f-18e3-4f25-b6f5-7726c9122574}", and
  //          "Microphone (Realtek High Definition Audio)".
  static HRESULT GetDeviceName(IMMDevice* device, AudioDeviceName* name);

  // Returns the device ID/path of the controller (a.k.a. physical device that
  // |device| is connected to.  This ID will be the same for all devices from
  // the same controller so it is useful for doing things like determining
  // whether a set of output and input devices belong to the same controller.
  // The device enumerator is required as well as the device itself since
  // looking at the device topology is required and we need to open up
  // associated devices to determine the controller id.
  // If the ID could not be determined for some reason, an empty string is
  // returned.
  static std::string GetAudioControllerID(IMMDevice* device,
      IMMDeviceEnumerator* enumerator);

  // Accepts an id of an input device and finds a matching output device id.
  // If the associated hardware does not have an audio output device (e.g.
  // a webcam with a mic), an empty string is returned.
  static std::string GetMatchingOutputDeviceID(
      const std::string& input_device_id);

  // Gets the user-friendly name of the endpoint device which is represented
  // by a unique id in |device_id|.
  static std::string GetFriendlyName(const std::string& device_id);

  // Returns true if the provided unique |device_id| corresponds to the current
  // default device for the specified by a data-flow direction and role.
  static bool DeviceIsDefault(
      EDataFlow flow, ERole role, const std::string& device_id);

  // Query if the audio device is a rendering device or a capture device.
  static EDataFlow GetDataFlow(IMMDevice* device);

  // The Windows Audio Session API (WASAPI) enables client applications to
  // manage the flow of audio data between the application and an audio endpoint
  // device.

  // Create an IAudioClient instance for the default IMMDevice where
  // flow direction and role is define by |data_flow| and |role|.
  // The IAudioClient interface enables a client to create and initialize an
  // audio stream between an audio application and the audio engine (for a
  // shared-mode stream) or the hardware buffer of an audio endpoint device
  // (for an exclusive-mode stream).
  static ScopedComPtr<IAudioClient> CreateDefaultClient(EDataFlow data_flow,
                                                        ERole role);

  // Create an IAudioClient instance for a specific device _or_ the default
  // device if |device_id| is empty.
  static ScopedComPtr<IAudioClient> CreateClient(const std::string& device_id,
                                                 EDataFlow data_flow,
                                                 ERole role);

  // Create an IAudioClient interface for an existing IMMDevice given by
  // |audio_device|. Flow direction and role is define by the |audio_device|.
  static ScopedComPtr<IAudioClient> CreateClient(IMMDevice* audio_device);

  // Get the mix format that the audio engine uses internally for processing
  // of shared-mode streams. This format is not necessarily a format that the
  // audio endpoint device supports. Thus, the caller might not succeed in
  // creating an exclusive-mode stream with a format obtained by this method.
  static HRESULT GetSharedModeMixFormat(IAudioClient* client,
                                        WAVEFORMATPCMEX* format);

  // Returns true if the specified |client| supports the format in |format|
  // for the given |share_mode| (shared or exclusive).
  static bool IsFormatSupported(IAudioClient* client,
                                AUDCLNT_SHAREMODE share_mode,
                                const WAVEFORMATPCMEX* format);

  // Returns true if the specified |channel_layout| is supported for the
  // default IMMDevice where flow direction and role is define by |data_flow|
  // and |role|. If this method returns true for a certain channel layout, it
  // means that SharedModeInitialize() will succeed using a format based on
  // the preferred format where the channel layout has been modified.
  static bool IsChannelLayoutSupported(const std::string& device_id,
                                       EDataFlow data_flow,
                                       ERole role,
                                       ChannelLayout channel_layout);

  // For a shared-mode stream, the audio engine periodically processes the
  // data in the endpoint buffer at the period obtained in |device_period|.
  // For an exclusive mode stream, |device_period| corresponds to the minimum
  // time interval between successive processing by the endpoint device.
  // This period plus the stream latency between the buffer and endpoint device
  // represents the minimum possible latency that an audio application can
  // achieve. The time in |device_period| is expressed in 100-nanosecond units.
  static HRESULT GetDevicePeriod(IAudioClient* client,
                                 AUDCLNT_SHAREMODE share_mode,
                                 REFERENCE_TIME* device_period);

  // Get the preferred audio parameters for the specified |client| or the
  // given direction and role is define by |data_flow| and |role|, or the
  // unique device id given by |device_id|.
  // The acquired values should only be utilized for shared mode streamed since
  // there are no preferred settings for an exclusive mode stream.
  static HRESULT GetPreferredAudioParameters(IAudioClient* client,
                                             AudioParameters* params);
  static HRESULT GetPreferredAudioParameters(const std::string& device_id,
                                             bool is_output_device,
                                             AudioParameters* params);

  // Retrieves an integer mask which corresponds to the channel layout the
  // audio engine uses for its internal processing/mixing of shared-mode
  // streams. This mask indicates which channels are present in the multi-
  // channel stream. The least significant bit corresponds with the Front Left
  // speaker, the next least significant bit corresponds to the Front Right
  // speaker, and so on, continuing in the order defined in KsMedia.h.
  // See http://msdn.microsoft.com/en-us/library/windows/hardware/ff537083(v=vs.85).aspx
  // for more details.
  // To get the channel config of the default device, pass an empty string
  // for |device_id|.
  static ChannelConfig GetChannelConfig(const std::string& device_id,
                                        EDataFlow data_flow);

  // After activating an IAudioClient interface on an audio endpoint device,
  // the client must initialize it once, and only once, to initialize the audio
  // stream between the client and the device. In shared mode, the client
  // connects indirectly through the audio engine which does the mixing.
  // In exclusive mode, the client connects directly to the audio hardware.
  // If a valid event is provided in |event_handle|, the client will be
  // initialized for event-driven buffer handling. If |event_handle| is set to
  // NULL, event-driven buffer handling is not utilized.
  // This function will initialize the audio client as part of the default
  // audio session if NULL is passed for |session_guid|, otherwise the client
  // will be associated with the specified session.
  static HRESULT SharedModeInitialize(IAudioClient* client,
                                      const WAVEFORMATPCMEX* format,
                                      HANDLE event_handle,
                                      uint32_t* endpoint_buffer_size,
                                      const GUID* session_guid);

  // TODO(henrika): add ExclusiveModeInitialize(...)

  // Create an IAudioRenderClient client for an existing IAudioClient given by
  // |client|. The IAudioRenderClient interface enables a client to write
  // output data to a rendering endpoint buffer.
  static ScopedComPtr<IAudioRenderClient> CreateRenderClient(
      IAudioClient* client);

  // Create an IAudioCaptureClient client for an existing IAudioClient given by
  // |client|. The IAudioCaptureClient interface enables a client to read
  // input data from a capture endpoint buffer.
  static ScopedComPtr<IAudioCaptureClient> CreateCaptureClient(
      IAudioClient* client);

  // Fills up the endpoint rendering buffer with silence for an existing
  // IAudioClient given by |client| and a corresponding IAudioRenderClient
  // given by |render_client|.
  static bool FillRenderEndpointBufferWithSilence(
      IAudioClient* client, IAudioRenderClient* render_client);

  // Returns the default audio driver file name and version string according to
  // DxDiag.  Used for crash reporting.  Can be slow (~seconds).
  static bool GetDxDiagDetails(std::string* driver_name,
                               std::string* driver_version);

 private:
  CoreAudioUtil() {}
  ~CoreAudioUtil() {}
  DISALLOW_COPY_AND_ASSIGN(CoreAudioUtil);
};

// The special audio session identifier we use when opening up the default
// communication device.  This has the effect that a separate volume control
// will be shown in the system's volume mixer and control over ducking and
// visually observing the behavior of ducking, is easier.
// Use with |SharedModeInitialize|.
extern const GUID kCommunicationsSessionId;

}  // namespace media

#endif  // MEDIA_AUDIO_WIN_CORE_AUDIO_UTIL_WIN_H_
