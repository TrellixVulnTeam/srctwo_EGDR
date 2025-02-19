// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/animation/SizeListPropertyFunctions.h"

#include "core/style/ComputedStyle.h"

namespace blink {

static const FillLayer* GetFillLayerForSize(CSSPropertyID property,
                                            const ComputedStyle& style) {
  switch (property) {
    case CSSPropertyBackgroundSize:
      return &style.BackgroundLayers();
    case CSSPropertyWebkitMaskSize:
      return &style.MaskLayers();
    default:
      NOTREACHED();
      return nullptr;
  }
}

static FillLayer* AccessFillLayerForSize(CSSPropertyID property,
                                         ComputedStyle& style) {
  switch (property) {
    case CSSPropertyBackgroundSize:
      return &style.AccessBackgroundLayers();
    case CSSPropertyWebkitMaskSize:
      return &style.AccessMaskLayers();
    default:
      NOTREACHED();
      return nullptr;
  }
}

SizeList SizeListPropertyFunctions::GetInitialSizeList(CSSPropertyID property) {
  return GetSizeList(property, ComputedStyle::InitialStyle());
}

SizeList SizeListPropertyFunctions::GetSizeList(CSSPropertyID property,
                                                const ComputedStyle& style) {
  SizeList result;
  for (const FillLayer* fill_layer = GetFillLayerForSize(property, style);
       fill_layer && fill_layer->IsSizeSet(); fill_layer = fill_layer->Next())
    result.push_back(fill_layer->Size());
  return result;
}

void SizeListPropertyFunctions::SetSizeList(CSSPropertyID property,
                                            ComputedStyle& style,
                                            const SizeList& size_list) {
  FillLayer* fill_layer = AccessFillLayerForSize(property, style);
  FillLayer* prev = nullptr;
  for (const FillSize& size : size_list) {
    if (!fill_layer)
      fill_layer = prev->EnsureNext();
    fill_layer->SetSize(size);
    prev = fill_layer;
    fill_layer = fill_layer->Next();
  }
  while (fill_layer) {
    fill_layer->ClearSize();
    fill_layer = fill_layer->Next();
  }
}

}  // namespace blink
