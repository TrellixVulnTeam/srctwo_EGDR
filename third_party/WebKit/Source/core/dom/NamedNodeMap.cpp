/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Peter Kelly (pmk@post.com)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2013 Apple Inc. All rights
 * reserved.
 *           (C) 2007 Eric Seidel (eric@webkit.org)
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

#include "core/dom/NamedNodeMap.h"

#include "bindings/core/v8/ExceptionState.h"
#include "core/dom/Attr.h"
#include "core/dom/Element.h"
#include "core/dom/ExceptionCode.h"

namespace blink {

using namespace HTMLNames;

Attr* NamedNodeMap::getNamedItem(const AtomicString& name) const {
  return element_->getAttributeNode(name);
}

Attr* NamedNodeMap::getNamedItemNS(const AtomicString& namespace_uri,
                                   const AtomicString& local_name) const {
  return element_->getAttributeNodeNS(namespace_uri, local_name);
}

Attr* NamedNodeMap::removeNamedItem(const AtomicString& name,
                                    ExceptionState& exception_state) {
  size_t index =
      element_->Attributes().FindIndex(element_->LowercaseIfNecessary(name));
  if (index == kNotFound) {
    exception_state.ThrowDOMException(
        kNotFoundError, "No item with name '" + name + "' was found.");
    return nullptr;
  }
  return element_->DetachAttribute(index);
}

Attr* NamedNodeMap::removeNamedItemNS(const AtomicString& namespace_uri,
                                      const AtomicString& local_name,
                                      ExceptionState& exception_state) {
  size_t index = element_->Attributes().FindIndex(
      QualifiedName(g_null_atom, local_name, namespace_uri));
  if (index == kNotFound) {
    exception_state.ThrowDOMException(kNotFoundError,
                                      "No item with name '" + namespace_uri +
                                          "::" + local_name + "' was found.");
    return nullptr;
  }
  return element_->DetachAttribute(index);
}

Attr* NamedNodeMap::setNamedItem(Attr* attr, ExceptionState& exception_state) {
  DCHECK(attr);
  return element_->setAttributeNode(attr, exception_state);
}

Attr* NamedNodeMap::setNamedItemNS(Attr* attr,
                                   ExceptionState& exception_state) {
  DCHECK(attr);
  return element_->setAttributeNodeNS(attr, exception_state);
}

Attr* NamedNodeMap::item(unsigned index) const {
  AttributeCollection attributes = element_->Attributes();
  if (index >= attributes.size())
    return nullptr;
  return element_->EnsureAttr(attributes[index].GetName());
}

size_t NamedNodeMap::length() const {
  return element_->Attributes().size();
}

DEFINE_TRACE(NamedNodeMap) {
  visitor->Trace(element_);
}

}  // namespace blink
