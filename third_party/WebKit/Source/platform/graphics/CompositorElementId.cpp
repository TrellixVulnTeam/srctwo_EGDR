// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/graphics/CompositorElementId.h"

#include "platform/RuntimeEnabledFeatures.h"

namespace blink {

UniqueObjectId NewUniqueObjectId() {
  static UniqueObjectId counter = 0;
  return ++counter;
}

static CompositorElementId CreateCompositorElementId(
    uint64_t blink_id,
    CompositorElementIdNamespace namespace_id) {
  DCHECK(
      blink_id > 0 &&
      blink_id <
          std::numeric_limits<uint64_t>::max() /
              static_cast<unsigned>(
                  CompositorElementIdNamespace::kMaxRepresentableNamespaceId));
  // Shift to make room for namespace_id enum bits.
  cc::ElementIdType id = blink_id << kCompositorNamespaceBitCount;
  id += static_cast<uint64_t>(namespace_id);
  return CompositorElementId(id);
}

CompositorElementId PLATFORM_EXPORT CompositorElementIdFromUniqueObjectId(
    UniqueObjectId id,
    CompositorElementIdNamespace namespace_id) {
  DCHECK(namespace_id == CompositorElementIdNamespace::kPrimary ||
         namespace_id == CompositorElementIdNamespace::kScroll ||
         namespace_id == CompositorElementIdNamespace::kEffectFilter ||
         namespace_id == CompositorElementIdNamespace::kEffectMask);
  return CreateCompositorElementId(id, namespace_id);
}

CompositorElementId PLATFORM_EXPORT
CompositorElementIdFromDOMNodeId(DOMNodeId id) {
  return CreateCompositorElementId(
      id, CompositorElementIdNamespace::kUniqueObjectId);
}

CompositorElementId PLATFORM_EXPORT
CompositorElementIdFromUniqueObjectId(UniqueObjectId id) {
  return CreateCompositorElementId(
      id, CompositorElementIdNamespace::kUniqueObjectId);
}

CompositorElementIdNamespace NamespaceFromCompositorElementId(
    CompositorElementId element_id) {
  return static_cast<CompositorElementIdNamespace>(
      element_id.ToInternalValue() %
      static_cast<uint64_t>(
          CompositorElementIdNamespace::kMaxRepresentableNamespaceId));
}

}  // namespace blink
