// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_SHELL_BROWSER_SHELL_BROWSER_CONTEXT_H_
#define EXTENSIONS_SHELL_BROWSER_SHELL_BROWSER_CONTEXT_H_

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "content/shell/browser/shell_browser_context.h"
#include "storage/browser/quota/special_storage_policy.h"

namespace extensions {

class ShellBrowserMainParts;

// The BrowserContext used by the content, apps and extensions systems in
// app_shell.
class ShellBrowserContext : public content::ShellBrowserContext {
 public:
  explicit ShellBrowserContext(ShellBrowserMainParts* browser_main_parts);
  ~ShellBrowserContext() override;

  // content::BrowserContext implementation.
  content::BrowserPluginGuestManager* GetGuestManager() override;
  storage::SpecialStoragePolicy* GetSpecialStoragePolicy() override;
  net::URLRequestContextGetter* CreateRequestContext(
      content::ProtocolHandlerMap* protocol_handlers,
      content::URLRequestInterceptorScopedVector request_interceptors) override;

 private:
  void InitURLRequestContextOnIOThread();

  scoped_refptr<storage::SpecialStoragePolicy> storage_policy_;
  ShellBrowserMainParts* browser_main_parts_;

  DISALLOW_COPY_AND_ASSIGN(ShellBrowserContext);
};

}  // namespace extensions

#endif  // EXTENSIONS_SHELL_BROWSER_SHELL_BROWSER_CONTEXT_H_
