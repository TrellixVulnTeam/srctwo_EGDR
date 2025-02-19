// Copyright (c) 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_VIZ_COMMON_GPU_CONTEXT_CACHE_CONTROLLER_H_
#define COMPONENTS_VIZ_COMMON_GPU_CONTEXT_CACHE_CONTROLLER_H_

#include <cstdint>
#include <memory>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/single_thread_task_runner.h"
#include "components/viz/common/viz_common_export.h"

class GrContext;

namespace base {
class Lock;
}

namespace gpu {
class ContextSupport;
}

namespace viz {

// ContextCacheController manages clearing cached data on ContextProvider when
// appropriate. Currently, cache clearing is triggered when the Context
// provider transitions from Visible to Not Visible, or from Busy to Idle. As a
// ContextProvider may have multiple clients, ContextCacheController tracks
// visibility and idle status across all clients and only cleans up when
// appropriate.
class VIZ_COMMON_EXPORT ContextCacheController {
 public:
  class VIZ_COMMON_EXPORT ScopedToken {
   public:
    ~ScopedToken();

   private:
    friend class ContextCacheController;
    ScopedToken();
    void Release();

    bool released_ = false;
  };
  using ScopedVisibility = ScopedToken;
  using ScopedBusy = ScopedToken;

  ContextCacheController(
      gpu::ContextSupport* context_support,
      scoped_refptr<base::SingleThreadTaskRunner> task_runner);
  virtual ~ContextCacheController();

  void SetGrContext(GrContext* gr_context);
  void SetLock(base::Lock* lock);

  // Clients of the owning ContextProvider should call this function when they
  // become visible. The returned ScopedVisibility pointer must be passed back
  // to ClientBecameNotVisible or it will DCHECK in its destructor.
  virtual std::unique_ptr<ScopedVisibility> ClientBecameVisible();

  // When a client becomes not visible (either due to a visibility change or
  // because it is being deleted), it must pass back any ScopedVisibility
  // pointers it owns via this function.
  virtual void ClientBecameNotVisible(
      std::unique_ptr<ScopedVisibility> scoped_visibility);

  // Clients of the owning ContextProvider may call this function when they
  // become busy. The returned ScopedBusy pointer must be passed back
  // to ClientBecameNotBusy or it will DCHECK in its destructor.
  std::unique_ptr<ScopedBusy> ClientBecameBusy();

  // When a client becomes not busy, it must pass back any ScopedBusy
  // pointers it owns via this function.
  void ClientBecameNotBusy(std::unique_ptr<ScopedBusy> scoped_busy);

 private:
  void OnIdle(uint32_t idle_generation);
  void PostIdleCallback(uint32_t current_idle_generation) const;
  void InvalidatePendingIdleCallbacks();

  gpu::ContextSupport* context_support_;
  scoped_refptr<base::SingleThreadTaskRunner> task_runner_;
  GrContext* gr_context_ = nullptr;

  // If set, |context_lock_| must be held before accessing any member within
  // the idle callback. Exceptions to this are |current_idle_generation_|,
  // which has its own lock, and weak_ptr_ and task_runner_, which may be
  // accessed from multiple threads without locking.
  base::Lock* context_lock_ = nullptr;

  uint32_t num_clients_visible_ = 0;
  uint32_t num_clients_busy_ = 0;
  bool callback_pending_ = false;

  // |current_idle_generation_lock_| must be held when accessing
  // |current_idle_generation_|. |current_idle_generation_lock_| must never be
  // held while acquiring |context_lock_|.
  base::Lock current_idle_generation_lock_;
  uint32_t current_idle_generation_ = 0;

  base::WeakPtr<ContextCacheController> weak_ptr_;
  base::WeakPtrFactory<ContextCacheController> weak_factory_;
};

}  // namespace viz

#endif  // COMPONENTS_VIZ_COMMON_GPU_CONTEXT_CACHE_CONTROLLER_H_
