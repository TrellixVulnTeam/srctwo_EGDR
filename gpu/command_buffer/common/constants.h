// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_COMMAND_BUFFER_COMMON_CONSTANTS_H_
#define GPU_COMMAND_BUFFER_COMMON_CONSTANTS_H_

#include <stddef.h>
#include <stdint.h>

#include "build/build_config.h"

namespace gpu {

typedef int32_t CommandBufferOffset;
const CommandBufferOffset kInvalidCommandBufferOffset = -1;

namespace error {
  enum Error {
    kNoError,
    kInvalidSize,
    kOutOfBounds,
    kUnknownCommand,
    kInvalidArguments,
    kLostContext,
    kGenericError,
    kDeferCommandUntilLater,
    kDeferLaterCommands,
    kErrorLast = kDeferLaterCommands,
  };

  // Return true if the given error code is an actual error.
  inline bool IsError(Error error) {
    return error != kNoError && error != kDeferCommandUntilLater &&
           error != kDeferLaterCommands;
  }

  // Provides finer grained information about why the context was lost.
  enum ContextLostReason {
    // This context definitely provoked the loss of context.
    kGuilty,

    // This context definitely did not provoke the loss of context.
    kInnocent,

    // It is unknown whether this context provoked the loss of context.
    kUnknown,

    // GL_OUT_OF_MEMORY caused this context to be lost.
    kOutOfMemory,

    // A failure to make the context current caused it to be lost.
    kMakeCurrentFailed,

    // The GPU channel was lost. This error is set client-side.
    kGpuChannelLost,

    // The GPU process sent an invalid message/reply. This error is set
    // client-side.
    kInvalidGpuMessage,

    kContextLostReasonLast = kInvalidGpuMessage
  };
}

// Invalid shared memory Id, returned by RegisterSharedMemory in case of
// failure.
const int32_t kInvalidSharedMemoryId = -1;

// Common Command Buffer shared memory transfer buffer ID.
const int32_t kCommandBufferSharedMemoryId = 4;

// The size to set for the program cache for default and low-end device cases.
#if !defined(OS_ANDROID)
const size_t kDefaultMaxProgramCacheMemoryBytes = 6 * 1024 * 1024;
#else
const size_t kDefaultMaxProgramCacheMemoryBytes = 2 * 1024 * 1024;
const size_t kLowEndMaxProgramCacheMemoryBytes = 128 * 1024;
#endif

// Namespace used to separate various command buffer types.
enum CommandBufferNamespace : int8_t {
  INVALID = -1,

  GPU_IO,
  IN_PROCESS,
  MOJO,
  MOJO_LOCAL,

  NUM_COMMAND_BUFFER_NAMESPACES
};

}  // namespace gpu

#endif  // GPU_COMMAND_BUFFER_COMMON_CONSTANTS_H_
