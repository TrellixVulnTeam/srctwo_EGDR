/*
 * Copyright (C) 2004, 2005, 2006, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2008 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2008 Dirk Schulze <krit@webkit.org>
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
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

#include "core/svg/SVGLinearGradientElement.h"

#include "core/layout/svg/LayoutSVGResourceLinearGradient.h"
#include "core/svg/LinearGradientAttributes.h"
#include "core/svg/SVGLength.h"

namespace blink {

inline SVGLinearGradientElement::SVGLinearGradientElement(Document& document)
    : SVGGradientElement(SVGNames::linearGradientTag, document),
      x1_(SVGAnimatedLength::Create(this,
                                    SVGNames::x1Attr,
                                    SVGLength::Create(SVGLengthMode::kWidth))),
      y1_(SVGAnimatedLength::Create(this,
                                    SVGNames::y1Attr,
                                    SVGLength::Create(SVGLengthMode::kHeight))),
      x2_(SVGAnimatedLength::Create(this,
                                    SVGNames::x2Attr,
                                    SVGLength::Create(SVGLengthMode::kWidth))),
      y2_(SVGAnimatedLength::Create(
          this,
          SVGNames::y2Attr,
          SVGLength::Create(SVGLengthMode::kHeight))) {
  // Spec: If the x1|y1|y2 attribute is not specified, the effect is as if a
  // value of "0%" were specified.
  x1_->SetDefaultValueAsString("0%");
  y1_->SetDefaultValueAsString("0%");
  y2_->SetDefaultValueAsString("0%");

  // Spec: If the x2 attribute is not specified, the effect is as if a value of
  // "100%" were specified.
  x2_->SetDefaultValueAsString("100%");

  AddToPropertyMap(x1_);
  AddToPropertyMap(y1_);
  AddToPropertyMap(x2_);
  AddToPropertyMap(y2_);
}

DEFINE_TRACE(SVGLinearGradientElement) {
  visitor->Trace(x1_);
  visitor->Trace(y1_);
  visitor->Trace(x2_);
  visitor->Trace(y2_);
  SVGGradientElement::Trace(visitor);
}

DEFINE_NODE_FACTORY(SVGLinearGradientElement)

void SVGLinearGradientElement::SvgAttributeChanged(
    const QualifiedName& attr_name) {
  if (attr_name == SVGNames::x1Attr || attr_name == SVGNames::x2Attr ||
      attr_name == SVGNames::y1Attr || attr_name == SVGNames::y2Attr) {
    SVGElement::InvalidationGuard invalidation_guard(this);

    UpdateRelativeLengthsInformation();

    LayoutSVGResourceContainer* layout_object =
        ToLayoutSVGResourceContainer(this->GetLayoutObject());
    if (layout_object)
      layout_object->InvalidateCacheAndMarkForLayout();

    return;
  }

  SVGGradientElement::SvgAttributeChanged(attr_name);
}

LayoutObject* SVGLinearGradientElement::CreateLayoutObject(
    const ComputedStyle&) {
  return new LayoutSVGResourceLinearGradient(this);
}

static void SetGradientAttributes(const SVGGradientElement& element,
                                  LinearGradientAttributes& attributes,
                                  bool is_linear) {
  element.SynchronizeAnimatedSVGAttribute(AnyQName());
  element.CollectCommonAttributes(attributes);

  if (!is_linear)
    return;
  const SVGLinearGradientElement& linear = toSVGLinearGradientElement(element);

  if (!attributes.HasX1() && linear.x1()->IsSpecified())
    attributes.SetX1(linear.x1()->CurrentValue());

  if (!attributes.HasY1() && linear.y1()->IsSpecified())
    attributes.SetY1(linear.y1()->CurrentValue());

  if (!attributes.HasX2() && linear.x2()->IsSpecified())
    attributes.SetX2(linear.x2()->CurrentValue());

  if (!attributes.HasY2() && linear.y2()->IsSpecified())
    attributes.SetY2(linear.y2()->CurrentValue());
}

bool SVGLinearGradientElement::CollectGradientAttributes(
    LinearGradientAttributes& attributes) {
  DCHECK(GetLayoutObject());

  VisitedSet visited;
  const SVGGradientElement* current = this;

  while (true) {
    SetGradientAttributes(*current, attributes,
                          isSVGLinearGradientElement(*current));
    visited.insert(current);

    current = current->ReferencedElement();
    if (!current || visited.Contains(current))
      break;
    if (!current->GetLayoutObject())
      return false;
  }
  return true;
}

bool SVGLinearGradientElement::SelfHasRelativeLengths() const {
  return x1_->CurrentValue()->IsRelative() ||
         y1_->CurrentValue()->IsRelative() ||
         x2_->CurrentValue()->IsRelative() || y2_->CurrentValue()->IsRelative();
}

}  // namespace blink
