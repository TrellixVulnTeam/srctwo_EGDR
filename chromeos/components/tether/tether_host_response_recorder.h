// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_COMPONENTS_TETHER_TETHER_HOST_RESPONSE_RECORDER_H_
#define CHROMEOS_COMPONENTS_TETHER_TETHER_HOST_RESPONSE_RECORDER_H_

#include <string>
#include <vector>

#include "base/macros.h"
#include "base/observer_list.h"
#include "components/cryptauth/remote_device.h"

class PrefService;
class PrefRegistrySimple;

namespace chromeos {

namespace tether {

// Records responses from tether hosts in user prefs, which persists these
// responses between reboots of the device. When a TetherAvailabilityResponse or
// ConnectTetheringResponse is received, it should be recorded using this class.
// Responses can be retrieved at a later time via getter methods.
class TetherHostResponseRecorder {
 public:
  class Observer {
   public:
    virtual void OnPreviouslyConnectedHostIdsChanged() = 0;
  };

  // Registers the prefs used by this class to |registry|. Must be called before
  // this class is utilized.
  static void RegisterPrefs(PrefRegistrySimple* registry);

  // Note: The PrefService* passed here must be created using the same registry
  // passed to RegisterPrefs().
  explicit TetherHostResponseRecorder(PrefService* pref_service);
  virtual ~TetherHostResponseRecorder();

  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);

  // Records a TetherAvailabilityResponse. This function should be called each
  // time that a response is received from a potential host, even if a
  // connection is not started.
  virtual void RecordSuccessfulTetherAvailabilityResponse(
      const cryptauth::RemoteDevice& remote_device);

  // Gets device IDs corresponding to hosts which have sent
  // TetherAvailabilityResponses with a response code indicating that tethering
  // is available. The list is sorted; the IDs of the devices which have
  // responded most recently are at the front of the list and the IDs of the
  // devices which have responded least recently are at the end of the list.
  virtual std::vector<std::string> GetPreviouslyAvailableHostIds() const;

  // Records a ConnectTetheringResponse. This function should be called each
  // time that a response is received from a host.
  virtual void RecordSuccessfulConnectTetheringResponse(
      const cryptauth::RemoteDevice& remote_device);

  // Gets device IDs corresponding to hosts which have sent
  // ConnectTetheringResponses with a response code indicating that they have
  // successfully turned on their Wi-Fi hotspots. The list is sorted; the IDs of
  // the devices which have responded most recently are at the front of the list
  // and the IDs of the devices which have responded least recently are at the
  // end of the list.
  virtual std::vector<std::string> GetPreviouslyConnectedHostIds() const;

 private:
  friend class NetworkHostScanCacheTest;

  void NotifyObserversPreviouslyConnectedHostIdsChanged();

  // Returns whether the list was changed due to adding the response.
  bool AddRecentResponse(const std::string& device_id,
                         const std::string& pref_name);
  std::vector<std::string> GetDeviceIdsForPref(
      const std::string& pref_name) const;

  PrefService* pref_service_;
  base::ObserverList<Observer> observer_list_;

  DISALLOW_COPY_AND_ASSIGN(TetherHostResponseRecorder);
};

}  // namespace tether

}  // namespace chromeos

#endif  // CHROMEOS_COMPONENTS_TETHER_TETHER_HOST_RESPONSE_RECORDER_H_
