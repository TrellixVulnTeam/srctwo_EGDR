// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_RENDERER_WORKER_SCRIPT_CONTEXT_SET_H_
#define EXTENSIONS_RENDERER_WORKER_SCRIPT_CONTEXT_SET_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "base/threading/thread_local.h"
#include "content/public/child/worker_thread.h"
#include "url/gurl.h"
#include "v8/include/v8.h"

namespace extensions {

class ScriptContext;

// A set of ScriptContexts owned by worker threads. Thread safe.
class WorkerScriptContextSet : public content::WorkerThread::Observer {
 public:
  WorkerScriptContextSet();

  ~WorkerScriptContextSet() override;

  // Inserts |context| into the set. Contexts are stored in TLS.
  void Insert(std::unique_ptr<ScriptContext> context);

  // Removes the ScriptContext associated with |v8_context| which was added for
  // |url| (used for sanity checking).
  void Remove(v8::Local<v8::Context> v8_context, const GURL& url);

 private:
  // WorkerThread::Observer:
  void WillStopCurrentWorkerThread() override;

  // Implement thread safety by storing each ScriptContext in TLS.
  base::ThreadLocalPointer<std::vector<std::unique_ptr<ScriptContext>>>
      contexts_tls_;

  DISALLOW_COPY_AND_ASSIGN(WorkerScriptContextSet);
};

}  // namespace extensions

#endif  // EXTENSIONS_RENDERER_WORKER_SCRIPT_CONTEXT_SET_H_
