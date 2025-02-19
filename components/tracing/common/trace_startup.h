// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_TRACING_COMMON_TRACE_STARTUP_H
#define COMPONENTS_TRACING_COMMON_TRACE_STARTUP_H

#include "components/tracing/tracing_export.h"

namespace tracing {

// Enables TraceLog with config based on the command line flags of the process.
// If |can_access_file_system| is false then TraceLog is not enabled in case it
// is required to read config file to start tracing.
void TRACING_EXPORT EnableStartupTracingIfNeeded(bool can_access_file_system);

}  // namespace tracing

#endif  // COMPONENTS_TRACING_COMMON_TRACE_STARTUP_H
