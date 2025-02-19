// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/invalidation/impl/invalidation_service_util.h"

#include "base/base64.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "base/rand_util.h"
#include "components/invalidation/impl/invalidation_switches.h"

namespace invalidation {

notifier::NotifierOptions ParseNotifierOptions(
    const base::CommandLine& command_line) {
  notifier::NotifierOptions notifier_options;

  if (command_line.HasSwitch(switches::kSyncNotificationHostPort)) {
    notifier_options.xmpp_host_port =
        net::HostPortPair::FromString(
            command_line.GetSwitchValueASCII(
                switches::kSyncNotificationHostPort));
    DVLOG(1) << "Using " << notifier_options.xmpp_host_port.ToString()
             << " for test sync notification server.";
  }

  notifier_options.allow_insecure_connection =
      command_line.HasSwitch(switches::kSyncAllowInsecureXmppConnection);
  DVLOG_IF(1, notifier_options.allow_insecure_connection)
      << "Allowing insecure XMPP connections.";

  return notifier_options;
}

std::string GenerateInvalidatorClientId() {
  // Generate a GUID with 128 bits worth of base64-encoded randomness.
  // This format is similar to that of sync's cache_guid.
  const int kGuidBytes = 128 / 8;
  std::string guid;
  base::Base64Encode(base::RandBytesAsString(kGuidBytes), &guid);
  DCHECK(!guid.empty());
  return guid;
}

}  // namespace invalidation
