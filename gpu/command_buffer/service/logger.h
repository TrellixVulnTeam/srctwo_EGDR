// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file contains the Logger class.

#ifndef GPU_COMMAND_BUFFER_SERVICE_LOGGER_H_
#define GPU_COMMAND_BUFFER_SERVICE_LOGGER_H_

#include <stdint.h>

#include <string>

#include "base/callback.h"
#include "base/macros.h"
#include "gpu/gpu_export.h"

namespace gpu {
namespace gles2 {

class DebugMarkerManager;
class GLES2DecoderClient;

class GPU_EXPORT Logger {
 public:
  static const int kMaxLogMessages = 256;

  Logger(const DebugMarkerManager* debug_marker_manager,
         GLES2DecoderClient* client);
  ~Logger();

  void LogMessage(const char* filename, int line, const std::string& msg);
  const std::string& GetLogPrefix() const;

  // Defaults to true. Set to false for the gpu_unittests as they
  // are explicitly checking errors are generated and so don't need the numerous
  // messages. Otherwise, chromium code that generates these errors likely has a
  // bug.
  void set_log_synthesized_gl_errors(bool enabled) {
    log_synthesized_gl_errors_ = enabled;
  }

 private:
  // Uses the current marker to add information to logs.
  const DebugMarkerManager* debug_marker_manager_;
  GLES2DecoderClient* client_;
  std::string this_in_hex_;

  int log_message_count_;
  bool log_synthesized_gl_errors_;

  DISALLOW_COPY_AND_ASSIGN(Logger);
};

}  // namespace gles2
}  // namespace gpu

#endif  // GPU_COMMAND_BUFFER_SERVICE_LOGGER_H_

