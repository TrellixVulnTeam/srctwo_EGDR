// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_DATA_REDUCTION_PROXY_CORE_COMMON_DATA_REDUCTION_PROXY_SERVER_H_
#define COMPONENTS_DATA_REDUCTION_PROXY_CORE_COMMON_DATA_REDUCTION_PROXY_SERVER_H_

#include <vector>

#include "components/data_reduction_proxy/core/common/resource_type_provider.h"
#include "components/data_reduction_proxy/proto/client_config.pb.h"
#include "net/proxy/proxy_server.h"

namespace data_reduction_proxy {

// A class that stores information about a single data reduction proxy server.
class DataReductionProxyServer {
 public:
  DataReductionProxyServer(const net::ProxyServer& proxy_server,
                           ProxyServer_ProxyType proxy_type);

  DataReductionProxyServer(const DataReductionProxyServer& other) = default;

  DataReductionProxyServer& operator=(const DataReductionProxyServer& other) =
      default;

  bool operator==(const DataReductionProxyServer& other) const;

  bool SupportsResourceType(
      ResourceTypeProvider::ContentType content_type) const;

  const net::ProxyServer& proxy_server() const { return proxy_server_; }

  static std::vector<net::ProxyServer> ConvertToNetProxyServers(
      const std::vector<DataReductionProxyServer>&
          data_reduction_proxy_servers);

  // Returns |proxy_type_| for verification by tests.
  ProxyServer_ProxyType GetProxyTypeForTesting() const;

 private:
  net::ProxyServer proxy_server_;
  ProxyServer_ProxyType proxy_type_;
};

}  // namespace data_reduction_proxy

#endif  // COMPONENTS_DATA_REDUCTION_PROXY_CORE_COMMON_DATA_REDUCTION_PROXY_SERVER_H_
