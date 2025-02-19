// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_PROTOCOL_PAIRING_CLIENT_AUTHENTICATOR_H_
#define REMOTING_PROTOCOL_PAIRING_CLIENT_AUTHENTICATOR_H_

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "remoting/protocol/client_authentication_config.h"
#include "remoting/protocol/pairing_authenticator_base.h"

namespace remoting {
namespace protocol {

class PairingClientAuthenticator : public PairingAuthenticatorBase {
 public:
  PairingClientAuthenticator(
      const ClientAuthenticationConfig& client_auth_config,
      const CreateBaseAuthenticatorCallback&
          create_base_authenticator_callback);
  ~PairingClientAuthenticator() override;

  // Start() or StartPaired() must be called after the authenticator is created.
  // Start() handles both cases when pairing exists and when it doesn't.
  // StartPaired() can only be used when pairing exists (i.e. client_id and
  // pairing_secret are set in the |client_auth_config|). It is used to
  // initialize the authenticator synchronously in
  // NegotiatingClientAuthentitcator, while Start() may be executed
  // asynchronously to fetch the PIN.
  void Start(State initial_state, const base::Closure& resume_callback);
  void StartPaired(State initial_state);

  // Authenticator interface.
  State state() const override;

 private:
  // PairingAuthenticatorBase overrides.
  void CreateSpakeAuthenticatorWithPin(
      State initial_state,
      const base::Closure& resume_callback) override;

  void OnPinFetched(State initial_state,
                    const base::Closure& resume_callback,
                    const std::string& pin);

  ClientAuthenticationConfig client_auth_config_;
  CreateBaseAuthenticatorCallback create_base_authenticator_callback_;

  // Set to true if a PIN-based authenticator has been requested but has not
  // yet been set.
  bool waiting_for_pin_ = false;

  base::WeakPtrFactory<PairingClientAuthenticator> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(PairingClientAuthenticator);
};

}  // namespace protocol
}  // namespace remoting

#endif  // REMOTING_PROTOCOL_PAIRING_AUTHENTICATOR_H_
