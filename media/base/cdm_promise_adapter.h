// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_BASE_CDM_PROMISE_ADAPTER_H_
#define MEDIA_BASE_CDM_PROMISE_ADAPTER_H_

#include <stdint.h>

#include <memory>
#include <unordered_map>

#include "base/macros.h"
#include "base/threading/thread_checker.h"
#include "media/base/cdm_promise.h"
#include "media/base/media_export.h"

namespace media {

// Helps convert CdmPromises to an integer identifier and vice versa. The
// integer identifier is needed where we cannot pass CdmPromises through, such
// as PPAPI, IPC and JNI.
class MEDIA_EXPORT CdmPromiseAdapter {
 public:
  CdmPromiseAdapter();
  ~CdmPromiseAdapter();

  // Takes ownership of |promise| and returns an integer promise ID.
  uint32_t SavePromise(std::unique_ptr<media::CdmPromise> promise);

  // Takes the promise for |promise_id|, sanity checks its |type|, and resolves
  // it with |result|.
  template <typename... T>
  void ResolvePromise(uint32_t promise_id, const T&... result);

  // Takes the promise for |promise_id| and rejects it with |exception_code|,
  // |system_code| and |error_message|.
  void RejectPromise(uint32_t promise_id,
                     CdmPromise::Exception exception_code,
                     uint32_t system_code,
                     const std::string& error_message);

  // Rejects and clears all |promises_|.
  void Clear();

 private:
  // A map between promise IDs and CdmPromises.
  using PromiseMap = std::unordered_map<uint32_t, std::unique_ptr<CdmPromise>>;

  // Finds, takes the ownership of and returns the promise for |promise_id|.
  // Returns null if no promise can be found.
  std::unique_ptr<CdmPromise> TakePromise(uint32_t promise_id);

  uint32_t next_promise_id_;
  PromiseMap promises_;

  base::ThreadChecker thread_checker_;
  DISALLOW_COPY_AND_ASSIGN(CdmPromiseAdapter);
};

}  // namespace media

#endif  // MEDIA_BASE_CDM_PROMISE_ADAPTER_H_
