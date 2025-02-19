// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_DBUS_FAKE_CRAS_AUDIO_CLIENT_H_
#define CHROMEOS_DBUS_FAKE_CRAS_AUDIO_CLIENT_H_

#include <stdint.h>

#include "base/macros.h"
#include "chromeos/chromeos_export.h"
#include "chromeos/dbus/cras_audio_client.h"

namespace chromeos {

// The CrasAudioClient implementation used on Linux desktop.
class CHROMEOS_EXPORT FakeCrasAudioClient : public CrasAudioClient {
 public:
  FakeCrasAudioClient();
  ~FakeCrasAudioClient() override;

  // CrasAudioClient overrides:
  void Init(dbus::Bus* bus) override;
  void AddObserver(Observer* observer) override;
  void RemoveObserver(Observer* observer) override;
  bool HasObserver(const Observer* observer) const override;
  void GetVolumeState(const GetVolumeStateCallback& callback) override;
  void GetDefaultOutputBufferSize(
      const GetDefaultOutputBufferSizeCallback& callback) override;
  void GetNodes(const GetNodesCallback& callback,
                const ErrorCallback& error_callback) override;
  void SetOutputNodeVolume(uint64_t node_id, int32_t volume) override;
  void SetOutputUserMute(bool mute_on) override;
  void SetInputNodeGain(uint64_t node_id, int32_t gain) override;
  void SetInputMute(bool mute_on) override;
  void SetActiveOutputNode(uint64_t node_id) override;
  void SetActiveInputNode(uint64_t node_id) override;
  void AddActiveInputNode(uint64_t node_id) override;
  void RemoveActiveInputNode(uint64_t node_id) override;
  void AddActiveOutputNode(uint64_t node_id) override;
  void RemoveActiveOutputNode(uint64_t node_id) override;
  void SwapLeftRight(uint64_t node_id, bool swap) override;
  void SetGlobalOutputChannelRemix(int32_t channels,
                                   const std::vector<double>& mixer) override;
  void WaitForServiceToBeAvailable(
      const WaitForServiceToBeAvailableCallback& callback) override;

  // Modifies an AudioNode from |node_list_| based on |audio_node.id|.
  // if the |audio_node.id| cannot be found in list, Add an
  // AudioNode to |node_list_|
  void InsertAudioNodeToList(const AudioNode& audio_node);

  // Removes an AudioNode from |node_list_| based on |node_id|.
  void RemoveAudioNodeFromList(const uint64_t& node_id);

  // Updates |node_list_| to contain |audio_nodes|.
  void SetAudioNodesForTesting(const AudioNodeList& audio_nodes);

  // Calls SetAudioNodesForTesting() and additionally notifies |observers_|.
  void SetAudioNodesAndNotifyObserversForTesting(
      const AudioNodeList& new_nodes);

  // Generates fake signal for OutputNodeVolumeChanged.
  void NotifyOutputNodeVolumeChangedForTesting(uint64_t node_id, int volume);

  const AudioNodeList& node_list() const { return node_list_; }
  const uint64_t& active_input_node_id() const { return active_input_node_id_; }
  const uint64_t& active_output_node_id() const {
    return active_output_node_id_;
  }
  void set_notify_volume_change_with_delay(bool notify_with_delay) {
    notify_volume_change_with_delay_ = notify_with_delay;
  }

 private:
  // Finds a node in the list based on the id.
  AudioNodeList::iterator FindNode(uint64_t node_id);

  VolumeState volume_state_;
  AudioNodeList node_list_;
  uint64_t active_input_node_id_;
  uint64_t active_output_node_id_;
  // By default, immediately sends OutputNodeVolumeChange signal following the
  // SetOutputNodeVolume fake dbus call.
  bool notify_volume_change_with_delay_ = false;
  base::ObserverList<Observer> observers_;

  DISALLOW_COPY_AND_ASSIGN(FakeCrasAudioClient);
};

}  // namespace chromeos

#endif  // CHROMEOS_DBUS_FAKE_CRAS_AUDIO_CLIENT_H_
