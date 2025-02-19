// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_DEVTOOLS_FORWARDING_AGENT_HOST_H_
#define CONTENT_BROWSER_DEVTOOLS_FORWARDING_AGENT_HOST_H_

#include <memory>

#include "base/memory/ref_counted.h"
#include "content/browser/devtools/devtools_agent_host_impl.h"
#include "content/public/browser/devtools_external_agent_proxy.h"
#include "content/public/browser/devtools_external_agent_proxy_delegate.h"

namespace content {

class ForwardingAgentHost
    : public DevToolsAgentHostImpl,
      public DevToolsExternalAgentProxy {
 public:
  ForwardingAgentHost(
      const std::string& id,
      std::unique_ptr<DevToolsExternalAgentProxyDelegate> delegate);

 private:
  ~ForwardingAgentHost() override;

  // DevToolsExternalAgentProxy implementation.
  void DispatchOnClientHost(const std::string& message) override;
  void ConnectionClosed() override;

  // DevToolsAgentHostImpl implementation.
  void AttachSession(DevToolsSession* session) override;
  void DetachSession(int session_id) override;
  bool DispatchProtocolMessage(
      DevToolsSession* session,
      const std::string& message) override;

  // DevToolsAgentHost implementation
  std::string GetType() override;
  std::string GetTitle() override;
  GURL GetURL() override;
  GURL GetFaviconURL() override;
  std::string GetFrontendURL() override;
  bool Activate() override;
  void Reload() override;
  bool Close() override;
  base::TimeTicks GetLastActivityTime() override;

  std::unique_ptr<DevToolsExternalAgentProxyDelegate> delegate_;
  std::string type_;
  std::string title_;
  GURL url_;
};

}  // namespace content

#endif  // CONTENT_BROWSER_DEVTOOLS_FORWARDING_AGENT_HOST_H_
