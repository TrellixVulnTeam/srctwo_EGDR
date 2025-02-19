/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "core/animation/EffectStack.h"

#include <algorithm>
#include "core/animation/CompositorAnimations.h"
#include "core/animation/InvalidatableInterpolation.h"
#include "core/animation/css/CSSAnimations.h"
#include "platform/RuntimeEnabledFeatures.h"
#include "platform/wtf/NonCopyingSort.h"

namespace blink {

namespace {

void CopyToActiveInterpolationsMap(
    const Vector<RefPtr<Interpolation>>& source,
    EffectStack::PropertyHandleFilter property_handle_filter,
    ActiveInterpolationsMap& target) {
  for (const auto& interpolation : source) {
    PropertyHandle property = interpolation->GetProperty();
    if (property_handle_filter && !property_handle_filter(property))
      continue;
    ActiveInterpolationsMap::AddResult entry =
        target.insert(property, ActiveInterpolations(1));
    ActiveInterpolations& active_interpolations = entry.stored_value->value;
    if (!entry.is_new_entry &&
        (RuntimeEnabledFeatures::StackedCSSPropertyAnimationsEnabled() ||
         !property.IsCSSProperty() || property.IsPresentationAttribute()) &&
        interpolation->IsInvalidatableInterpolation() &&
        ToInvalidatableInterpolation(*interpolation)
            .DependsOnUnderlyingValue()) {
      active_interpolations.push_back(interpolation.Get());
    } else {
      active_interpolations.at(0) = interpolation.Get();
    }
  }
}

bool CompareSampledEffects(const Member<SampledEffect>& sampled_effect1,
                           const Member<SampledEffect>& sampled_effect2) {
  DCHECK(sampled_effect1 && sampled_effect2);
  return sampled_effect1->SequenceNumber() < sampled_effect2->SequenceNumber();
}

void CopyNewAnimationsToActiveInterpolationsMap(
    const HeapVector<Member<const InertEffect>>& new_animations,
    EffectStack::PropertyHandleFilter property_handle_filter,
    ActiveInterpolationsMap& result) {
  for (const auto& new_animation : new_animations) {
    Vector<RefPtr<Interpolation>> sample;
    new_animation->Sample(sample);
    if (!sample.IsEmpty())
      CopyToActiveInterpolationsMap(sample, property_handle_filter, result);
  }
}

}  // namespace

EffectStack::EffectStack() {}

bool EffectStack::HasActiveAnimationsOnCompositor(
    const PropertyHandle& property) const {
  for (const auto& sampled_effect : sampled_effects_) {
    // TODO(dstockwell): move the playing check into AnimationEffectReadOnly and
    // expose both hasAnimations and hasActiveAnimations
    if (sampled_effect->Effect() &&
        sampled_effect->Effect()->GetAnimation()->Playing() &&
        sampled_effect->Effect()->HasActiveAnimationsOnCompositor(property))
      return true;
  }
  return false;
}

bool EffectStack::AffectsProperties(PropertyHandleFilter filter) const {
  for (const auto& sampled_effect : sampled_effects_) {
    for (const auto& interpolation : sampled_effect->Interpolations()) {
      if (filter(interpolation->GetProperty()))
        return true;
    }
  }
  return false;
}

ActiveInterpolationsMap EffectStack::ActiveInterpolations(
    EffectStack* effect_stack,
    const HeapVector<Member<const InertEffect>>* new_animations,
    const HeapHashSet<Member<const Animation>>* suppressed_animations,
    KeyframeEffectReadOnly::Priority priority,
    PropertyHandleFilter property_handle_filter) {
  ActiveInterpolationsMap result;

  if (effect_stack) {
    HeapVector<Member<SampledEffect>>& sampled_effects =
        effect_stack->sampled_effects_;
    // std::sort doesn't work with OwnPtrs
    NonCopyingSort(sampled_effects.begin(), sampled_effects.end(),
                   CompareSampledEffects);
    effect_stack->RemoveRedundantSampledEffects();
    for (const auto& sampled_effect : sampled_effects) {
      if (sampled_effect->GetPriority() != priority ||
          (suppressed_animations && sampled_effect->Effect() &&
           suppressed_animations->Contains(
               sampled_effect->Effect()->GetAnimation())))
        continue;
      CopyToActiveInterpolationsMap(sampled_effect->Interpolations(),
                                    property_handle_filter, result);
    }
  }

  if (new_animations) {
    CopyNewAnimationsToActiveInterpolationsMap(*new_animations,
                                               property_handle_filter, result);
  }
  return result;
}

void EffectStack::RemoveRedundantSampledEffects() {
  HashSet<PropertyHandle> replaced_properties;
  for (size_t i = sampled_effects_.size(); i--;) {
    SampledEffect& sampled_effect = *sampled_effects_[i];
    if (sampled_effect.WillNeverChange()) {
      sampled_effect.RemoveReplacedInterpolations(replaced_properties);
      sampled_effect.UpdateReplacedProperties(replaced_properties);
    }
  }

  size_t new_size = 0;
  for (auto& sampled_effect : sampled_effects_) {
    if (!sampled_effect->Interpolations().IsEmpty())
      sampled_effects_[new_size++].Swap(sampled_effect);
    else if (sampled_effect->Effect())
      sampled_effect->Effect()->NotifySampledEffectRemovedFromEffectStack();
  }
  sampled_effects_.Shrink(new_size);
}

DEFINE_TRACE(EffectStack) {
  visitor->Trace(sampled_effects_);
}

bool EffectStack::GetAnimatedBoundingBox(FloatBox& box,
                                         CSSPropertyID property) const {
  FloatBox original_box(box);
  for (const auto& sampled_effect : sampled_effects_) {
    if (sampled_effect->Effect() &&
        sampled_effect->Effect()->Affects(PropertyHandle(property))) {
      KeyframeEffectReadOnly* effect = sampled_effect->Effect();
      const Timing& timing = effect->SpecifiedTiming();
      double start_range = 0;
      double end_range = 1;
      timing.timing_function->Range(&start_range, &end_range);
      FloatBox expanding_box(original_box);
      if (!CompositorAnimations::GetAnimatedBoundingBox(
              expanding_box, *effect->Model(), start_range, end_range))
        return false;
      box.ExpandTo(expanding_box);
    }
  }
  return true;
}

}  // namespace blink
