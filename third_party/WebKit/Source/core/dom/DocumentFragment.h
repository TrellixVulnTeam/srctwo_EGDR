/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2009 Apple Inc. All rights reserved.
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

#ifndef DocumentFragment_h
#define DocumentFragment_h

#include "core/CoreExport.h"
#include "core/dom/ContainerNode.h"
#include "core/dom/ParserContentPolicy.h"

namespace blink {

class CORE_EXPORT DocumentFragment : public ContainerNode {
  DEFINE_WRAPPERTYPEINFO();

 public:
  static DocumentFragment* Create(Document&);

  void ParseHTML(const String&,
                 Element* context_element,
                 ParserContentPolicy = kAllowScriptingContent);
  bool ParseXML(const String&,
                Element* context_element,
                ParserContentPolicy = kAllowScriptingContent);

  bool CanContainRangeEndPoint() const final { return true; }
  virtual bool IsTemplateContent() const { return false; }

 protected:
  DocumentFragment(Document*, ConstructionType = kCreateContainer);
  String nodeName() const final;

 private:
  NodeType getNodeType() const final;
  Node* cloneNode(bool deep, ExceptionState&) override;
  bool ChildTypeAllowed(NodeType) const override;

  bool IsDocumentFragment() const =
      delete;  // This will catch anyone doing an unnecessary check.
};

DEFINE_NODE_TYPE_CASTS(DocumentFragment, IsDocumentFragment());

}  // namespace blink

#endif  // DocumentFragment_h
