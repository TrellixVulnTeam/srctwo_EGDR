// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "modules/csspaint/PaintWorkletPendingGeneratorRegistry.h"

#include "modules/csspaint/CSSPaintDefinition.h"

namespace blink {

void PaintWorkletPendingGeneratorRegistry::NotifyGeneratorReady(
    const String& name) {
  GeneratorHashSet* set = pending_generators_.at(name);
  if (set) {
    for (const auto& generator : *set) {
      if (generator)
        generator->NotifyGeneratorReady();
    }
  }
  pending_generators_.erase(name);
}

void PaintWorkletPendingGeneratorRegistry::AddPendingGenerator(
    const String& name,
    CSSPaintImageGeneratorImpl* generator) {
  Member<GeneratorHashSet>& set =
      pending_generators_.insert(name, nullptr).stored_value->value;
  if (!set)
    set = new GeneratorHashSet;
  set->insert(generator);
}

DEFINE_TRACE(PaintWorkletPendingGeneratorRegistry) {
  visitor->Trace(pending_generators_);
}

}  // namespace blink
