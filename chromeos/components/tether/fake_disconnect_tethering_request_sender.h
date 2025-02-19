// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_COMPONENTS_TETHER_FAKE_DISCONNECT_TETHERING_REQUEST_SENDER_H_
#define CHROMEOS_COMPONENTS_TETHER_FAKE_DISCONNECT_TETHERING_REQUEST_SENDER_H_

#include <string>
#include <vector>

#include "base/macros.h"
#include "base/observer_list.h"
#include "chromeos/components/tether/disconnect_tethering_request_sender.h"

namespace chromeos {

namespace tether {

// Test double for DisconnectTetheringRequestSender.
class FakeDisconnectTetheringRequestSender
    : public DisconnectTetheringRequestSender {
 public:
  FakeDisconnectTetheringRequestSender();
  ~FakeDisconnectTetheringRequestSender() override;

  void NotifyPendingDisconnectRequestsComplete();

  void set_has_pending_requests(bool has_pending_requests) {
    has_pending_requests_ = has_pending_requests;
  }

  const std::vector<std::string>& device_ids_sent_requests() {
    return device_ids_sent_requests_;
  }

  // DisconnectTetheringRequestSender:
  void SendDisconnectRequestToDevice(const std::string& device_id) override;
  bool HasPendingRequests() override;

 private:
  bool has_pending_requests_ = false;
  std::vector<std::string> device_ids_sent_requests_;

  DISALLOW_COPY_AND_ASSIGN(FakeDisconnectTetheringRequestSender);
};

}  // namespace tether

}  // namespace chromeos

#endif  // CHROMEOS_COMPONENTS_TETHER_FAKE_DISCONNECT_TETHERING_REQUEST_SENDER_H_
