/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2010 Apple Inc. All rights reserved.
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

#ifndef HTMLFieldSetElement_h
#define HTMLFieldSetElement_h

#include "core/CoreExport.h"
#include "core/html/HTMLFormControlElement.h"

namespace blink {

class HTMLCollection;

class CORE_EXPORT HTMLFieldSetElement final : public HTMLFormControlElement {
  DEFINE_WRAPPERTYPEINFO();

 public:
  static HTMLFieldSetElement* Create(Document&);
  HTMLLegendElement* Legend() const;
  HTMLCollection* elements();

 protected:
  void DisabledAttributeChanged() override;

 private:
  explicit HTMLFieldSetElement(Document&);

  bool IsEnumeratable() const override { return true; }
  bool SupportsFocus() const override;
  LayoutObject* CreateLayoutObject(const ComputedStyle&) override;
  const AtomicString& FormControlType() const override;
  bool RecalcWillValidate() const override { return false; }
  int tabIndex() const final;
  bool MatchesValidityPseudoClasses() const final;
  bool IsValidElement() final;
  void ChildrenChanged(const ChildrenChange&) override;
  bool AreAuthorShadowsAllowed() const override { return false; }
  bool IsSubmittableElement() override;
  bool AlwaysCreateUserAgentShadowRoot() const override { return false; }

  Element* InvalidateDescendantDisabledStateAndFindFocusedOne(Element& base);
};

}  // namespace blink

#endif  // HTMLFieldSetElement_h
