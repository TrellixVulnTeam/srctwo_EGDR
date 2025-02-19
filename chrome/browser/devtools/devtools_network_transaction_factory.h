// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_DEVTOOLS_DEVTOOLS_NETWORK_TRANSACTION_FACTORY_H_
#define CHROME_BROWSER_DEVTOOLS_DEVTOOLS_NETWORK_TRANSACTION_FACTORY_H_

#include <memory>

#include "base/macros.h"
#include "net/base/request_priority.h"
#include "net/http/http_transaction_factory.h"

class DevToolsNetworkController;

namespace net {
class HttpCache;
class HttpNetworkSession;
class HttpTransaction;
}

// NetworkTransactionFactory wraps HttpNetworkTransactions.
class DevToolsNetworkTransactionFactory : public net::HttpTransactionFactory {
 public:
  DevToolsNetworkTransactionFactory(
      DevToolsNetworkController* controller,
      net::HttpNetworkSession* session);
  ~DevToolsNetworkTransactionFactory() override;

  // net::HttpTransactionFactory methods:
  int CreateTransaction(net::RequestPriority priority,
                        std::unique_ptr<net::HttpTransaction>* trans) override;
  net::HttpCache* GetCache() override;
  net::HttpNetworkSession* GetSession() override;

 private:
  DevToolsNetworkController* controller_;
  std::unique_ptr<net::HttpTransactionFactory> network_layer_;

  DISALLOW_COPY_AND_ASSIGN(DevToolsNetworkTransactionFactory);
};

#endif  // CHROME_BROWSER_DEVTOOLS_DEVTOOLS_NETWORK_TRANSACTION_FACTORY_H_
