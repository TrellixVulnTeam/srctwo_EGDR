// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/accessibility/ax_relative_bounds.h"

#include "base/strings/string_number_conversions.h"
#include "ui/gfx/transform.h"

using base::IntToString;

namespace ui {

AXRelativeBounds::AXRelativeBounds()
    : offset_container_id(-1) {
}

AXRelativeBounds::~AXRelativeBounds() {
}

AXRelativeBounds::AXRelativeBounds(const AXRelativeBounds& other) {
  offset_container_id = other.offset_container_id;
  bounds = other.bounds;
  if (other.transform)
    transform.reset(new gfx::Transform(*other.transform));
}

AXRelativeBounds& AXRelativeBounds::operator=(AXRelativeBounds other) {
  offset_container_id = other.offset_container_id;
  bounds = other.bounds;
  if (other.transform)
    transform.reset(new gfx::Transform(*other.transform));
  return *this;
}

bool AXRelativeBounds::operator==(const AXRelativeBounds& other) {
  if (offset_container_id != other.offset_container_id)
    return false;
  if (bounds != other.bounds)
    return false;
  if (!transform && !other.transform)
    return true;
  if ((transform && !other.transform) || (!transform && other.transform))
    return false;
  return *transform == *other.transform;
}

bool AXRelativeBounds::operator!=(const AXRelativeBounds& other) {
  return !operator==(other);
}

std::string AXRelativeBounds::ToString() const {
  std::string result;

  if (offset_container_id != -1)
    result += "offset_container_id=" + IntToString(offset_container_id) + " ";

  result += "(" + IntToString(bounds.x()) + ", " +
      IntToString(bounds.y()) + ")-(" +
      IntToString(bounds.width()) + ", " +
      IntToString(bounds.height()) + ")";

  if (transform && !transform->IsIdentity())
    result += " transform=" + transform->ToString();

  return result;
}

}  // namespace ui
