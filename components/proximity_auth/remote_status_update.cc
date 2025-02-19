// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/proximity_auth/remote_status_update.h"

#include "base/logging.h"
#include "base/values.h"

namespace {

// The value of the 'type' status update field.
const char kStatusUpdateType[] = "status_update";

// Keys in the serialized RemoteStatusUpdate JSON object.
const char kType[] = "type";
const char kUserPresence[] = "user_presence";
const char kSecureScreenLock[] = "secure_screen_lock";
const char kTrustAgent[] = "trust_agent";

// Values in the serialized RemoteStatusUpdate JSON object.
const char kUserPresent[] = "present";
const char kUserAbsent[] = "absent";
const char kUserPresenceUnknown[] = "unknown";

const char kSecureScreenLockEnabled[] = "enabled";
const char kSecureScreenLockDisabled[] = "disabled";
const char kSecureScreenLockStateUnknown[] = "unknown";

const char kTrustAgentEnabled[] = "enabled";
const char kTrustAgentDisabled[] = "disabled";
const char kTrustAgentUnsupported[] = "unsupported";

}  // namespace

namespace proximity_auth {

// static
std::unique_ptr<RemoteStatusUpdate> RemoteStatusUpdate::Deserialize(
    const base::DictionaryValue& serialized_value) {
  std::string type;
  if (!serialized_value.GetString(kType, &type) || type != kStatusUpdateType) {
    VLOG(1) << "Unable to parse remote status update: unexpected type. "
            << "Expected: '" << kStatusUpdateType << "', "
            << "Saw: '" << type << "'.";
    return std::unique_ptr<RemoteStatusUpdate>();
  }

  std::string user_presence, secure_screen_lock_state, trust_agent_state;
  if (!serialized_value.GetString(kUserPresence, &user_presence) ||
      !serialized_value.GetString(kSecureScreenLock,
                                  &secure_screen_lock_state) ||
      !serialized_value.GetString(kTrustAgent, &trust_agent_state)) {
    VLOG(1) << "Unable to parse remote status update: missing data value. "
            << "Status update:\n" << serialized_value;
    return std::unique_ptr<RemoteStatusUpdate>();
  }

  std::unique_ptr<RemoteStatusUpdate> parsed_update(new RemoteStatusUpdate);
  if (user_presence == kUserPresent) {
    parsed_update->user_presence = USER_PRESENT;
  } else if (user_presence == kUserAbsent) {
    parsed_update->user_presence = USER_ABSENT;
  } else if (user_presence == kUserPresenceUnknown) {
    parsed_update->user_presence = USER_PRESENCE_UNKNOWN;
  } else {
    VLOG(1) << "Unable to parse remote status update: invalid user presence: '"
            << user_presence << "'.";
    return std::unique_ptr<RemoteStatusUpdate>();
  }

  if (secure_screen_lock_state == kSecureScreenLockEnabled) {
    parsed_update->secure_screen_lock_state = SECURE_SCREEN_LOCK_ENABLED;
  } else if (secure_screen_lock_state == kSecureScreenLockDisabled) {
    parsed_update->secure_screen_lock_state = SECURE_SCREEN_LOCK_DISABLED;
  } else if (secure_screen_lock_state == kSecureScreenLockStateUnknown) {
    parsed_update->secure_screen_lock_state = SECURE_SCREEN_LOCK_STATE_UNKNOWN;
  } else {
    VLOG(1) << "Unable to parse remote status update: invalid secure screen "
            << "lock state: '" << secure_screen_lock_state << "'.";
    return std::unique_ptr<RemoteStatusUpdate>();
  }

  if (trust_agent_state == kTrustAgentEnabled) {
    parsed_update->trust_agent_state = TRUST_AGENT_ENABLED;
  } else if (trust_agent_state == kTrustAgentDisabled) {
    parsed_update->trust_agent_state = TRUST_AGENT_DISABLED;
  } else if (trust_agent_state == kTrustAgentUnsupported) {
    parsed_update->trust_agent_state = TRUST_AGENT_UNSUPPORTED;
  } else {
    VLOG(1) << "Unable to parse remote status update: invalid trust agent "
            << "state: '" << trust_agent_state << "'.";
    return std::unique_ptr<RemoteStatusUpdate>();
  }

  return parsed_update;
}

}  // namespace proximity_auth
