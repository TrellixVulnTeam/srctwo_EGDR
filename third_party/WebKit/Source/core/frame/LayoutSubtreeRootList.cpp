// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/frame/LayoutSubtreeRootList.h"

#include "core/layout/LayoutObject.h"

namespace blink {

void LayoutSubtreeRootList::ClearAndMarkContainingBlocksForLayout() {
  for (auto& iter : Unordered())
    iter->MarkContainerChainForLayout(false);
  Clear();
}

LayoutObject* LayoutSubtreeRootList::RandomRoot() {
  DCHECK(!IsEmpty());
  return *Unordered().begin();
}

void LayoutSubtreeRootList::CountObjectsNeedingLayoutInRoot(
    const LayoutObject* object,
    unsigned& needs_layout_objects,
    unsigned& total_objects) {
  for (const LayoutObject* o = object; o; o = o->NextInPreOrder(object)) {
    ++total_objects;
    if (o->NeedsLayout())
      ++needs_layout_objects;
  }
}

void LayoutSubtreeRootList::CountObjectsNeedingLayout(
    unsigned& needs_layout_objects,
    unsigned& total_objects) {
  // TODO(leviw): This will double-count nested roots crbug.com/509141
  for (auto& root : Unordered())
    CountObjectsNeedingLayoutInRoot(root, needs_layout_objects, total_objects);
}

}  // namespace blink
