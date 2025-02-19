// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/protocol/webrtc_audio_module.h"

#include "base/bind.h"
#include "base/memory/ptr_util.h"
#include "base/single_thread_task_runner.h"
#include "base/stl_util.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/timer/timer.h"

namespace remoting {
namespace protocol {

namespace {

const int kSamplingRate = 48000;

// Webrtc uses 10ms frames.
const int kFrameLengthMs = 10;
const int kSamplesPerFrame = kSamplingRate * kFrameLengthMs / 1000;

constexpr base::TimeDelta kPollInterval =
    base::TimeDelta::FromMilliseconds(5 * kFrameLengthMs);
const int kChannels = 2;
const int kBytesPerSample = 2;

}  // namespace

// webrtc::AudioDeviceModule is a generic interface that aims to provide all
// functionality normally supported audio input/output devices, but most of
// the functions are never called in Webrtc. This class implements only
// functions that are actually used. All unused functions are marked as
// NOTREACHED().

WebrtcAudioModule::WebrtcAudioModule() {}
WebrtcAudioModule::~WebrtcAudioModule() {}

void WebrtcAudioModule::SetAudioTaskRunner(
    scoped_refptr<base::SingleThreadTaskRunner> audio_task_runner) {
  DCHECK(!audio_task_runner_);
  DCHECK(audio_task_runner);
  audio_task_runner_ = audio_task_runner;
}

int64_t WebrtcAudioModule::TimeUntilNextProcess() {
  // We don't need to do anything in Process(), so return an arbitrary value
  // that's not too low, so that Process() doesn't get called too frequently.
  return 1000000;
}

void WebrtcAudioModule::Process() {}

int32_t WebrtcAudioModule::ActiveAudioLayer(AudioLayer* audio_layer) const {
  NOTREACHED();
  return -1;
}

WebrtcAudioModule::ErrorCode WebrtcAudioModule::LastError() const {
  return kAdmErrNone;
}

int32_t WebrtcAudioModule::RegisterEventObserver(
    webrtc::AudioDeviceObserver* event_callback) {
  return 0;
}

int32_t WebrtcAudioModule::RegisterAudioCallback(
    webrtc::AudioTransport* audio_transport) {
  base::AutoLock lock(lock_);
  audio_transport_ = audio_transport;
  return 0;
}

int32_t WebrtcAudioModule::Init() {
  base::AutoLock auto_lock(lock_);
  initialized_ = true;
  return 0;
}

int32_t WebrtcAudioModule::Terminate() {
  base::AutoLock auto_lock(lock_);
  initialized_ = false;
  return 0;
}

bool WebrtcAudioModule::Initialized() const {
  base::AutoLock auto_lock(lock_);
  return initialized_;
}

int16_t WebrtcAudioModule::PlayoutDevices() {
  return 0;
}

int16_t WebrtcAudioModule::RecordingDevices() {
  return 0;
}

int32_t WebrtcAudioModule::PlayoutDeviceName(
    uint16_t index,
    char name[webrtc::kAdmMaxDeviceNameSize],
    char guid[webrtc::kAdmMaxGuidSize]) {
  return 0;
}

int32_t WebrtcAudioModule::RecordingDeviceName(
    uint16_t index,
    char name[webrtc::kAdmMaxDeviceNameSize],
    char guid[webrtc::kAdmMaxGuidSize]) {
  return 0;
}

int32_t WebrtcAudioModule::SetPlayoutDevice(uint16_t index) {
  return 0;
}

int32_t WebrtcAudioModule::SetPlayoutDevice(WindowsDeviceType device) {
  return 0;
}

int32_t WebrtcAudioModule::SetRecordingDevice(uint16_t index) {
  return 0;
}

int32_t WebrtcAudioModule::SetRecordingDevice(WindowsDeviceType device) {
  return 0;
}

int32_t WebrtcAudioModule::PlayoutIsAvailable(bool* available) {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::InitPlayout() {
  return 0;
}

bool WebrtcAudioModule::PlayoutIsInitialized() const {
  base::AutoLock auto_lock(lock_);
  return initialized_;
}

int32_t WebrtcAudioModule::RecordingIsAvailable(bool* available) {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::InitRecording() {
  return 0;
}

bool WebrtcAudioModule::RecordingIsInitialized() const {
  return false;
}

int32_t WebrtcAudioModule::StartPlayout() {
  base::AutoLock auto_lock(lock_);
  if (!playing_ && audio_task_runner_) {
    audio_task_runner_->PostTask(
        FROM_HERE,
        base::Bind(&WebrtcAudioModule::StartPlayoutOnAudioThread, this));
    playing_ = true;
  }
  return 0;
}

int32_t WebrtcAudioModule::StopPlayout() {
  base::AutoLock auto_lock(lock_);
  if (playing_) {
    audio_task_runner_->PostTask(
        FROM_HERE,
        base::Bind(&WebrtcAudioModule::StopPlayoutOnAudioThread, this));
    playing_ = false;
  }
  return 0;
}

bool WebrtcAudioModule::Playing() const {
  base::AutoLock auto_lock(lock_);
  return playing_;
}

int32_t WebrtcAudioModule::StartRecording() {
  return 0;
}

int32_t WebrtcAudioModule::StopRecording() {
  return 0;
}

bool WebrtcAudioModule::Recording() const {
  return false;
}

int32_t WebrtcAudioModule::SetAGC(bool enable) {
  return 0;
}

bool WebrtcAudioModule::AGC() const {
  NOTREACHED();
  return false;
}

int32_t WebrtcAudioModule::SetWaveOutVolume(uint16_t volume_left,
                                            uint16_t volume_right) {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::WaveOutVolume(uint16_t* volume_left,
                                         uint16_t* volume_right) const {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::InitSpeaker() {
  return 0;
}

bool WebrtcAudioModule::SpeakerIsInitialized() const {
  return false;
}

int32_t WebrtcAudioModule::InitMicrophone() {
  return 0;
}

bool WebrtcAudioModule::MicrophoneIsInitialized() const {
  return false;
}

int32_t WebrtcAudioModule::SpeakerVolumeIsAvailable(bool* available) {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::SetSpeakerVolume(uint32_t volume) {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::SpeakerVolume(uint32_t* volume) const {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::MaxSpeakerVolume(uint32_t* max_volume) const {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::MinSpeakerVolume(uint32_t* min_volume) const {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::SpeakerVolumeStepSize(uint16_t* step_size) const {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::MicrophoneVolumeIsAvailable(bool* available) {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::SetMicrophoneVolume(uint32_t volume) {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::MicrophoneVolume(uint32_t* volume) const {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::MaxMicrophoneVolume(uint32_t* max_volume) const {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::MinMicrophoneVolume(uint32_t* min_volume) const {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::MicrophoneVolumeStepSize(uint16_t* step_size) const {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::SpeakerMuteIsAvailable(bool* available) {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::SetSpeakerMute(bool enable) {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::SpeakerMute(bool* enabled) const {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::MicrophoneMuteIsAvailable(bool* available) {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::SetMicrophoneMute(bool enable) {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::MicrophoneMute(bool* enabled) const {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::MicrophoneBoostIsAvailable(bool* available) {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::SetMicrophoneBoost(bool enable) {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::MicrophoneBoost(bool* enabled) const {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::StereoPlayoutIsAvailable(bool* available) const {
  *available = true;
  return 0;
}

int32_t WebrtcAudioModule::SetStereoPlayout(bool enable) {
  DCHECK(enable);
  return 0;
}

int32_t WebrtcAudioModule::StereoPlayout(bool* enabled) const {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::StereoRecordingIsAvailable(bool* available) const {
  *available = false;
  return 0;
}

int32_t WebrtcAudioModule::SetStereoRecording(bool enable) {
  return 0;
}

int32_t WebrtcAudioModule::StereoRecording(bool* enabled) const {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::SetRecordingChannel(const ChannelType channel) {
  return 0;
}

int32_t WebrtcAudioModule::RecordingChannel(ChannelType* channel) const {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::SetPlayoutBuffer(const BufferType type,
                                            uint16_t size_ms) {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::PlayoutBuffer(BufferType* type,
                                         uint16_t* size_ms) const {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::PlayoutDelay(uint16_t* delay_ms) const {
  *delay_ms = 0;
  return 0;
}

int32_t WebrtcAudioModule::RecordingDelay(uint16_t* delay_ms) const {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::CPULoad(uint16_t* load) const {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::StartRawOutputFileRecording(
    const char pcm_file_name_utf8[webrtc::kAdmMaxFileNameSize]) {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::StopRawOutputFileRecording() {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::StartRawInputFileRecording(
    const char pcm_file_name_utf8[webrtc::kAdmMaxFileNameSize]) {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::StopRawInputFileRecording() {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::SetRecordingSampleRate(
    const uint32_t samples_per_sec) {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::RecordingSampleRate(
    uint32_t* samples_per_sec) const {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::SetPlayoutSampleRate(
    const uint32_t samples_per_sec) {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::PlayoutSampleRate(uint32_t* samples_per_sec) const {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::ResetAudioDevice() {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::SetLoudspeakerStatus(bool enable) {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::GetLoudspeakerStatus(bool* enabled) const {
  NOTREACHED();
  return -1;
}

bool WebrtcAudioModule::BuiltInAECIsAvailable() const {
  return false;
}

bool WebrtcAudioModule::BuiltInAGCIsAvailable() const {
  return false;
}

bool WebrtcAudioModule::BuiltInNSIsAvailable() const {
  return false;
}

int32_t WebrtcAudioModule::EnableBuiltInAEC(bool enable) {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::EnableBuiltInAGC(bool enable) {
  NOTREACHED();
  return -1;
}

int32_t WebrtcAudioModule::EnableBuiltInNS(bool enable) {
  NOTREACHED();
  return -1;
}

#if defined(WEBRTC_IOS)
int WebrtcAudioModule::GetPlayoutAudioParameters(
    webrtc::AudioParameters* params) const {
  NOTREACHED();
  return -1;
}

int WebrtcAudioModule::GetRecordAudioParameters(
    webrtc::AudioParameters* params) const {
  NOTREACHED();
  return -1;
}
#endif  // WEBRTC_IOS

void WebrtcAudioModule::StartPlayoutOnAudioThread() {
  DCHECK(audio_task_runner_->BelongsToCurrentThread());
  poll_timer_ = base::MakeUnique<base::RepeatingTimer>();
  poll_timer_->Start(
      FROM_HERE, kPollInterval,
      base::Bind(&WebrtcAudioModule::PollFromSource, base::Unretained(this)));
}

void WebrtcAudioModule::StopPlayoutOnAudioThread() {
  DCHECK(audio_task_runner_->BelongsToCurrentThread());
  poll_timer_.reset();
}

void WebrtcAudioModule::PollFromSource() {
  DCHECK(audio_task_runner_->BelongsToCurrentThread());

  base::AutoLock lock(lock_);
  if (!audio_transport_)
    return;

  for (int i = 0; i < kPollInterval.InMilliseconds() / kFrameLengthMs; i++) {
    int64_t elapsed_time_ms = -1;
    int64_t ntp_time_ms = -1;
    char data[kBytesPerSample * kChannels * kSamplesPerFrame];
    audio_transport_->PullRenderData(kBytesPerSample * 8, kSamplingRate,
                                     kChannels, kSamplesPerFrame, data,
                                     &elapsed_time_ms, &ntp_time_ms);
  }
}

}  // namespace protocol
}  // namespace remoting
