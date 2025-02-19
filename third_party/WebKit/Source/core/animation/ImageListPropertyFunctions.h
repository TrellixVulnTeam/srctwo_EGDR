// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ImageListPropertyFunctions_h
#define ImageListPropertyFunctions_h

#include "core/CSSPropertyNames.h"
#include "core/style/ComputedStyle.h"
#include "platform/heap/Handle.h"

namespace blink {

using StyleImageList = PersistentHeapVector<Member<StyleImage>, 1>;

class ImageListPropertyFunctions {
 public:
  static void GetInitialImageList(CSSPropertyID, StyleImageList& result) {
    result.clear();
  }

  static void GetImageList(CSSPropertyID property,
                           const ComputedStyle& style,
                           StyleImageList& result) {
    const FillLayer* fill_layer = nullptr;
    switch (property) {
      case CSSPropertyBackgroundImage:
        fill_layer = &style.BackgroundLayers();
        break;
      case CSSPropertyWebkitMaskImage:
        fill_layer = &style.MaskLayers();
        break;
      default:
        NOTREACHED();
        return;
    }

    result.clear();
    while (fill_layer) {
      result.push_back(fill_layer->GetImage());
      fill_layer = fill_layer->Next();
    }
  }

  static void SetImageList(CSSPropertyID property,
                           ComputedStyle& style,
                           const StyleImageList& image_list) {
    FillLayer* fill_layer = nullptr;
    switch (property) {
      case CSSPropertyBackgroundImage:
        fill_layer = &style.AccessBackgroundLayers();
        break;
      case CSSPropertyWebkitMaskImage:
        fill_layer = &style.AccessMaskLayers();
        break;
      default:
        NOTREACHED();
        return;
    }

    FillLayer* prev = nullptr;
    for (size_t i = 0; i < image_list.size(); i++) {
      if (!fill_layer)
        fill_layer = prev->EnsureNext();
      fill_layer->SetImage(image_list[i]);
      prev = fill_layer;
      fill_layer = fill_layer->Next();
    }
    while (fill_layer) {
      fill_layer->ClearImage();
      fill_layer = fill_layer->Next();
    }
  }
};

}  // namespace blink

#endif  // ImageListPropertyFunctions_h
