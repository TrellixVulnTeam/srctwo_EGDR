// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/devtools/protocol/inspector_handler.h"

#include "content/browser/devtools/devtools_session.h"
#include "content/browser/frame_host/render_frame_host_impl.h"

namespace content {
namespace protocol {

InspectorHandler::InspectorHandler()
    : DevToolsDomainHandler(Inspector::Metainfo::domainName),
      host_(nullptr) {
}

InspectorHandler::~InspectorHandler() {
}

// static
std::vector<InspectorHandler*> InspectorHandler::ForAgentHost(
    DevToolsAgentHostImpl* host) {
  return DevToolsSession::HandlersForAgentHost<InspectorHandler>(
      host, Inspector::Metainfo::domainName);
}

void InspectorHandler::Wire(UberDispatcher* dispatcher) {
  frontend_.reset(new Inspector::Frontend(dispatcher->channel()));
  Inspector::Dispatcher::wire(dispatcher, this);
}

void InspectorHandler::SetRenderFrameHost(RenderFrameHostImpl* host) {
  host_ = host;
}

void InspectorHandler::TargetCrashed() {
  frontend_->TargetCrashed();
}

void InspectorHandler::TargetDetached(const std::string& reason) {
  frontend_->Detached(reason);
}

Response InspectorHandler::Enable() {
  if (host_ && !host_->IsRenderFrameLive())
    frontend_->TargetCrashed();
  return Response::OK();
}

Response InspectorHandler::Disable() {
  return Response::OK();
}

}  // namespace protocol
}  // namespace content
