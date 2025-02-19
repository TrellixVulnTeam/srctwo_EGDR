// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PPAPI_NACL_IRT_MANIFEST_SERVICE_H_
#define PPAPI_NACL_IRT_MANIFEST_SERVICE_H_

#include <memory>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/synchronization/lock.h"

namespace base {
class SingleThreadTaskRunner;
class WaitableEvent;
}  // namespace base

namespace IPC {
struct ChannelHandle;
class ChannelProxy;
class SyncMessageFilter;
}  // namespace IPC

namespace ppapi {

class ManifestService {
 public:
  ManifestService(const IPC::ChannelHandle& handle,
                  scoped_refptr<base::SingleThreadTaskRunner> io_task_runner,
                  base::WaitableEvent* shutdown_event);
  ~ManifestService();

  void StartupInitializationComplete();
  bool OpenResource(const char* file, int* fd);

 private:
  std::unique_ptr<IPC::ChannelProxy> channel_;
  scoped_refptr<IPC::SyncMessageFilter> filter_;

  base::Lock open_resource_lock_;

  DISALLOW_COPY_AND_ASSIGN(ManifestService);
};

}  // namespace ppapi

#endif  // PPAPI_NACL_IRT_MANIFEST_SERVICE_H_
