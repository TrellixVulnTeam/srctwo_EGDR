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
#ifndef ServiceWorkerThread_h
#define ServiceWorkerThread_h

#include "core/frame/csp/ContentSecurityPolicy.h"
#include "core/workers/WorkerThread.h"
#include "modules/ModulesExport.h"
#include <memory>

namespace blink {

class ServiceWorkerGlobalScopeProxy;
class ServiceWorkerInstalledScriptsManager;
struct GlobalScopeCreationParams;

// ServiceWorkerThread is an implementation of WorkerThread for service workers.
// This provides a backing thread and an installed scripts manager.
class MODULES_EXPORT ServiceWorkerThread final : public WorkerThread {
 public:
  // ServiceWorkerThread owns a given ServiceWorkerGlobalScopeProxy via
  // Persistent.
  ServiceWorkerThread(ThreadableLoadingContext*,
                      ServiceWorkerGlobalScopeProxy*,
                      std::unique_ptr<ServiceWorkerInstalledScriptsManager>);
  ~ServiceWorkerThread() override;

  WorkerBackingThread& GetWorkerBackingThread() override {
    return *worker_backing_thread_;
  }
  void ClearWorkerBackingThread() override;

 protected:
  WorkerOrWorkletGlobalScope* CreateWorkerGlobalScope(
      std::unique_ptr<GlobalScopeCreationParams>) override;

  InstalledScriptsManager* GetInstalledScriptsManager() override;

 private:
  Persistent<ServiceWorkerGlobalScopeProxy> global_scope_proxy_;
  std::unique_ptr<WorkerBackingThread> worker_backing_thread_;
  std::unique_ptr<ServiceWorkerInstalledScriptsManager>
      installed_scripts_manager_;
};

}  // namespace blink

#endif  // ServiceWorkerThread_h
