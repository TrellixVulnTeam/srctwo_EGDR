// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "net/http/transport_security_state.h"

namespace net {

class TransportSecurityStateStaticFuzzer {
 public:
  bool FuzzStaticDomainState(TransportSecurityState* state,
                             const std::string& input) {
    state->enable_static_pins_ = true;
    TransportSecurityState::STSState sts_result;
    TransportSecurityState::PKPState pkp_result;
    return state->GetStaticDomainState(input, &sts_result, &pkp_result);
  }

  bool FuzzStaticExpectCTState(TransportSecurityState* state,
                               const std::string& input) {
    state->enable_static_expect_ct_ = true;
    TransportSecurityState::ExpectCTState result;
    return state->GetStaticExpectCTState(input, &result);
  }

  bool FuzzStaticExpectStapleState(TransportSecurityState* state,
                                   const std::string& input) {
    state->enable_static_expect_staple_ = true;
    TransportSecurityState::ExpectStapleState result;
    return state->GetStaticExpectStapleState(input, &result);
  }
};

}  // namespace net

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  std::string input(reinterpret_cast<const char*>(data), size);

  net::TransportSecurityStateStaticFuzzer helper;
  net::TransportSecurityState state;

  helper.FuzzStaticDomainState(&state, input);
  helper.FuzzStaticExpectCTState(&state, input);
  helper.FuzzStaticExpectStapleState(&state, input);

  return 0;
}
