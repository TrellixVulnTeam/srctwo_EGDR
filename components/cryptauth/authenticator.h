// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_CRYPTAUTH_AUTHENTICATOR_H_
#define COMPONENTS_CRYPTAUTH_AUTHENTICATOR_H_

#include <memory>

#include "base/callback_forward.h"

namespace cryptauth {

class SecureContext;

// Interface for authenticating the remote connection. The two devices
// authenticate each other, and if the protocol succeeds, establishes a
// SecureContext that is used to securely encode and decode all messages sent
// and received over the connection.
// Do not reuse after calling |Authenticate()|.
class Authenticator {
 public:
  // The result of the authentication protocol.
  enum class Result {
    SUCCESS,
    DISCONNECTED,
    FAILURE,
  };

  // Feature to be used in |WireMessage|s sent during the authentication
  // handshake.
  static const char kAuthenticationFeature[];

  virtual ~Authenticator() {}

  // Initiates the authentication attempt, invoking |callback| with the result.
  // If the authentication protocol succeeds, then |secure_context| will be
  // contain the SecureContext used to securely exchange messages. Otherwise, it
  // will be null if the protocol fails.
  typedef base::Callback<void(Result result,
                              std::unique_ptr<SecureContext> secure_context)>
      AuthenticationCallback;
  virtual void Authenticate(const AuthenticationCallback& callback) = 0;
};

}  // namespace cryptauth

#endif  // COMPONENTS_CRYPTAUTH_AUTHENTICATOR_H_
