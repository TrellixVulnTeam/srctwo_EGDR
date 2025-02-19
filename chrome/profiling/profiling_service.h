// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_PROFILING_PROFILING_SERVICE_H_
#define CHROME_PROFILING_PROFILING_SERVICE_H_

#include "base/memory/weak_ptr.h"
#include "chrome/common/profiling/memlog.mojom.h"
#include "mojo/public/cpp/bindings/binding_set.h"
#include "services/service_manager/public/cpp/binder_registry.h"
#include "services/service_manager/public/cpp/service_context.h"
#include "services/service_manager/public/cpp/service_context_ref.h"

namespace profiling {

class MemlogImpl;

// Service implementation for Profiling. This will be called in the profiling
// process (which is a sandboxed utility process created on demand by the
// ServiceManager) to set manage the global state as well as the bound
// interface.
//
// Currently the service only manages a memlog interface, but there could be
// more in the future.
//
// The expectation is each memlog sender will bind a connection to this service
// and request a pipe.
//
// This class lives in the UI thread of the Utility process.
class ProfilingService : public service_manager::Service {
 public:
  ProfilingService();
  ~ProfilingService() override;

  // Factory method for creating the service. Used by the ServiceManager piping
  // to instantiate this thing.
  static std::unique_ptr<service_manager::Service> CreateService();

  // Lifescycle events that occur after the service has started to spinup.
  void OnStart() override;
  void OnBindInterface(const service_manager::BindSourceInfo& source_info,
                       const std::string& interface_name,
                       mojo::ScopedMessagePipeHandle interface_pipe) override;

 private:
  void MaybeRequestQuitDelayed();
  void MaybeRequestQuit();

  // Uses |binding_set_| to resolve |request| allowing for on instance of
  // MemlogImpl to serve all interface bindings.
  void OnMemlogRequest(service_manager::ServiceContextRefFactory* ref_factory,
                       mojom::MemlogRequest request);

  // State needed to manage service lifecycle and lifecycle of bound clients.
  std::unique_ptr<service_manager::ServiceContextRefFactory> ref_factory_;
  std::unique_ptr<MemlogImpl> memlog_impl_;
  service_manager::BinderRegistry registry_;
  mojo::BindingSet<mojom::Memlog> binding_set_;
  base::WeakPtrFactory<ProfilingService> weak_factory_;
};

}  // namespace profiling

#endif  // CHROME_PROFILING_PROFILING_SERVICE_H_
