// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SHELL_BROWSER_SHELL_RESOURCE_DISPATCHER_HOST_DELEGATE_H_
#define CONTENT_SHELL_BROWSER_SHELL_RESOURCE_DISPATCHER_HOST_DELEGATE_H_

#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/macros.h"
#include "content/public/browser/resource_dispatcher_host_delegate.h"

namespace content {

class ShellResourceDispatcherHostDelegate
    : public ResourceDispatcherHostDelegate {
 public:
  ShellResourceDispatcherHostDelegate();
  ~ShellResourceDispatcherHostDelegate() override;

  // ResourceDispatcherHostDelegate implementation.
  ResourceDispatcherHostLoginDelegate* CreateLoginDelegate(
      net::AuthChallengeInfo* auth_info,
      net::URLRequest* request) override;

  // Used for content_browsertests.
  void set_login_request_callback(
      base::Callback<void()> login_request_callback) {
    login_request_callback_ = login_request_callback;
  }

 private:
  base::Callback<void()> login_request_callback_;

  DISALLOW_COPY_AND_ASSIGN(ShellResourceDispatcherHostDelegate);
};

}  // namespace content

#endif  // CONTENT_SHELL_BROWSER_SHELL_RESOURCE_DISPATCHER_HOST_DELEGATE_H_
