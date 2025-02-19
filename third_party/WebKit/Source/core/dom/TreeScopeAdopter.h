/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Apple Inc. All
 * rights reserved.
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2009 Torch Mobile Inc. All rights reserved.
 * (http://www.torchmobile.com/)
 * Copyright (C) 2011 Google Inc. All rights reserved.
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
#ifndef TreeScopeAdopter_h
#define TreeScopeAdopter_h

#include "core/dom/Node.h"

namespace blink {

class TreeScope;

class TreeScopeAdopter {
  STACK_ALLOCATED();

 public:
  TreeScopeAdopter(Node& to_adopt, TreeScope& new_scope);

  void Execute() const;
  bool NeedsScopeChange() const { return old_scope_ != new_scope_; }

#if DCHECK_IS_ON()
  static void EnsureDidMoveToNewDocumentWasCalled(Document&);
#else
  static void EnsureDidMoveToNewDocumentWasCalled(Document&) {}
#endif

 private:
  void UpdateTreeScope(Node&) const;
  void MoveTreeToNewScope(Node&) const;
  void MoveTreeToNewDocument(Node&,
                             Document& old_document,
                             Document& new_document) const;
  void MoveNodeToNewDocument(Node&,
                             Document& old_document,
                             Document& new_document) const;
  TreeScope& OldScope() const { return *old_scope_; }
  TreeScope& NewScope() const { return *new_scope_; }

  Member<Node> to_adopt_;
  Member<TreeScope> new_scope_;
  Member<TreeScope> old_scope_;
};

inline TreeScopeAdopter::TreeScopeAdopter(Node& to_adopt, TreeScope& new_scope)
    : to_adopt_(to_adopt),
      new_scope_(new_scope),
      old_scope_(to_adopt.GetTreeScope()) {}

}  // namespace blink

#endif
