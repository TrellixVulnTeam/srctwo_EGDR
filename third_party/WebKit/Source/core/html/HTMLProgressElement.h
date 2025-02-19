/*
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
 *
 */

#ifndef HTMLProgressElement_h
#define HTMLProgressElement_h

#include "core/CoreExport.h"
#include "core/html/LabelableElement.h"

namespace blink {

class LayoutProgress;

class CORE_EXPORT HTMLProgressElement final : public LabelableElement {
  DEFINE_WRAPPERTYPEINFO();

 public:
  static const double kIndeterminatePosition;
  static const double kInvalidPosition;

  static HTMLProgressElement* Create(Document&);

  double value() const;
  void setValue(double);

  double max() const;
  void setMax(double);

  double position() const;

  bool CanContainRangeEndPoint() const override { return false; }

  DECLARE_VIRTUAL_TRACE();

 private:
  explicit HTMLProgressElement(Document&);
  ~HTMLProgressElement() override;

  bool AreAuthorShadowsAllowed() const override { return false; }
  bool ShouldAppearIndeterminate() const override;
  bool SupportLabels() const override { return true; }

  LayoutObject* CreateLayoutObject(const ComputedStyle&) override;
  LayoutProgress* GetLayoutProgress() const;

  void ParseAttribute(const AttributeModificationParams&) override;

  void AttachLayoutTree(AttachContext&) override;

  void DidElementStateChange();
  void DidAddUserAgentShadowRoot(ShadowRoot&) override;
  bool IsDeterminate() const;
  void SetValueWidthPercentage(double) const;

  Member<Element> value_;
};

}  // namespace blink

#endif  // HTMLProgressElement_h
