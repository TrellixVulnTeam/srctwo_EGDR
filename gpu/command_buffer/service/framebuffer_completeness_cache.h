// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_COMMAND_BUFFER_SERVICE_FRAMEBUFFER_COMPLETENESS_CACHE_H_
#define GPU_COMMAND_BUFFER_SERVICE_FRAMEBUFFER_COMPLETENESS_CACHE_H_

#include <string>

#include "base/containers/hash_tables.h"
#include "base/macros.h"
#include "gpu/gpu_export.h"

namespace gpu {
namespace gles2 {

// Refcounted wrapper for a hash_set of framebuffer format signatures
// representing framebuffer configurations that are reported by the GL
// driver as complete according to glCheckFramebufferStatusEXT.
class GPU_EXPORT FramebufferCompletenessCache {
 public:
  FramebufferCompletenessCache();
  ~FramebufferCompletenessCache();

  bool IsComplete(const std::string& signature) const;
  void SetComplete(const std::string& signature);

 private:
  typedef base::hash_set<std::string> Map;

  Map cache_;

  DISALLOW_COPY_AND_ASSIGN(FramebufferCompletenessCache);
};

}  // namespace gles2
}  // namespace gpu

#endif  // GPU_COMMAND_BUFFER_SERVICE_FRAMEBUFFER_COMPLETENESS_CACHE_H_
