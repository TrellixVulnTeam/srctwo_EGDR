// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_ACCESSIBILITY_AX_TREE_DATA_H_
#define UI_ACCESSIBILITY_AX_TREE_DATA_H_

#include <stdint.h>

#include <map>
#include <string>
#include <vector>

#include "base/strings/string16.h"
#include "base/strings/string_split.h"
#include "ui/accessibility/ax_enums.h"
#include "ui/accessibility/ax_export.h"
#include "ui/gfx/geometry/rect.h"

namespace ui {

// The data associated with an accessibility tree that's global to the
// tree and not associated with any particular node in the tree.
struct AX_EXPORT AXTreeData {
  AXTreeData();
  AXTreeData(const AXTreeData& other);
  virtual ~AXTreeData();

  // Return a string representation of this data, for debugging.
  virtual std::string ToString() const;

  // This is a simple serializable struct. All member variables should be
  // public and copyable.

  // The globally unique ID of this accessibility tree.
  int32_t tree_id;

  // The ID of the accessibility tree that this tree is contained in, if any.
  int32_t parent_tree_id;

  // The ID of the accessibility tree that has focus. This is typically set
  // on the root frame in a frame tree.
  int32_t focused_tree_id;

  // Attributes specific to trees that are web frames.
  std::string doctype;
  bool loaded;
  float loading_progress;
  std::string mimetype;
  std::string title;
  std::string url;

  // The node with keyboard focus within this tree, if any, or -1 if no node
  // in this tree has focus.
  int32_t focus_id;

  // The current text selection within this tree, if any, expressed as the
  // node ID and character offset of the anchor (selection start) and focus
  // (selection end). If the offset could correspond to a position on two
  // different lines, sel_upstream_affinity means the cursor is on the first
  // line, otherwise it's on the second line.
  int32_t sel_anchor_object_id;
  int32_t sel_anchor_offset;
  ui::AXTextAffinity sel_anchor_affinity;
  int32_t sel_focus_object_id;
  int32_t sel_focus_offset;
  ui::AXTextAffinity sel_focus_affinity;
};

AX_EXPORT bool operator==(const AXTreeData& lhs, const AXTreeData& rhs);
AX_EXPORT bool operator!=(const AXTreeData& lhs, const AXTreeData& rhs);

}  // namespace ui

#endif  // UI_ACCESSIBILITY_AX_TREE_DATA_H_
