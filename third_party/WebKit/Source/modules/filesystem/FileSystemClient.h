/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef FileSystemClient_h
#define FileSystemClient_h

#include <memory>
#include "modules/ModulesExport.h"
#include "platform/FileSystemType.h"
#include "platform/wtf/Allocator.h"
#include "platform/wtf/Forward.h"
#include "platform/wtf/Noncopyable.h"

namespace blink {

class ExecutionContext;
class LocalFrame;
class ContentSettingCallbacks;
class WorkerClients;

class FileSystemClient {
  USING_FAST_MALLOC(FileSystemClient);
  WTF_MAKE_NONCOPYABLE(FileSystemClient);

 public:
  FileSystemClient() {}
  virtual ~FileSystemClient() {}

  virtual bool RequestFileSystemAccessSync(ExecutionContext*) = 0;
  virtual void RequestFileSystemAccessAsync(
      ExecutionContext*,
      std::unique_ptr<ContentSettingCallbacks>) = 0;
};

MODULES_EXPORT void ProvideLocalFileSystemTo(LocalFrame&,
                                             std::unique_ptr<FileSystemClient>);

MODULES_EXPORT void ProvideLocalFileSystemToWorker(
    WorkerClients*,
    std::unique_ptr<FileSystemClient>);

}  // namespace blink

#endif  // FileSystemClient_h
