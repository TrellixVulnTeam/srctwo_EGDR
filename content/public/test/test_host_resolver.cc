// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/test/test_host_resolver.h"

#include "base/macros.h"
#include "base/message_loop/message_loop.h"
#include "base/threading/thread.h"
#include "content/browser/browser_thread_impl.h"
#include "content/browser/notification_service_impl.h"
#include "net/base/net_errors.h"
#include "net/base/network_interfaces.h"
#include "net/dns/mock_host_resolver.h"

namespace content {

namespace {
// In many cases it may be not obvious that a test makes a real DNS lookup.
// We generally don't want to rely on external DNS servers for our tests,
// so this host resolver procedure catches external queries and returns a failed
// lookup result.
class LocalHostResolverProc : public net::HostResolverProc {
 public:
  LocalHostResolverProc() : HostResolverProc(NULL) {}

  int Resolve(const std::string& host,
              net::AddressFamily address_family,
              net::HostResolverFlags host_resolver_flags,
              net::AddressList* addrlist,
              int* os_error) override {
    const char* kLocalHostNames[] = {"localhost", "127.0.0.1", "::1"};
    bool local = false;

    if (host == net::GetHostName()) {
      local = true;
    } else {
      for (size_t i = 0; i < arraysize(kLocalHostNames); i++)
        if (host == kLocalHostNames[i]) {
          local = true;
          break;
        }
    }

    // To avoid depending on external resources and to reduce (if not preclude)
    // network interactions from tests, we simulate failure for non-local DNS
    // queries, rather than perform them.
    // If you really need to make an external DNS query, use
    // net::RuleBasedHostResolverProc and its AllowDirectLookup method.
    if (!local) {
      DVLOG(1) << "To avoid external dependencies, simulating failure for "
                  "external DNS lookup of "
               << host;
      return net::ERR_NOT_IMPLEMENTED;
    }

    return ResolveUsingPrevious(host, address_family, host_resolver_flags,
                                addrlist, os_error);
  }

 private:
  ~LocalHostResolverProc() override {}
};
}

TestHostResolver::TestHostResolver()
    : local_resolver_(new LocalHostResolverProc()),
      rule_based_resolver_(
          new net::RuleBasedHostResolverProc(local_resolver_.get())),
      scoped_local_host_resolver_proc_(
          new net::ScopedDefaultHostResolverProc(rule_based_resolver_.get())) {
  rule_based_resolver_->AddSimulatedFailure("wpad");
}

TestHostResolver::~TestHostResolver() {}

}  // namespace content
