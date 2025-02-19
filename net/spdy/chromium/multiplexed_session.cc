// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/spdy/chromium/multiplexed_session.h"

namespace net {

MultiplexedSessionHandle::MultiplexedSessionHandle(
    base::WeakPtr<MultiplexedSession> session)
    : session_(session) {
  SaveSSLInfo();
}

MultiplexedSessionHandle::~MultiplexedSessionHandle() {}

bool MultiplexedSessionHandle::GetRemoteEndpoint(IPEndPoint* endpoint) {
  if (!session_)
    return false;

  return session_->GetRemoteEndpoint(endpoint);
}

bool MultiplexedSessionHandle::GetSSLInfo(SSLInfo* ssl_info) const {
  if (!has_ssl_info_)
    return false;

  *ssl_info = ssl_info_;
  return true;
}

void MultiplexedSessionHandle::SaveSSLInfo() {
  has_ssl_info_ = session_->GetSSLInfo(&ssl_info_);
}

Error MultiplexedSessionHandle::GetTokenBindingSignature(
    crypto::ECPrivateKey* key,
    TokenBindingType tb_type,
    std::vector<uint8_t>* out) {
  if (!session_)
    return ERR_CONNECTION_CLOSED;
  return session_->GetTokenBindingSignature(key, tb_type, out);
}

}  // namespace net
