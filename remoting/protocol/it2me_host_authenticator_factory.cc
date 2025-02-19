// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/protocol/it2me_host_authenticator_factory.h"

#include <memory>
#include <string>
#include <utility>

#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "remoting/base/rsa_key_pair.h"
#include "remoting/protocol/negotiating_host_authenticator.h"
#include "remoting/protocol/validating_authenticator.h"

namespace remoting {
namespace protocol {

It2MeHostAuthenticatorFactory::It2MeHostAuthenticatorFactory(
    const std::string& local_cert,
    scoped_refptr<RsaKeyPair> key_pair,
    const std::string& access_code_hash,
    const ValidatingAuthenticator::ValidationCallback& callback)
    : local_cert_(local_cert),
      key_pair_(key_pair),
      access_code_hash_(access_code_hash),
      validation_callback_(callback) {}

It2MeHostAuthenticatorFactory::~It2MeHostAuthenticatorFactory() {}

std::unique_ptr<Authenticator>
It2MeHostAuthenticatorFactory::CreateAuthenticator(
    const std::string& local_jid,
    const std::string& remote_jid) {
  std::unique_ptr<Authenticator> authenticator(
      NegotiatingHostAuthenticator::CreateWithSharedSecret(
          local_jid, remote_jid, local_cert_, key_pair_, access_code_hash_,
          nullptr));

  return base::MakeUnique<ValidatingAuthenticator>(
      remote_jid, validation_callback_, std::move(authenticator));
}

}  // namespace protocol
}  // namespace remoting
