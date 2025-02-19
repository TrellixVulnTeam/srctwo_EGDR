// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/sync/engine_impl/events/normal_get_updates_request_event.h"

#include "base/strings/stringprintf.h"
#include "components/sync/engine_impl/cycle/nudge_tracker.h"
#include "components/sync/protocol/proto_value_conversions.h"

namespace syncer {

NormalGetUpdatesRequestEvent::NormalGetUpdatesRequestEvent(
    base::Time timestamp,
    const NudgeTracker& nudge_tracker,
    const sync_pb::ClientToServerMessage& request)
    : timestamp_(timestamp),
      nudged_types_(nudge_tracker.GetNudgedTypes()),
      notified_types_(nudge_tracker.GetNotifiedTypes()),
      refresh_requested_types_(nudge_tracker.GetRefreshRequestedTypes()),
      is_retry_(nudge_tracker.IsRetryRequired()),
      request_(request) {}

NormalGetUpdatesRequestEvent::~NormalGetUpdatesRequestEvent() {}

base::Time NormalGetUpdatesRequestEvent::GetTimestamp() const {
  return timestamp_;
}

std::string NormalGetUpdatesRequestEvent::GetType() const {
  return "Normal GetUpdate request";
}

std::string NormalGetUpdatesRequestEvent::GetDetails() const {
  std::string details;

  if (!nudged_types_.Empty()) {
    if (!details.empty())
      details.append("\n");
    details.append(base::StringPrintf(
        "Nudged types: %s", ModelTypeSetToString(nudged_types_).c_str()));
  }

  if (!notified_types_.Empty()) {
    if (!details.empty())
      details.append("\n");
    details.append(base::StringPrintf(
        "Notified types: %s", ModelTypeSetToString(notified_types_).c_str()));
  }

  if (!refresh_requested_types_.Empty()) {
    if (!details.empty())
      details.append("\n");
    details.append(base::StringPrintf(
        "Refresh requested types: %s",
        ModelTypeSetToString(refresh_requested_types_).c_str()));
  }

  if (is_retry_) {
    if (!details.empty())
      details.append("\n");
    details.append(base::StringPrintf("Is retry: True"));
  }

  return details;
}

std::unique_ptr<base::DictionaryValue>
NormalGetUpdatesRequestEvent::GetProtoMessage(bool include_specifics) const {
  return std::unique_ptr<base::DictionaryValue>(
      ClientToServerMessageToValue(request_, include_specifics));
}

std::unique_ptr<ProtocolEvent> NormalGetUpdatesRequestEvent::Clone() const {
  return std::unique_ptr<ProtocolEvent>(new NormalGetUpdatesRequestEvent(
      timestamp_, nudged_types_, notified_types_, refresh_requested_types_,
      is_retry_, request_));
}

NormalGetUpdatesRequestEvent::NormalGetUpdatesRequestEvent(
    base::Time timestamp,
    ModelTypeSet nudged_types,
    ModelTypeSet notified_types,
    ModelTypeSet refresh_requested_types,
    bool is_retry,
    sync_pb::ClientToServerMessage request)
    : timestamp_(timestamp),
      nudged_types_(nudged_types),
      notified_types_(notified_types),
      refresh_requested_types_(refresh_requested_types),
      is_retry_(is_retry),
      request_(request) {}

}  // namespace syncer
