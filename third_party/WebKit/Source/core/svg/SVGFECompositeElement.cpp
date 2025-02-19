/*
 * Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "core/svg/SVGFECompositeElement.h"

#include "core/SVGNames.h"
#include "core/svg/graphics/filters/SVGFilterBuilder.h"

namespace blink {

template <>
const SVGEnumerationStringEntries&
GetStaticStringEntries<CompositeOperationType>() {
  DEFINE_STATIC_LOCAL(SVGEnumerationStringEntries, entries, ());
  if (entries.IsEmpty()) {
    entries.push_back(std::make_pair(FECOMPOSITE_OPERATOR_OVER, "over"));
    entries.push_back(std::make_pair(FECOMPOSITE_OPERATOR_IN, "in"));
    entries.push_back(std::make_pair(FECOMPOSITE_OPERATOR_OUT, "out"));
    entries.push_back(std::make_pair(FECOMPOSITE_OPERATOR_ATOP, "atop"));
    entries.push_back(std::make_pair(FECOMPOSITE_OPERATOR_XOR, "xor"));
    entries.push_back(
        std::make_pair(FECOMPOSITE_OPERATOR_ARITHMETIC, "arithmetic"));
    entries.push_back(std::make_pair(FECOMPOSITE_OPERATOR_LIGHTER, "lighter"));
  }
  return entries;
}

template <>
unsigned short GetMaxExposedEnumValue<CompositeOperationType>() {
  return FECOMPOSITE_OPERATOR_ARITHMETIC;
}

inline SVGFECompositeElement::SVGFECompositeElement(Document& document)
    : SVGFilterPrimitiveStandardAttributes(SVGNames::feCompositeTag, document),
      k1_(SVGAnimatedNumber::Create(this,
                                    SVGNames::k1Attr,
                                    SVGNumber::Create())),
      k2_(SVGAnimatedNumber::Create(this,
                                    SVGNames::k2Attr,
                                    SVGNumber::Create())),
      k3_(SVGAnimatedNumber::Create(this,
                                    SVGNames::k3Attr,
                                    SVGNumber::Create())),
      k4_(SVGAnimatedNumber::Create(this,
                                    SVGNames::k4Attr,
                                    SVGNumber::Create())),
      in1_(SVGAnimatedString::Create(this, SVGNames::inAttr)),
      in2_(SVGAnimatedString::Create(this, SVGNames::in2Attr)),
      svg_operator_(SVGAnimatedEnumeration<CompositeOperationType>::Create(
          this,
          SVGNames::operatorAttr,
          FECOMPOSITE_OPERATOR_OVER)) {
  AddToPropertyMap(k1_);
  AddToPropertyMap(k2_);
  AddToPropertyMap(k3_);
  AddToPropertyMap(k4_);
  AddToPropertyMap(in1_);
  AddToPropertyMap(in2_);
  AddToPropertyMap(svg_operator_);
}

DEFINE_TRACE(SVGFECompositeElement) {
  visitor->Trace(k1_);
  visitor->Trace(k2_);
  visitor->Trace(k3_);
  visitor->Trace(k4_);
  visitor->Trace(in1_);
  visitor->Trace(in2_);
  visitor->Trace(svg_operator_);
  SVGFilterPrimitiveStandardAttributes::Trace(visitor);
}

DEFINE_NODE_FACTORY(SVGFECompositeElement)

bool SVGFECompositeElement::SetFilterEffectAttribute(
    FilterEffect* effect,
    const QualifiedName& attr_name) {
  FEComposite* composite = static_cast<FEComposite*>(effect);
  if (attr_name == SVGNames::operatorAttr)
    return composite->SetOperation(svg_operator_->CurrentValue()->EnumValue());
  if (attr_name == SVGNames::k1Attr)
    return composite->SetK1(k1_->CurrentValue()->Value());
  if (attr_name == SVGNames::k2Attr)
    return composite->SetK2(k2_->CurrentValue()->Value());
  if (attr_name == SVGNames::k3Attr)
    return composite->SetK3(k3_->CurrentValue()->Value());
  if (attr_name == SVGNames::k4Attr)
    return composite->SetK4(k4_->CurrentValue()->Value());

  return SVGFilterPrimitiveStandardAttributes::SetFilterEffectAttribute(
      effect, attr_name);
}

void SVGFECompositeElement::SvgAttributeChanged(
    const QualifiedName& attr_name) {
  if (attr_name == SVGNames::operatorAttr || attr_name == SVGNames::k1Attr ||
      attr_name == SVGNames::k2Attr || attr_name == SVGNames::k3Attr ||
      attr_name == SVGNames::k4Attr) {
    SVGElement::InvalidationGuard invalidation_guard(this);
    PrimitiveAttributeChanged(attr_name);
    return;
  }

  if (attr_name == SVGNames::inAttr || attr_name == SVGNames::in2Attr) {
    SVGElement::InvalidationGuard invalidation_guard(this);
    Invalidate();
    return;
  }

  SVGFilterPrimitiveStandardAttributes::SvgAttributeChanged(attr_name);
}

FilterEffect* SVGFECompositeElement::Build(SVGFilterBuilder* filter_builder,
                                           Filter* filter) {
  FilterEffect* input1 = filter_builder->GetEffectById(
      AtomicString(in1_->CurrentValue()->Value()));
  FilterEffect* input2 = filter_builder->GetEffectById(
      AtomicString(in2_->CurrentValue()->Value()));
  DCHECK(input1);
  DCHECK(input2);

  FilterEffect* effect = FEComposite::Create(
      filter, svg_operator_->CurrentValue()->EnumValue(),
      k1_->CurrentValue()->Value(), k2_->CurrentValue()->Value(),
      k3_->CurrentValue()->Value(), k4_->CurrentValue()->Value());
  FilterEffectVector& input_effects = effect->InputEffects();
  input_effects.ReserveCapacity(2);
  input_effects.push_back(input1);
  input_effects.push_back(input2);
  return effect;
}

}  // namespace blink
