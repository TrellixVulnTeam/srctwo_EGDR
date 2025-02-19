/*
 * Copyright (C) 2012 Adobe Systems Incorporated. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "core/paint/PaintLayerResourceInfo.h"

#include "core/paint/PaintLayer.h"
#include "platform/graphics/filters/FilterEffect.h"

namespace blink {

PaintLayerResourceInfo::PaintLayerResourceInfo(PaintLayer* layer)
    : layer_(layer) {}

PaintLayerResourceInfo::~PaintLayerResourceInfo() {
  DCHECK(!layer_);
}

TreeScope* PaintLayerResourceInfo::GetTreeScope() {
  DCHECK(layer_);
  Node* node = layer_->GetLayoutObject().GetNode();
  return node ? &node->GetTreeScope() : nullptr;
}

void PaintLayerResourceInfo::ResourceContentChanged() {
  DCHECK(layer_);
  LayoutObject& layout_object = layer_->GetLayoutObject();
  layout_object.SetShouldDoFullPaintInvalidation();
  // The effect paint property nodes depend on SVG filters so we need
  // to update these properties when filter resources change.
  layout_object.SetNeedsPaintPropertyUpdate();
  const ComputedStyle& style = layout_object.StyleRef();
  if (style.HasFilter() && style.Filter().HasReferenceFilter())
    InvalidateFilterChain();
}

void PaintLayerResourceInfo::ResourceElementChanged() {
  DCHECK(layer_);
  LayoutObject& layout_object = layer_->GetLayoutObject();
  layout_object.SetShouldDoFullPaintInvalidation();
  // The effect paint property nodes depend on SVG filters so we need
  // to update these properties when filter resources change.
  layout_object.SetNeedsPaintPropertyUpdate();
  const ComputedStyle& style = layout_object.StyleRef();
  if (style.HasFilter() && style.Filter().HasReferenceFilter())
    InvalidateFilterChain();
}

void PaintLayerResourceInfo::SetLastEffect(FilterEffect* last_effect) {
  last_effect_ = last_effect;
}

FilterEffect* PaintLayerResourceInfo::LastEffect() const {
  return last_effect_;
}

void PaintLayerResourceInfo::InvalidateFilterChain() {
  last_effect_ = nullptr;
}

DEFINE_TRACE(PaintLayerResourceInfo) {
  visitor->Trace(last_effect_);
  SVGResourceClient::Trace(visitor);
}

}  // namespace blink
