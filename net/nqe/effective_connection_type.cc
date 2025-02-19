// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/nqe/effective_connection_type.h"

#include "base/logging.h"

namespace {

const char kDeprectedEffectiveConnectionTypeSlow2G[] = "Slow2G";

}  // namespace

namespace net {

const char kEffectiveConnectionTypeUnknown[] = "Unknown";
const char kEffectiveConnectionTypeOffline[] = "Offline";
const char kEffectiveConnectionTypeSlow2G[] = "Slow-2G";
const char kEffectiveConnectionType2G[] = "2G";
const char kEffectiveConnectionType3G[] = "3G";
const char kEffectiveConnectionType4G[] = "4G";

const char* GetNameForEffectiveConnectionType(EffectiveConnectionType type) {
  switch (type) {
    case EFFECTIVE_CONNECTION_TYPE_UNKNOWN:
      return kEffectiveConnectionTypeUnknown;
    case EFFECTIVE_CONNECTION_TYPE_OFFLINE:
      return kEffectiveConnectionTypeOffline;
    case EFFECTIVE_CONNECTION_TYPE_SLOW_2G:
      return kEffectiveConnectionTypeSlow2G;
    case EFFECTIVE_CONNECTION_TYPE_2G:
      return kEffectiveConnectionType2G;
    case EFFECTIVE_CONNECTION_TYPE_3G:
      return kEffectiveConnectionType3G;
    case EFFECTIVE_CONNECTION_TYPE_4G:
      return kEffectiveConnectionType4G;
    case EFFECTIVE_CONNECTION_TYPE_LAST:
      NOTREACHED();
      return "";
  }
  NOTREACHED();
  return "";
}

base::Optional<EffectiveConnectionType> GetEffectiveConnectionTypeForName(
    base::StringPiece connection_type_name) {
  if (connection_type_name == kEffectiveConnectionTypeUnknown)
    return EFFECTIVE_CONNECTION_TYPE_UNKNOWN;
  if (connection_type_name == kEffectiveConnectionTypeOffline)
    return EFFECTIVE_CONNECTION_TYPE_OFFLINE;
  if (connection_type_name == kEffectiveConnectionTypeSlow2G)
    return EFFECTIVE_CONNECTION_TYPE_SLOW_2G;
  // Return EFFECTIVE_CONNECTION_TYPE_SLOW_2G if the deprecated string
  // representation is in use.
  if (connection_type_name == kDeprectedEffectiveConnectionTypeSlow2G)
    return EFFECTIVE_CONNECTION_TYPE_SLOW_2G;
  if (connection_type_name == kEffectiveConnectionType2G)
    return EFFECTIVE_CONNECTION_TYPE_2G;
  if (connection_type_name == kEffectiveConnectionType3G)
    return EFFECTIVE_CONNECTION_TYPE_3G;
  if (connection_type_name == kEffectiveConnectionType4G)
    return EFFECTIVE_CONNECTION_TYPE_4G;
  return base::nullopt;
}

const char* DeprecatedGetNameForEffectiveConnectionType(
    EffectiveConnectionType type) {
  switch (type) {
    case EFFECTIVE_CONNECTION_TYPE_SLOW_2G:
      return kDeprectedEffectiveConnectionTypeSlow2G;
    default:
      return GetNameForEffectiveConnectionType(type);
  }
}

}  // namespace net
