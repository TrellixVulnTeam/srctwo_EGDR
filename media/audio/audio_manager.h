// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_AUDIO_AUDIO_MANAGER_H_
#define MEDIA_AUDIO_AUDIO_MANAGER_H_

#include <memory>
#include <string>

#include "base/callback_forward.h"
#include "base/gtest_prod_util.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/strings/string16.h"
#include "base/threading/thread_checker.h"
#include "build/build_config.h"
#include "media/audio/audio_device_description.h"
#include "media/audio/audio_logging.h"
#include "media/audio/audio_thread.h"
#include "media/base/audio_parameters.h"

namespace base {
class FilePath;
class SingleThreadTaskRunner;
}

namespace media {

class AudioInputStream;
class AudioManager;
class AudioOutputStream;

// Manages all audio resources.  Provides some convenience functions that avoid
// the need to provide iterators over the existing streams.
//
// Except on OSX, a hang monitor for the audio thread is always created. When a
// thread hang is detected, it is reported to UMA.
class MEDIA_EXPORT AudioManager {
 public:
  virtual ~AudioManager();

  // Construct the audio manager; only one instance is allowed.
  //
  // The manager will forward CreateAudioLog() calls to the provided
  // AudioLogFactory; as such |audio_log_factory| must outlive the AudioManager.
  //
  // The manager will use |audio_thread->GetTaskRunner()| for audio IO.
  // On OS_MACOSX, CoreAudio requires that |audio_thread->GetTaskRunner()|
  // must belong to the main thread of the process, which in our case is sadly
  // the browser UI thread. Failure to execute calls on the right thread leads
  // to crashes and odd behavior. See http://crbug.com/158170.
  //
  // The manager will use |audio_thread->GetWorkerTaskRunner()| for heavyweight
  // tasks. The |audio_thread->GetWorkerTaskRunner()| may be the same as
  // |audio_thread->GetTaskRunner()|.
  static std::unique_ptr<AudioManager> Create(
      std::unique_ptr<AudioThread> audio_thread,
      AudioLogFactory* audio_log_factory);

  // A convenience wrapper of AudioManager::Create for testing.
  static std::unique_ptr<AudioManager> CreateForTesting(
      std::unique_ptr<AudioThread> audio_thread);

  // Starts monitoring AudioManager task runner for hangs.
  // Runs the monitor on the given |task_runner|, which must be different from
  // AudioManager::GetTaskRunner to be meaningful.
  // This must be called only after an AudioManager instance is created.
  static void StartHangMonitorIfNeeded(
      scoped_refptr<base::SingleThreadTaskRunner> task_runner);

#if defined(OS_LINUX)
  // Sets the name of the audio source as seen by external apps. Only actually
  // used with PulseAudio as of this writing.
  static void SetGlobalAppName(const std::string& app_name);

  // Returns the app name or an empty string if it is not set.
  static const std::string& GetGlobalAppName();
#endif

  // Returns the pointer to the last created instance, or NULL if not yet
  // created. This is a utility method for the code outside of media directory,
  // like src/chrome.
  static AudioManager* Get();

  // Synchronously releases all audio resources.
  // Must be called before deletion and on the same thread as AudioManager
  // was created.
  // Returns true on success but false if AudioManager could not be shutdown.
  // AudioManager instance must not be deleted if shutdown failed.
  virtual bool Shutdown();

  // Log callback used for sending log messages from a stream to the object
  // that manages the stream.
  using LogCallback = base::Callback<void(const std::string&)>;

  // Factory for all the supported stream formats. |params| defines parameters
  // of the audio stream to be created.
  //
  // |params.sample_per_packet| is the requested buffer allocation which the
  // audio source thinks it can usually fill without blocking. Internally two
  // or three buffers are created, one will be locked for playback and one will
  // be ready to be filled in the call to AudioSourceCallback::OnMoreData().
  //
  // To create a stream for the default output device, pass an empty string
  // for |device_id|, otherwise the specified audio device will be opened.
  //
  // Returns NULL if the combination of the parameters is not supported, or if
  // we have reached some other platform specific limit.
  //
  // |params.format| can be set to AUDIO_PCM_LOW_LATENCY and that has two
  // effects:
  // 1- Instead of triple buffered the audio will be double buffered.
  // 2- A low latency driver or alternative audio subsystem will be used when
  //    available.
  //
  // Do not free the returned AudioOutputStream. It is owned by AudioManager.
  virtual AudioOutputStream* MakeAudioOutputStream(
      const AudioParameters& params,
      const std::string& device_id,
      const LogCallback& log_callback) = 0;

  // Creates new audio output proxy. A proxy implements
  // AudioOutputStream interface, but unlike regular output stream
  // created with MakeAudioOutputStream() it opens device only when a
  // sound is actually playing.
  virtual AudioOutputStream* MakeAudioOutputStreamProxy(
      const AudioParameters& params,
      const std::string& device_id) = 0;

  // Factory to create audio recording streams.
  // |channels| can be 1 or 2.
  // |sample_rate| is in hertz and can be any value supported by the platform.
  // |bits_per_sample| can be any value supported by the platform.
  // |samples_per_packet| is in hertz as well and can be 0 to |sample_rate|,
  // with 0 suggesting that the implementation use a default value for that
  // platform.
  // Returns NULL if the combination of the parameters is not supported, or if
  // we have reached some other platform specific limit.
  //
  // Do not free the returned AudioInputStream. It is owned by AudioManager.
  // When you are done with it, call |Stop()| and |Close()| to release it.
  virtual AudioInputStream* MakeAudioInputStream(
      const AudioParameters& params,
      const std::string& device_id,
      const LogCallback& log_callback) = 0;

  // Returns the task runner used for audio IO.
  base::SingleThreadTaskRunner* GetTaskRunner() const {
    return audio_thread_->GetTaskRunner();
  }

  // Heavyweight tasks should use GetWorkerTaskRunner() instead of
  // GetTaskRunner(). On most platforms they are the same, but some share the
  // UI loop with the audio IO loop.
  base::SingleThreadTaskRunner* GetWorkerTaskRunner() const {
    return audio_thread_->GetWorkerTaskRunner();
  }

  // Allows clients to listen for device state changes; e.g. preferred sample
  // rate or channel layout changes.  The typical response to receiving this
  // callback is to recreate the stream.
  class AudioDeviceListener {
   public:
    virtual void OnDeviceChange() = 0;
  };

  virtual void AddOutputDeviceChangeListener(AudioDeviceListener* listener) = 0;
  virtual void RemoveOutputDeviceChangeListener(
      AudioDeviceListener* listener) = 0;

  // Create a new AudioLog object for tracking the behavior for one or more
  // instances of the given component.  See AudioLogFactory for more details.
  virtual std::unique_ptr<AudioLog> CreateAudioLog(
      AudioLogFactory::AudioComponent component) = 0;

  // Enable output debug recording. InitializeOutputDebugRecording() must be
  // called before this function.
  // TODO(grunell): Control input debug recording via these functions too.
  virtual void EnableOutputDebugRecording(
      const base::FilePath& base_file_name) = 0;

  // Disable output debug recording.
  virtual void DisableOutputDebugRecording() = 0;

  // Gets the name of the audio manager (e.g., Windows, Mac, PulseAudio).
  virtual const char* GetName() = 0;

  // Limits the number of streams that can be created for testing purposes.
  virtual void SetMaxStreamCountForTesting(int max_input, int max_output);

 protected:
  FRIEND_TEST_ALL_PREFIXES(AudioManagerTest, AudioDebugRecording);
  friend class AudioDeviceInfoAccessorForTests;

  explicit AudioManager(std::unique_ptr<AudioThread> audio_thread);

  virtual void ShutdownOnAudioThread() = 0;

  // Initializes output debug recording. Can be called on any thread; will post
  // to the audio thread if not called on it.
  virtual void InitializeOutputDebugRecording() = 0;

  // Returns true if the OS reports existence of audio devices. This does not
  // guarantee that the existing devices support all formats and sample rates.
  virtual bool HasAudioOutputDevices() = 0;

  // Returns true if the OS reports existence of audio recording devices. This
  // does not guarantee that the existing devices support all formats and
  // sample rates.
  virtual bool HasAudioInputDevices() = 0;

  // Returns a human readable string for the model/make of the active audio
  // input device for this computer.
  virtual base::string16 GetAudioInputDeviceModel() = 0;

  // Appends a list of available input devices to |device_descriptions|,
  // which must initially be empty. It is not guaranteed that all the
  // devices in the list support all formats and sample rates for
  // recording.
  //
  // Not threadsafe; in production this should only be called from the
  // Audio worker thread (see GetTaskRunner()).
  virtual void GetAudioInputDeviceDescriptions(
      AudioDeviceDescriptions* device_descriptions) = 0;

  // Appends a list of available output devices to |device_descriptions|,
  // which must initially be empty.
  //
  // Not threadsafe; in production this should only be called from the
  // Audio worker thread (see GetTaskRunner()).
  virtual void GetAudioOutputDeviceDescriptions(
      AudioDeviceDescriptions* device_descriptions) = 0;

  // Returns the default output hardware audio parameters for opening output
  // streams. It is a convenience interface to
  // AudioManagerBase::GetPreferredOutputStreamParameters and each AudioManager
  // does not need their own implementation to this interface.
  // TODO(tommi): Remove this method and use GetOutputStreamParameteres instead.
  virtual AudioParameters GetDefaultOutputStreamParameters() = 0;

  // Returns the output hardware audio parameters for a specific output device.
  virtual AudioParameters GetOutputStreamParameters(
      const std::string& device_id) = 0;

  // Returns the input hardware audio parameters of the specific device
  // for opening input streams. Each AudioManager needs to implement their own
  // version of this interface.
  virtual AudioParameters GetInputStreamParameters(
      const std::string& device_id) = 0;

  // Returns the device id of an output device that belongs to the same hardware
  // as the specified input device.
  // If the hardware has only an input device (e.g. a webcam), the return value
  // will be empty (which the caller can then interpret to be the default output
  // device).  Implementations that don't yet support this feature, must return
  // an empty string. Must be called on the audio worker thread (see
  // GetTaskRunner()).
  virtual std::string GetAssociatedOutputDeviceID(
      const std::string& input_device_id) = 0;

 private:
  friend class AudioSystemHelper;

  std::unique_ptr<AudioThread> audio_thread_;
  bool shutdown_ = false;  // True after |this| has been shutdown.

  THREAD_CHECKER(thread_checker_);
  DISALLOW_COPY_AND_ASSIGN(AudioManager);
};

}  // namespace media

#endif  // MEDIA_AUDIO_AUDIO_MANAGER_H_
