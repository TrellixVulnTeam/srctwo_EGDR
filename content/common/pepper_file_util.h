// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_COMMON_PEPPER_FILE_UTIL_H_
#define CONTENT_COMMON_PEPPER_FILE_UTIL_H_

#include "base/files/file.h"
#include "base/sync_socket.h"
#include "ppapi/c/pp_file_info.h"
#include "ppapi/features/features.h"
#include "storage/common/fileapi/file_system_types.h"

#if !BUILDFLAG(ENABLE_PLUGINS)
#error "Plugins should be enabled"
#endif

namespace content {

storage::FileSystemType PepperFileSystemTypeToFileSystemType(
    PP_FileSystemType type);

int IntegerFromSyncSocketHandle(
    const base::SyncSocket::Handle& socket_handle);

}  // namespace content

#endif  // CONTENT_COMMON_PEPPER_FILE_UTIL_H_
