// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/websockets/websocket_handshake_request_info_impl.h"

#include "base/memory/ptr_util.h"
#include "net/url_request/url_request.h"

namespace content {

namespace {

constexpr int g_tag = 0;

}  // namesapce

WebSocketHandshakeRequestInfoImpl::WebSocketHandshakeRequestInfoImpl(
    int child_id,
    int render_frame_id)
    : child_id_(child_id), render_frame_id_(render_frame_id) {}

WebSocketHandshakeRequestInfoImpl::~WebSocketHandshakeRequestInfoImpl() {}

void WebSocketHandshakeRequestInfoImpl::CreateInfoAndAssociateWithRequest(
    int child_id,
    int render_frame_id,
    net::URLRequest* request) {
  request->SetUserData(&g_tag,
                       base::WrapUnique(new WebSocketHandshakeRequestInfoImpl(
                           child_id, render_frame_id)));
}

int WebSocketHandshakeRequestInfoImpl::GetChildId() const {
  return child_id_;
}

int WebSocketHandshakeRequestInfoImpl::GetRenderFrameId() const {
  return render_frame_id_;
}

const WebSocketHandshakeRequestInfo* WebSocketHandshakeRequestInfo::ForRequest(
    const net::URLRequest* request) {
  return static_cast<WebSocketHandshakeRequestInfoImpl*>(
      request->GetUserData(&g_tag));
}

}  // namespace content
