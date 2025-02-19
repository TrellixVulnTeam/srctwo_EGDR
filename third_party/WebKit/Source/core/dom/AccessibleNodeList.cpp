// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/dom/AccessibleNodeList.h"

#include "core/dom/AccessibleNode.h"
#include "platform/RuntimeEnabledFeatures.h"

namespace blink {

// The spec doesn't give a limit, but there's no reason to allow relations
// between an arbitrarily large number of other accessible nodes.
static const unsigned kMaxItems = 65536;

// static
AccessibleNodeList* AccessibleNodeList::Create(
    const HeapVector<Member<AccessibleNode>>& nodes) {
  AccessibleNodeList* result = new AccessibleNodeList();
  result->nodes_ = nodes;
  return result;
}

AccessibleNodeList::AccessibleNodeList() {
  DCHECK(RuntimeEnabledFeatures::AccessibilityObjectModelEnabled());
}

AccessibleNodeList::~AccessibleNodeList() {}

void AccessibleNodeList::AddOwner(AOMRelationListProperty property,
                                  AccessibleNode* node) {
  owners_.push_back(std::make_pair(property, node));
}

void AccessibleNodeList::RemoveOwner(AOMRelationListProperty property,
                                     AccessibleNode* node) {
  for (size_t i = 0; i < owners_.size(); ++i) {
    auto& item = owners_[i];
    if (item.first == property && item.second == node) {
      owners_.erase(i);
      return;
    }
  }
}

AccessibleNode* AccessibleNodeList::item(unsigned offset) const {
  if (offset < nodes_.size())
    return nodes_[offset];
  return nullptr;
}

void AccessibleNodeList::add(AccessibleNode* node, AccessibleNode* before) {
  if (nodes_.size() == kMaxItems)
    return;

  unsigned index = nodes_.size();
  if (before) {
    for (index = 0; index < nodes_.size(); ++index) {
      if (nodes_[index] == before)
        break;
    }
    if (index == nodes_.size())
      return;
  }

  nodes_.insert(index, node);
}

void AccessibleNodeList::remove(int index) {
  if (index >= 0 && index < static_cast<int>(nodes_.size()))
    nodes_.erase(index);
}

bool AccessibleNodeList::AnonymousIndexedSetter(unsigned index,
                                                AccessibleNode* node,
                                                ExceptionState& state) {
  if (!node) {
    remove(index);
    return true;
  }
  if (index >= kMaxItems)
    return false;
  if (index >= nodes_.size()) {
    unsigned old_size = nodes_.size();
    nodes_.resize(index + 1);
    for (unsigned i = old_size; i < nodes_.size(); ++i)
      nodes_[i] = nullptr;
  }
  nodes_[index] = node;
  return true;
}

unsigned AccessibleNodeList::length() const {
  return nodes_.size();
}

void AccessibleNodeList::setLength(unsigned new_length) {
  if (new_length >= kMaxItems)
    return;
  nodes_.resize(new_length);
}

void AccessibleNodeList::NotifyChanged() {
  for (auto& owner : owners_)
    owner.second->OnRelationListChanged(owner.first);
}

DEFINE_TRACE(AccessibleNodeList) {
  visitor->Trace(nodes_);
  visitor->Trace(owners_);
}

}  // namespace blink
