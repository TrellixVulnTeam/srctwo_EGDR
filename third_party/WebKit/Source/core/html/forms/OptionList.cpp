// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/html/forms/OptionList.h"

#include "core/dom/ElementTraversal.h"
#include "core/html/HTMLOptionElement.h"
#include "core/html/HTMLSelectElement.h"

namespace blink {

void OptionListIterator::Advance(HTMLOptionElement* previous) {
  // This function returns only
  // - An OPTION child of m_select, or
  // - An OPTION child of an OPTGROUP child of m_select.

  Element* current;
  if (previous) {
    DCHECK_EQ(previous->OwnerSelectElement(), select_);
    current = ElementTraversal::NextSkippingChildren(*previous, select_);
  } else {
    current = ElementTraversal::FirstChild(*select_);
  }
  while (current) {
    if (isHTMLOptionElement(current)) {
      current_ = toHTMLOptionElement(current);
      return;
    }
    if (isHTMLOptGroupElement(current) &&
        current->parentNode() == select_.Get()) {
      if ((current_ = Traversal<HTMLOptionElement>::FirstChild(*current)))
        return;
    }
    current = ElementTraversal::NextSkippingChildren(*current, select_);
  }
  current_ = nullptr;
}

}  // namespace blink
