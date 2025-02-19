// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/gcm_driver/gcm_client.h"

namespace gcm {

GCMClient::ChromeBuildInfo::ChromeBuildInfo()
    : platform(PLATFORM_UNKNOWN),
      channel(CHANNEL_UNKNOWN) {
}

GCMClient::ChromeBuildInfo::~ChromeBuildInfo() {
}

GCMClient::SendErrorDetails::SendErrorDetails() : result(UNKNOWN_ERROR) {}

GCMClient::SendErrorDetails::SendErrorDetails(const SendErrorDetails& other) =
    default;

GCMClient::SendErrorDetails::~SendErrorDetails() {}

GCMClient::GCMStatistics::GCMStatistics()
    : is_recording(false),
      gcm_client_created(false),
      connection_client_created(false),
      android_id(0),
      send_queue_size(0),
      resend_queue_size(0) {
}

GCMClient::GCMStatistics::GCMStatistics(const GCMStatistics& other) = default;

GCMClient::GCMStatistics::~GCMStatistics() {
}

GCMClient::GCMClient() {
}

GCMClient::~GCMClient() {
}

}  // namespace gcm
