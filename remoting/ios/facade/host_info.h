// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_IOS_FACADE_HOST_INFO_H_
#define REMOTING_IOS_FACADE_HOST_INFO_H_

#include <string>
#include <vector>

#include "base/time/time.h"
#include "base/values.h"

namespace remoting {

enum HostStatus { kHostStatusOnline, kHostStatusOffline };

// |remoting::HostInfo| is an object containing the data from host list fetch
// for transport in native code.
struct HostInfo {
  HostInfo();
  HostInfo(const HostInfo& other);
  ~HostInfo();

  // Returns true if |host_info| is valid and initializes HostInfo.
  bool ParseHostInfo(const base::DictionaryValue& host_info);

  // Returns true if the chromoting host is ready to accept connections.
  bool IsReadyForConnection() const;

  std::string host_id;
  std::string host_jid;
  std::string host_name;
  std::string host_os;
  std::string host_os_version;
  std::string host_version;
  HostStatus status = kHostStatusOffline;
  std::string offline_reason;
  std::string public_key;
  base::Time updated_time;
  std::vector<std::string> token_url_patterns;
};

}  // namespace remoting

#endif  // REMOTING_IOS_FACADE_HOST_INFO_H_
