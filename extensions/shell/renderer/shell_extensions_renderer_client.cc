// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/shell/renderer/shell_extensions_renderer_client.h"

#include "base/memory/ptr_util.h"
#include "extensions/renderer/dispatcher.h"
#include "extensions/renderer/dispatcher_delegate.h"

namespace extensions {

ShellExtensionsRendererClient::ShellExtensionsRendererClient()
    : dispatcher_delegate_(std::make_unique<DispatcherDelegate>()),
      dispatcher_(std::make_unique<Dispatcher>(dispatcher_delegate_.get())) {}

ShellExtensionsRendererClient::~ShellExtensionsRendererClient() {
}

bool ShellExtensionsRendererClient::IsIncognitoProcess() const {
  // app_shell doesn't support off-the-record contexts.
  return false;
}

int ShellExtensionsRendererClient::GetLowestIsolatedWorldId() const {
  // app_shell doesn't need to reserve world IDs for anything other than
  // extensions, so we always return 1. Note that 0 is reserved for the global
  // world.
  return 1;
}

Dispatcher* ShellExtensionsRendererClient::GetDispatcher() {
  return dispatcher_.get();
}

}  // namespace extensions
