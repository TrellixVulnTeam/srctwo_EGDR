// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/PluginScriptForbiddenScope.h"

#include "platform/wtf/Assertions.h"

namespace blink {

static unsigned g_plugin_script_forbidden_count = 0;

PluginScriptForbiddenScope::PluginScriptForbiddenScope() {
  DCHECK(IsMainThread());
  ++g_plugin_script_forbidden_count;
}

PluginScriptForbiddenScope::~PluginScriptForbiddenScope() {
  DCHECK(IsMainThread());
  DCHECK(g_plugin_script_forbidden_count);
  --g_plugin_script_forbidden_count;
}

bool PluginScriptForbiddenScope::IsForbidden() {
  DCHECK(IsMainThread());
  return g_plugin_script_forbidden_count > 0;
}

}  // namespace blink
