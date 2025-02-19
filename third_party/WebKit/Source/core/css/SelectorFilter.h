/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 2004-2005 Allan Sandfeld Jensen (kde@carewolf.com)
 * Copyright (C) 2006, 2007 Nicholas Shanks (webkit@nickshanks.com)
 * Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Apple Inc. All rights
 * reserved.
 * Copyright (C) 2007 Alexey Proskuryakov <ap@webkit.org>
 * Copyright (C) 2007, 2008 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2008, 2009 Torch Mobile Inc. All rights reserved.
 * (http://www.torchmobile.com/)
 * Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 * Copyright (C) Research In Motion Limited 2011. All rights reserved.
 * Copyright (C) 2012 Google Inc. All rights reserved.
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

#ifndef SelectorFilter_h
#define SelectorFilter_h

#include <memory>
#include "core/dom/Element.h"
#include "platform/wtf/BloomFilter.h"
#include "platform/wtf/Vector.h"

namespace blink {

class CSSSelector;

class CORE_EXPORT SelectorFilter {
  WTF_MAKE_NONCOPYABLE(SelectorFilter);
  DISALLOW_NEW();

 public:
  class ParentStackFrame {
    DISALLOW_NEW_EXCEPT_PLACEMENT_NEW();

   public:
    ParentStackFrame() : element(nullptr) {}
    explicit ParentStackFrame(Element& element) : element(&element) {}

    DECLARE_TRACE();

    Member<Element> element;
    Vector<unsigned, 4> identifier_hashes;
  };

  SelectorFilter() {}

  void PushParent(Element& parent);
  void PopParent(Element& parent);

  bool ParentStackIsConsistent(const ContainerNode* parent_node) const {
    return !parent_stack_.IsEmpty() &&
           parent_stack_.back().element == parent_node;
  }

  template <unsigned maximumIdentifierCount>
  inline bool FastRejectSelector(const unsigned* identifier_hashes) const;
  static void CollectIdentifierHashes(const CSSSelector&,
                                      unsigned* identifier_hashes,
                                      unsigned maximum_identifier_count);

  DECLARE_TRACE();

 private:
  void PushParentStackFrame(Element& parent);
  void PopParentStackFrame();

  HeapVector<ParentStackFrame> parent_stack_;

  // With 100 unique strings in the filter, 2^12 slot table has false positive
  // rate of ~0.2%.
  using IdentifierFilter = BloomFilter<12>;
  std::unique_ptr<IdentifierFilter> ancestor_identifier_filter_;
};

template <unsigned maximumIdentifierCount>
inline bool SelectorFilter::FastRejectSelector(
    const unsigned* identifier_hashes) const {
  DCHECK(ancestor_identifier_filter_);
  for (unsigned n = 0; n < maximumIdentifierCount && identifier_hashes[n];
       ++n) {
    if (!ancestor_identifier_filter_->MayContain(identifier_hashes[n]))
      return true;
  }
  return false;
}

}  // namespace blink

WTF_ALLOW_INIT_WITH_MEM_FUNCTIONS(blink::SelectorFilter::ParentStackFrame);

#endif
