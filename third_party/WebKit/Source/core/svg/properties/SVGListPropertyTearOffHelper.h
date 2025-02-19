/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SVGListPropertyTearOffHelper_h
#define SVGListPropertyTearOffHelper_h

#include "core/svg/properties/SVGPropertyTearOff.h"
#include "platform/wtf/TypeTraits.h"

namespace blink {

template <typename ItemProperty>
class ListItemPropertyTraits {
  STATIC_ONLY(ListItemPropertyTraits);

 public:
  typedef ItemProperty ItemPropertyType;
  typedef typename ItemPropertyType::TearOffType ItemTearOffType;

  static ItemPropertyType* GetValueForInsertionFromTearOff(
      ItemTearOffType* new_item,
      SVGElement* context_element,
      const QualifiedName& attribute_name) {
    // |newItem| is immutable, OR
    // |newItem| belongs to a SVGElement, but it does not belong to an animated
    // list, e.g. "textElement.x.baseVal.appendItem(rectElement.width.baseVal)"
    // Spec: If newItem is already in a list, then a new object is created with
    // the same values as newItem and this item is inserted into the list.
    // Otherwise, newItem itself is inserted into the list.
    if (new_item->IsImmutable() || new_item->Target()->OwnerList() ||
        new_item->contextElement()) {
      // We have to copy the incoming |newItem|,
      // Otherwise we'll end up having two tearoffs that operate on the same
      // SVGProperty. Consider the example below: SVGRectElements
      // SVGAnimatedLength 'width' property baseVal points to the same tear off
      // object that's inserted into SVGTextElements SVGAnimatedLengthList 'x'.
      // textElement.x.baseVal.getItem(0).value += 150 would mutate the
      // rectElement width _and_ the textElement x list. That's obviously wrong,
      // take care of that.
      return new_item->Target()->Clone();
    }

    new_item->AttachToSVGElementAttribute(context_element, attribute_name);
    return new_item->Target();
  }

  static ItemTearOffType* CreateTearOff(
      ItemPropertyType* value,
      SVGElement* context_element,
      PropertyIsAnimValType property_is_anim_val,
      const QualifiedName& attribute_name) {
    return ItemTearOffType::Create(value, context_element, property_is_anim_val,
                                   attribute_name);
  }
};

template <typename Derived, typename ListProperty>
class SVGListPropertyTearOffHelper : public SVGPropertyTearOff<ListProperty> {
 public:
  typedef ListProperty ListPropertyType;
  typedef typename ListPropertyType::ItemPropertyType ItemPropertyType;
  typedef typename ItemPropertyType::TearOffType ItemTearOffType;
  typedef ListItemPropertyTraits<ItemPropertyType> ItemTraits;

  // SVG*List DOM interface:

  // WebIDL requires "unsigned long" type instead of size_t.
  unsigned long length() { return ToDerived()->Target()->length(); }

  void clear(ExceptionState& exception_state) {
    if (ToDerived()->IsImmutable()) {
      SVGPropertyTearOffBase::ThrowReadOnly(exception_state);
      return;
    }
    ToDerived()->Target()->Clear();
  }

  ItemTearOffType* initialize(ItemTearOffType* item,
                              ExceptionState& exception_state) {
    if (ToDerived()->IsImmutable()) {
      SVGPropertyTearOffBase::ThrowReadOnly(exception_state);
      return nullptr;
    }
    DCHECK(item);
    ItemPropertyType* value = ToDerived()->Target()->Initialize(
        GetValueForInsertionFromTearOff(item));
    ToDerived()->CommitChange();
    return CreateItemTearOff(value);
  }

  ItemTearOffType* getItem(unsigned long index,
                           ExceptionState& exception_state) {
    ItemPropertyType* value =
        ToDerived()->Target()->GetItem(index, exception_state);
    return CreateItemTearOff(value);
  }

  ItemTearOffType* insertItemBefore(ItemTearOffType* item,
                                    unsigned long index,
                                    ExceptionState& exception_state) {
    if (ToDerived()->IsImmutable()) {
      SVGPropertyTearOffBase::ThrowReadOnly(exception_state);
      return nullptr;
    }
    DCHECK(item);
    ItemPropertyType* value = ToDerived()->Target()->InsertItemBefore(
        GetValueForInsertionFromTearOff(item), index);
    ToDerived()->CommitChange();
    return CreateItemTearOff(value);
  }

  ItemTearOffType* replaceItem(ItemTearOffType* item,
                               unsigned long index,
                               ExceptionState& exception_state) {
    if (ToDerived()->IsImmutable()) {
      SVGPropertyTearOffBase::ThrowReadOnly(exception_state);
      return nullptr;
    }
    DCHECK(item);
    ItemPropertyType* value = ToDerived()->Target()->ReplaceItem(
        GetValueForInsertionFromTearOff(item), index, exception_state);
    ToDerived()->CommitChange();
    return CreateItemTearOff(value);
  }

  bool AnonymousIndexedSetter(unsigned index,
                              ItemTearOffType* item,
                              ExceptionState& exception_state) {
    replaceItem(item, index, exception_state);
    return true;
  }

  ItemTearOffType* removeItem(unsigned long index,
                              ExceptionState& exception_state) {
    if (ToDerived()->IsImmutable()) {
      SVGPropertyTearOffBase::ThrowReadOnly(exception_state);
      return nullptr;
    }
    ItemPropertyType* value =
        ToDerived()->Target()->RemoveItem(index, exception_state);
    ToDerived()->CommitChange();
    return CreateItemTearOff(value);
  }

  ItemTearOffType* appendItem(ItemTearOffType* item,
                              ExceptionState& exception_state) {
    if (ToDerived()->IsImmutable()) {
      SVGPropertyTearOffBase::ThrowReadOnly(exception_state);
      return nullptr;
    }
    DCHECK(item);
    ItemPropertyType* value = ToDerived()->Target()->AppendItem(
        GetValueForInsertionFromTearOff(item));
    ToDerived()->CommitChange();
    return CreateItemTearOff(value);
  }

  DEFINE_INLINE_VIRTUAL_TRACE_WRAPPERS() {
    SVGPropertyTearOff<ListProperty>::TraceWrappers(visitor);
  }

 protected:
  SVGListPropertyTearOffHelper(
      ListPropertyType* target,
      SVGElement* context_element,
      PropertyIsAnimValType property_is_anim_val,
      const QualifiedName& attribute_name = QualifiedName::Null())
      : SVGPropertyTearOff<ListPropertyType>(target,
                                             context_element,
                                             property_is_anim_val,
                                             attribute_name) {}

  ItemPropertyType* GetValueForInsertionFromTearOff(ItemTearOffType* new_item) {
    return ItemTraits::GetValueForInsertionFromTearOff(
        new_item, ToDerived()->contextElement(), ToDerived()->AttributeName());
  }

  ItemTearOffType* CreateItemTearOff(ItemPropertyType* value) {
    if (!value)
      return nullptr;

    if (value->OwnerList() == ToDerived()->Target())
      return ItemTraits::CreateTearOff(value, ToDerived()->contextElement(),
                                       ToDerived()->PropertyIsAnimVal(),
                                       ToDerived()->AttributeName());

    return ItemTraits::CreateTearOff(value, 0, kPropertyIsNotAnimVal,
                                     QualifiedName::Null());
  }

 private:
  Derived* ToDerived() { return static_cast<Derived*>(this); }
};

}  // namespace blink

#endif  // SVGListPropertyTearOffHelper_h
