// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "modules/accessibility/AXRadioInput.h"

#include "core/InputTypeNames.h"
#include "core/dom/AccessibleNode.h"
#include "core/html/HTMLInputElement.h"
#include "core/html/forms/RadioInputType.h"
#include "core/layout/LayoutObject.h"
#include "modules/accessibility/AXObjectCacheImpl.h"

namespace blink {

using namespace HTMLNames;

AXRadioInput::AXRadioInput(LayoutObject* layout_object,
                           AXObjectCacheImpl& ax_object_cache)
    : AXLayoutObject(layout_object, ax_object_cache) {
  // Updates posInSet and setSize for the current object and the next objects.
  if (!CalculatePosInSet())
    return;
  // When a new object is inserted, it needs to update setSize for the previous
  // objects.
  RequestUpdateToNextNode(false);
}

AXRadioInput* AXRadioInput::Create(LayoutObject* layout_object,
                                   AXObjectCacheImpl& ax_object_cache) {
  return new AXRadioInput(layout_object, ax_object_cache);
}

void AXRadioInput::UpdatePosAndSetSize(int position) {
  if (position)
    pos_in_set_ = position;
  set_size_ = SizeOfRadioGroup();
}

void AXRadioInput::RequestUpdateToNextNode(bool forward) {
  HTMLInputElement* next_element =
      RadioInputType::NextRadioButtonInGroup(GetInputElement(), forward);
  AXObject* next_axobject = AxObjectCache().Get(next_element);
  if (!next_axobject || !next_axobject->IsAXRadioInput())
    return;

  int position = 0;
  if (forward)
    position = PosInSet() + 1;
  // If it is backward, it keeps position as positions are already assigned for
  // previous objects.  updatePosAndSetSize() is called with '0' and it doesn't
  // modify m_posInSet and updates m_setSize as size is increased.

  ToAXRadioInput(next_axobject)->UpdatePosAndSetSize(position);
  AxObjectCache().PostNotification(next_axobject,
                                   AXObjectCacheImpl::kAXAriaAttributeChanged);
  ToAXRadioInput(next_axobject)->RequestUpdateToNextNode(forward);
}

HTMLInputElement* AXRadioInput::FindFirstRadioButtonInGroup(
    HTMLInputElement* current) const {
  while (HTMLInputElement* prev_element =
             RadioInputType::NextRadioButtonInGroup(current, false))
    current = prev_element;
  return current;
}

int AXRadioInput::PosInSet() const {
  uint32_t pos_in_set;
  if (HasAOMPropertyOrARIAAttribute(AOMUIntProperty::kPosInSet, pos_in_set))
    return pos_in_set;
  return pos_in_set_;
}

int AXRadioInput::SetSize() const {
  int32_t set_size;
  if (HasAOMPropertyOrARIAAttribute(AOMIntProperty::kSetSize, set_size))
    return set_size;
  return set_size_;
}

bool AXRadioInput::CalculatePosInSet() {
  // Calculate 'posInSet' attribute when AXRadioInputs need to be updated
  // as a new AXRadioInput Object is added or one of objects from RadioGroup is
  // removed.
  bool need_to_update_prev = false;
  int position = 1;
  HTMLInputElement* prev_element =
      RadioInputType::NextRadioButtonInGroup(GetInputElement(), false);
  if (prev_element) {
    AXObject* object = AxObjectCache().Get(prev_element);
    // If the previous element doesn't have AXObject yet, caculate position
    // from the first element.  Otherwise, get position from the previous
    // AXObject.
    if (!object || !object->IsAXRadioInput()) {
      position = CountFromFirstElement();
    } else {
      position = object->PosInSet() + 1;
      // It returns true if previous objects need to be updated.
      // When AX tree exists already and a new node is inserted,
      // as updating is started from the inserted node,
      // we need to update setSize for previous nodes.
      if (SetSize() != object->SetSize())
        need_to_update_prev = true;
    }
  }
  UpdatePosAndSetSize(position);

  // If it is not the last element, request update to the next node.
  if (position != SetSize())
    RequestUpdateToNextNode(true);
  return need_to_update_prev;
}

int AXRadioInput::CountFromFirstElement() const {
  int count = 1;
  HTMLInputElement* current = GetInputElement();
  while (HTMLInputElement* prev_element =
             RadioInputType::NextRadioButtonInGroup(current, false)) {
    current = prev_element;
    count++;
  }
  return count;
}

HTMLInputElement* AXRadioInput::GetInputElement() const {
  return toHTMLInputElement(layout_object_->GetNode());
}

int AXRadioInput::SizeOfRadioGroup() const {
  int size = GetInputElement()->SizeOfRadioGroup();
  // If it has no size in Group, it means that there is only itself.
  if (!size)
    return 1;
  return size;
}

}  // namespace blink
