// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/browser/app_window/app_window_client.h"

#include "base/logging.h"

namespace extensions {

namespace {

AppWindowClient* g_client = nullptr;

}  // namespace

AppWindowClient* AppWindowClient::Get() {
  return g_client;
}

void AppWindowClient::Set(AppWindowClient* client) {
  // Unit tests that set the AppWindowClient should clear it afterward.
  if (g_client && client) {
    // Rarely, a test may run multiple BrowserProcesses in a single process:
    // crbug.com/751242. This will lead to redundant calls, but the pointers
    // should at least be the same.
    DCHECK_EQ(g_client, client)
        << "AppWindowClient::Set called with different non-null pointers twice "
        << "in a row. A previous test may have set this without clearing it.";
    return;
  }

  g_client = client;
}

}  // namespace extensions
