// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_COMMON_DATA_PIPE_DRAINER_H_
#define MOJO_COMMON_DATA_PIPE_DRAINER_H_

#include <stddef.h>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "mojo/common/mojo_common_export.h"
#include "mojo/public/cpp/system/core.h"
#include "mojo/public/cpp/system/simple_watcher.h"

namespace mojo {
namespace common {

class MOJO_COMMON_EXPORT DataPipeDrainer {
 public:
  class Client {
   public:
    virtual void OnDataAvailable(const void* data, size_t num_bytes) = 0;
    virtual void OnDataComplete() = 0;

   protected:
    virtual ~Client() {}
  };

  DataPipeDrainer(Client*, mojo::ScopedDataPipeConsumerHandle source);
  ~DataPipeDrainer();

 private:
  void ReadData();
  void WaitComplete(MojoResult result);

  Client* client_;
  mojo::ScopedDataPipeConsumerHandle source_;
  mojo::SimpleWatcher handle_watcher_;

  base::WeakPtrFactory<DataPipeDrainer> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(DataPipeDrainer);
};

}  // namespace common
}  // namespace mojo

#endif  // MOJO_COMMON_DATA_PIPE_DRAINER_H_
