// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PPAPI_PROXY_PPB_BUFFER_PROXY_H_
#define PPAPI_PROXY_PPB_BUFFER_PROXY_H_

#include <stdint.h>

#include "base/macros.h"
#include "base/memory/shared_memory.h"
#include "ppapi/c/pp_instance.h"
#include "ppapi/proxy/interface_proxy.h"
#include "ppapi/shared_impl/resource.h"
#include "ppapi/thunk/ppb_buffer_api.h"

namespace ppapi {

class HostResource;

namespace proxy {

class SerializedHandle;

class Buffer : public thunk::PPB_Buffer_API, public Resource {
 public:
  Buffer(const HostResource& resource,
         const base::SharedMemoryHandle& shm_handle,
         uint32_t size);
  ~Buffer() override;

  // Resource overrides.
  thunk::PPB_Buffer_API* AsPPB_Buffer_API() override;

  // PPB_Buffer_API implementation.
  PP_Bool Describe(uint32_t* size_in_bytes) override;
  PP_Bool IsMapped() override;
  void* Map() override;
  void Unmap() override;

  // Trusted
  int32_t GetSharedMemory(base::SharedMemory** shm) override;

 private:
  base::SharedMemory shm_;
  uint32_t size_;
  int map_count_;

  DISALLOW_COPY_AND_ASSIGN(Buffer);
};

class PPB_Buffer_Proxy : public InterfaceProxy {
 public:
  explicit PPB_Buffer_Proxy(Dispatcher* dispatcher);
  ~PPB_Buffer_Proxy() override;

  static PP_Resource CreateProxyResource(PP_Instance instance,
                                         uint32_t size);
  static PP_Resource AddProxyResource(const HostResource& resource,
                                      base::SharedMemoryHandle shm_handle,
                                      uint32_t size);

  // InterfaceProxy implementation.
  bool OnMessageReceived(const IPC::Message& msg) override;

  static const ApiID kApiID = API_ID_PPB_BUFFER;

 private:
  // Message handlers.
  void OnMsgCreate(PP_Instance instance,
                   uint32_t size,
                   HostResource* result_resource,
                   ppapi::proxy::SerializedHandle* result_shm_handle);
};

}  // namespace proxy
}  // namespace ppapi

#endif  // PPAPI_PROXY_PPB_BUFFER_PROXY_H_
