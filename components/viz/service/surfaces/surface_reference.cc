// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/viz/service/surfaces/surface_reference.h"

#include "base/strings/stringprintf.h"

namespace viz {

SurfaceReference::SurfaceReference() = default;

SurfaceReference::SurfaceReference(const SurfaceId& parent_id,
                                   const SurfaceId& child_id)
    : parent_id_(parent_id), child_id_(child_id) {}

SurfaceReference::SurfaceReference(const SurfaceReference& other) = default;

SurfaceReference::~SurfaceReference() = default;

std::string SurfaceReference::ToString() const {
  return base::StringPrintf("parent=%s, child=%s",
                            parent_id_.ToString().c_str(),
                            child_id_.ToString().c_str());
}

}  // namespace viz
