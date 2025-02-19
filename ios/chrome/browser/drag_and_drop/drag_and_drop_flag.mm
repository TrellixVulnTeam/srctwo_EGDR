// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/chrome/browser/drag_and_drop/drag_and_drop_flag.h"

const base::Feature kDragAndDrop{"DragAndDrop",
                                 base::FEATURE_ENABLED_BY_DEFAULT};

bool DragAndDropIsEnabled() {
  return base::FeatureList::IsEnabled(kDragAndDrop);
}
