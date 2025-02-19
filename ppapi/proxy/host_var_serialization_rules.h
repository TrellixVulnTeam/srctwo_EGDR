// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PPAPI_PROXY_HOST_VAR_SERIALIZATION_RULES_H_
#define PPAPI_PROXY_HOST_VAR_SERIALIZATION_RULES_H_

#include <string>

#include "base/macros.h"
#include "ppapi/c/ppb_var.h"
#include "ppapi/proxy/var_serialization_rules.h"

namespace ppapi {
namespace proxy {

// Implementation of the VarSerializationRules interface for the host side.
class HostVarSerializationRules : public VarSerializationRules {
 public:
  HostVarSerializationRules();
  ~HostVarSerializationRules();

  // VarSerialization implementation.
  virtual PP_Var SendCallerOwned(const PP_Var& var);
  virtual PP_Var BeginReceiveCallerOwned(const PP_Var& var);
  virtual void EndReceiveCallerOwned(const PP_Var& var);
  virtual PP_Var ReceivePassRef(const PP_Var& var);
  virtual PP_Var BeginSendPassRef(const PP_Var& var);
  virtual void EndSendPassRef(const PP_Var& var);
  virtual void ReleaseObjectRef(const PP_Var& var);

 private:
  DISALLOW_COPY_AND_ASSIGN(HostVarSerializationRules);
};

}  // namespace proxy
}  // namespace ppapi

#endif  // PPAPI_PROXY_HOST_VAR_SERIALIZATION_RULES_H_
