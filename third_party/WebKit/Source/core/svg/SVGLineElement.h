/*
 * Copyright (C) 2004, 2005, 2006, 2008 Nikolas Zimmermann <zimmermann@kde.org>
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

#ifndef SVGLineElement_h
#define SVGLineElement_h

#include "core/SVGNames.h"
#include "core/svg/SVGAnimatedLength.h"
#include "core/svg/SVGGeometryElement.h"
#include "platform/heap/Handle.h"

namespace blink {

class SVGLineElement final : public SVGGeometryElement {
  DEFINE_WRAPPERTYPEINFO();

 public:
  DECLARE_NODE_FACTORY(SVGLineElement);

  Path AsPath() const override;

  SVGAnimatedLength* x1() const { return x1_.Get(); }
  SVGAnimatedLength* y1() const { return y1_.Get(); }
  SVGAnimatedLength* x2() const { return x2_.Get(); }
  SVGAnimatedLength* y2() const { return y2_.Get(); }

  DECLARE_VIRTUAL_TRACE();

 private:
  explicit SVGLineElement(Document&);

  void SvgAttributeChanged(const QualifiedName&) override;

  bool SelfHasRelativeLengths() const override;

  Member<SVGAnimatedLength> x1_;
  Member<SVGAnimatedLength> y1_;
  Member<SVGAnimatedLength> x2_;
  Member<SVGAnimatedLength> y2_;
};

}  // namespace blink

#endif  // SVGLineElement_h
