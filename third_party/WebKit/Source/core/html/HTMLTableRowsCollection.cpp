/*
 * Copyright (C) 2008, 2011, 2012, 2014 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "core/html/HTMLTableRowsCollection.h"

#include "core/HTMLNames.h"
#include "core/dom/ElementTraversal.h"
#include "core/html/HTMLTableElement.h"
#include "core/html/HTMLTableRowElement.h"

namespace blink {

using namespace HTMLNames;

static inline bool IsInSection(HTMLTableRowElement& row,
                               const HTMLQualifiedName& section_tag) {
  // Because we know that the parent is a table or a section, it's safe to cast
  // it to an HTMLElement giving us access to the faster hasTagName overload
  // from that class.
  return ToHTMLElement(row.parentNode())->HasTagName(section_tag);
}

HTMLTableRowElement* HTMLTableRowsCollection::RowAfter(
    HTMLTableElement& table,
    HTMLTableRowElement* previous) {
  // Start by looking for the next row in this section.
  // Continue only if there is none.
  if (previous && previous->parentNode() != table) {
    if (HTMLTableRowElement* row =
            Traversal<HTMLTableRowElement>::NextSibling(*previous))
      return row;
  }

  // If still looking at head sections, find the first row in the next head
  // section.
  HTMLElement* child = 0;
  if (!previous)
    child = Traversal<HTMLElement>::FirstChild(table);
  else if (IsInSection(*previous, theadTag))
    child = Traversal<HTMLElement>::NextSibling(*previous->parentNode());
  for (; child; child = Traversal<HTMLElement>::NextSibling(*child)) {
    if (child->HasTagName(theadTag)) {
      if (HTMLTableRowElement* row =
              Traversal<HTMLTableRowElement>::FirstChild(*child))
        return row;
    }
  }

  // If still looking at top level and bodies, find the next row in top level or
  // the first in the next body section.
  if (!previous || IsInSection(*previous, theadTag))
    child = Traversal<HTMLElement>::FirstChild(table);
  else if (previous->parentNode() == table)
    child = Traversal<HTMLElement>::NextSibling(*previous);
  else if (IsInSection(*previous, tbodyTag))
    child = Traversal<HTMLElement>::NextSibling(*previous->parentNode());
  for (; child; child = Traversal<HTMLElement>::NextSibling(*child)) {
    if (isHTMLTableRowElement(child))
      return toHTMLTableRowElement(child);
    if (child->HasTagName(tbodyTag)) {
      if (HTMLTableRowElement* row =
              Traversal<HTMLTableRowElement>::FirstChild(*child))
        return row;
    }
  }

  // Find the first row in the next foot section.
  if (!previous || !IsInSection(*previous, tfootTag))
    child = Traversal<HTMLElement>::FirstChild(table);
  else
    child = Traversal<HTMLElement>::NextSibling(*previous->parentNode());
  for (; child; child = Traversal<HTMLElement>::NextSibling(*child)) {
    if (child->HasTagName(tfootTag)) {
      if (HTMLTableRowElement* row =
              Traversal<HTMLTableRowElement>::FirstChild(*child))
        return row;
    }
  }

  return nullptr;
}

HTMLTableRowElement* HTMLTableRowsCollection::LastRow(HTMLTableElement& table) {
  for (HTMLElement* tfoot =
           Traversal<HTMLElement>::LastChild(table, HasHTMLTagName(tfootTag));
       tfoot; tfoot = Traversal<HTMLElement>::PreviousSibling(
                  *tfoot, HasHTMLTagName(tfootTag))) {
    if (HTMLTableRowElement* last_row =
            Traversal<HTMLTableRowElement>::LastChild(*tfoot))
      return last_row;
  }

  for (HTMLElement* child = Traversal<HTMLElement>::LastChild(table); child;
       child = Traversal<HTMLElement>::PreviousSibling(*child)) {
    if (isHTMLTableRowElement(child))
      return toHTMLTableRowElement(child);
    if (child->HasTagName(tbodyTag)) {
      if (HTMLTableRowElement* last_row =
              Traversal<HTMLTableRowElement>::LastChild(*child))
        return last_row;
    }
  }

  for (HTMLElement* thead =
           Traversal<HTMLElement>::LastChild(table, HasHTMLTagName(theadTag));
       thead; thead = Traversal<HTMLElement>::PreviousSibling(
                  *thead, HasHTMLTagName(theadTag))) {
    if (HTMLTableRowElement* last_row =
            Traversal<HTMLTableRowElement>::LastChild(*thead))
      return last_row;
  }

  return nullptr;
}

// Must call get() on the table in case that argument is compiled before
// dereferencing the table to get at the collection cache. Order of argument
// evaluation is undefined and can differ between compilers.
HTMLTableRowsCollection::HTMLTableRowsCollection(ContainerNode& table)
    : HTMLCollection(table, kTableRows, kOverridesItemAfter) {
  DCHECK(isHTMLTableElement(table));
}

HTMLTableRowsCollection* HTMLTableRowsCollection::Create(ContainerNode& table,
                                                         CollectionType type) {
  DCHECK_EQ(type, kTableRows);
  return new HTMLTableRowsCollection(table);
}

Element* HTMLTableRowsCollection::VirtualItemAfter(Element* previous) const {
  return RowAfter(toHTMLTableElement(ownerNode()),
                  toHTMLTableRowElement(previous));
}

}  // namespace blink
