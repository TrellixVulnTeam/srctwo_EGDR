// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_DEVTOOLS_PROTOCOL_INSPECTOR_HANDLER_H_
#define CONTENT_BROWSER_DEVTOOLS_PROTOCOL_INSPECTOR_HANDLER_H_

#include "base/macros.h"
#include "content/browser/devtools/protocol/devtools_domain_handler.h"
#include "content/browser/devtools/protocol/inspector.h"

namespace content {

class DevToolsAgentHostImpl;
class RenderFrameHostImpl;

namespace protocol {

class InspectorHandler : public DevToolsDomainHandler,
                         public Inspector::Backend {
 public:
  InspectorHandler();
  ~InspectorHandler() override;

  static std::vector<InspectorHandler*> ForAgentHost(
      DevToolsAgentHostImpl* host);

  void Wire(UberDispatcher* dispatcher) override;
  void SetRenderFrameHost(RenderFrameHostImpl* host) override;

  void TargetCrashed();
  void TargetDetached(const std::string& reason);

  Response Enable() override;
  Response Disable() override;

 private:
  std::unique_ptr<Inspector::Frontend> frontend_;
  RenderFrameHostImpl* host_;

  DISALLOW_COPY_AND_ASSIGN(InspectorHandler);
};

}  // namespace protocol
}  // namespace content

#endif  // CONTENT_BROWSER_DEVTOOLS_PROTOCOL_INSPECTOR_HANDLER_H_
