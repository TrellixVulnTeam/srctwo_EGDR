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

#ifndef SVGFETurbulenceElement_h
#define SVGFETurbulenceElement_h

#include "core/svg/SVGAnimatedEnumeration.h"
#include "core/svg/SVGAnimatedInteger.h"
#include "core/svg/SVGAnimatedNumber.h"
#include "core/svg/SVGAnimatedNumberOptionalNumber.h"
#include "core/svg/SVGFilterPrimitiveStandardAttributes.h"
#include "platform/graphics/filters/FETurbulence.h"
#include "platform/heap/Handle.h"

namespace blink {

enum SVGStitchOptions {
  kSvgStitchtypeUnknown = 0,
  kSvgStitchtypeStitch = 1,
  kSvgStitchtypeNostitch = 2
};
template <>
const SVGEnumerationStringEntries& GetStaticStringEntries<SVGStitchOptions>();

template <>
const SVGEnumerationStringEntries& GetStaticStringEntries<TurbulenceType>();

class SVGFETurbulenceElement final
    : public SVGFilterPrimitiveStandardAttributes {
  DEFINE_WRAPPERTYPEINFO();

 public:
  DECLARE_NODE_FACTORY(SVGFETurbulenceElement);

  SVGAnimatedNumber* baseFrequencyX() { return base_frequency_->FirstNumber(); }
  SVGAnimatedNumber* baseFrequencyY() {
    return base_frequency_->SecondNumber();
  }
  SVGAnimatedNumber* seed() { return seed_.Get(); }
  SVGAnimatedEnumeration<SVGStitchOptions>* stitchTiles() {
    return stitch_tiles_.Get();
  }
  SVGAnimatedEnumeration<TurbulenceType>* type() { return type_.Get(); }
  SVGAnimatedInteger* numOctaves() { return num_octaves_.Get(); }

  DECLARE_VIRTUAL_TRACE();

 private:
  explicit SVGFETurbulenceElement(Document&);

  bool SetFilterEffectAttribute(FilterEffect*,
                                const QualifiedName& attr_name) override;
  void SvgAttributeChanged(const QualifiedName&) override;
  FilterEffect* Build(SVGFilterBuilder*, Filter*) override;

  Member<SVGAnimatedNumberOptionalNumber> base_frequency_;
  Member<SVGAnimatedNumber> seed_;
  Member<SVGAnimatedEnumeration<SVGStitchOptions>> stitch_tiles_;
  Member<SVGAnimatedEnumeration<TurbulenceType>> type_;
  Member<SVGAnimatedInteger> num_octaves_;
};

}  // namespace blink

#endif  // SVGFETurbulenceElement_h
