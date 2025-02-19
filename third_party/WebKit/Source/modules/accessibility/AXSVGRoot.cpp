/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "modules/accessibility/AXSVGRoot.h"

#include "modules/accessibility/AXObjectCacheImpl.h"

namespace blink {

AXSVGRoot::AXSVGRoot(LayoutObject* layout_object,
                     AXObjectCacheImpl& ax_object_cache)
    : AXLayoutObject(layout_object, ax_object_cache) {}

AXSVGRoot::~AXSVGRoot() {}

AXSVGRoot* AXSVGRoot::Create(LayoutObject* layout_object,
                             AXObjectCacheImpl& ax_object_cache) {
  return new AXSVGRoot(layout_object, ax_object_cache);
}

void AXSVGRoot::SetParent(AXObject* parent) {
  // Only update the parent to another objcet if it wasn't already set to
  // something. Multiple elements in an HTML document can reference
  // the same remote SVG document, and in that case the parent should just
  // stay with the first one.
  if (!parent_ || !parent)
    parent_ = parent;
}

AXObject* AXSVGRoot::ComputeParent() const {
  DCHECK(!IsDetached());
  // If a parent was set because this is a remote SVG resource, use that
  // but otherwise, we should rely on the standard layout tree for the parent.
  if (parent_)
    return parent_;

  return AXLayoutObject::ComputeParent();
}

// SVG AAM 1.0 S8.2: the default role for an SVG root is "group".
AccessibilityRole AXSVGRoot::DetermineAccessibilityRole() {
  AccessibilityRole role = AXLayoutObject::DetermineAccessibilityRole();
  if (role == kUnknownRole)
    role = kGroupRole;
  return role;
}

// SVG elements are only ignored if they are a descendant of a leaf or when a
// generic element would also be ignored.
bool AXSVGRoot::ComputeAccessibilityIsIgnored(IgnoredReasons* reasons) const {
  if (IsDescendantOfLeafNode())
    return AXLayoutObject::ComputeAccessibilityIsIgnored(reasons);

  return AccessibilityIsIgnoredByDefault(reasons);
}

}  // namespace blink
