// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/sync/engine_impl/traffic_logger.h"

#include <memory>
#include <string>

#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/values.h"
#include "components/sync/protocol/proto_value_conversions.h"
#include "components/sync/protocol/sync.pb.h"

namespace syncer {

namespace {
template <class T>
void LogData(const T& data,
             std::unique_ptr<base::DictionaryValue> (
                 *to_dictionary_value)(const T&, bool),
             const std::string& description) {
  if (DCHECK_IS_ON() && VLOG_IS_ON(1)) {
    std::unique_ptr<base::DictionaryValue> value =
        (*to_dictionary_value)(data, true /* include_specifics */);
    std::string message;
    base::JSONWriter::WriteWithOptions(
        *value, base::JSONWriter::OPTIONS_PRETTY_PRINT, &message);
    DVLOG(1) << "\n" << description << "\n" << message << "\n";
  }
}
}  // namespace

void LogClientToServerMessage(const sync_pb::ClientToServerMessage& msg) {
  LogData(msg, &ClientToServerMessageToValue,
          "******Client To Server Message******");
}

void LogClientToServerResponse(
    const sync_pb::ClientToServerResponse& response) {
  LogData(response, &ClientToServerResponseToValue,
          "******Server Response******");
}

}  // namespace syncer
