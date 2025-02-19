/*
 * Copyright (C) 1997 Martin Jones (mjones@kde.org)
 *           (C) 1997 Torben Weis (weis@kde.org)
 *           (C) 1998 Waldo Bastian (bastian@kde.org)
 *           (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003, 2004, 2005, 2006, 2010 Apple Inc. All rights reserved.
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

#ifndef HTMLTableColElement_h
#define HTMLTableColElement_h

#include "core/html/HTMLTablePartElement.h"

namespace blink {

class HTMLTableColElement final : public HTMLTablePartElement {
  DEFINE_WRAPPERTYPEINFO();

 public:
  DECLARE_ELEMENT_FACTORY_WITH_TAGNAME(HTMLTableColElement);

  unsigned span() const { return span_; }
  void setSpan(unsigned);

  const AtomicString& Width() const;

 private:
  HTMLTableColElement(const QualifiedName& tag_name, Document&);

  void ParseAttribute(const AttributeModificationParams&) override;
  bool IsPresentationAttribute(const QualifiedName&) const override;
  void CollectStyleForPresentationAttribute(const QualifiedName&,
                                            const AtomicString&,
                                            MutableStylePropertySet*) override;
  const StylePropertySet* AdditionalPresentationAttributeStyle() override;

  unsigned span_;
};

inline bool IsHTMLTableColElement(const HTMLElement& element) {
  return element.HasTagName(HTMLNames::colTag) ||
         element.HasTagName(HTMLNames::colgroupTag);
}

DEFINE_HTMLELEMENT_TYPE_CASTS_WITH_FUNCTION(HTMLTableColElement);

}  // namespace blink

#endif  // HTMLTableColElement_h
