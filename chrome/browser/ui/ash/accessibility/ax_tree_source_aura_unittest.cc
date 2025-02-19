// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>

#include <vector>

#include "ash/test/ash_test_base.h"
#include "base/macros.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/browser/ui/aura/accessibility/ax_tree_source_aura.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/accessibility/ax_action_data.h"
#include "ui/accessibility/ax_enums.h"
#include "ui/accessibility/ax_node.h"
#include "ui/accessibility/ax_serializable_tree.h"
#include "ui/accessibility/ax_tree_serializer.h"
#include "ui/accessibility/ax_tree_update.h"
#include "ui/aura/window.h"
#include "ui/views/accessibility/ax_aura_obj_cache.h"
#include "ui/views/accessibility/ax_aura_obj_wrapper.h"
#include "ui/views/controls/textfield/textfield.h"
#include "ui/views/test/views_test_base.h"
#include "ui/views/widget/widget.h"

using views::AXAuraObjCache;
using views::AXAuraObjWrapper;
using views::Textfield;
using views::View;
using views::Widget;

using AuraAXTreeSerializer = ui::AXTreeSerializer<views::AXAuraObjWrapper*,
                                                  ui::AXNodeData,
                                                  ui::AXTreeData>;

// Helper to count the number of nodes in a tree.
size_t GetSize(AXAuraObjWrapper* tree) {
  size_t count = 1;

  std::vector<AXAuraObjWrapper*> out_children;
  tree->GetChildren(&out_children);

  for (size_t i = 0; i < out_children.size(); ++i)
    count += GetSize(out_children[i]);

  return count;
}

class AXTreeSourceAuraTest : public ash::AshTestBase {
 public:
  AXTreeSourceAuraTest() {}
  ~AXTreeSourceAuraTest() override {}

  void SetUp() override {
    AshTestBase::SetUp();

    widget_ = new Widget();
    Widget::InitParams init_params(Widget::InitParams::TYPE_WINDOW_FRAMELESS);
    init_params.context = CurrentContext();
    widget_->Init(init_params);

    content_ = new View();
    widget_->SetContentsView(content_);

    textfield_ = new Textfield();
    textfield_->SetText(base::ASCIIToUTF16("Value"));
    content_->AddChildView(textfield_);
    widget_->Show();
  }

 protected:
  Widget* widget_;
  View* content_;
  Textfield* textfield_;

 private:
  DISALLOW_COPY_AND_ASSIGN(AXTreeSourceAuraTest);
};

TEST_F(AXTreeSourceAuraTest, Accessors) {
  AXTreeSourceAura ax_tree;
  ASSERT_TRUE(ax_tree.GetRoot());

  // ID's should start at 1 and there should be a root.
  ASSERT_EQ(1, ax_tree.GetRoot()->GetID());

  // Grab the content view directly from cache to avoid walking down the tree.
  AXAuraObjWrapper* content =
      AXAuraObjCache::GetInstance()->GetOrCreate(content_);
  std::vector<AXAuraObjWrapper*> content_children;
  ax_tree.GetChildren(content, &content_children);
  ASSERT_EQ(1U, content_children.size());

  // Walk down to the text field and assert it is what we expect.
  AXAuraObjWrapper* textfield = content_children[0];
  AXAuraObjWrapper* cached_textfield =
      AXAuraObjCache::GetInstance()->GetOrCreate(textfield_);
  ASSERT_EQ(cached_textfield, textfield);
  std::vector<AXAuraObjWrapper*> textfield_children;
  ax_tree.GetChildren(textfield, &textfield_children);
  ASSERT_EQ(1U, textfield_children.size());

  ASSERT_EQ(content, textfield->GetParent());

  // Try walking up the tree to the root.
  AXAuraObjWrapper* test_root = NULL;
  for (AXAuraObjWrapper* root_finder = ax_tree.GetParent(content); root_finder;
       root_finder = ax_tree.GetParent(root_finder))
    test_root = root_finder;
  ASSERT_EQ(ax_tree.GetRoot(), test_root);
}

TEST_F(AXTreeSourceAuraTest, DoDefault) {
  AXTreeSourceAura ax_tree;

  // Grab a wrapper to |DoDefault| (click).
  AXAuraObjWrapper* textfield_wrapper =
      AXAuraObjCache::GetInstance()->GetOrCreate(textfield_);

  // Click and verify focus.
  ASSERT_FALSE(textfield_->HasFocus());
  ui::AXActionData action_data;
  action_data.action = ui::AX_ACTION_DO_DEFAULT;
  action_data.target_node_id = textfield_wrapper->GetID();
  textfield_wrapper->HandleAccessibleAction(action_data);
  ASSERT_TRUE(textfield_->HasFocus());
}

TEST_F(AXTreeSourceAuraTest, Focus) {
  AXTreeSourceAura ax_tree;

  // Grab a wrapper to focus.
  AXAuraObjWrapper* textfield_wrapper =
      AXAuraObjCache::GetInstance()->GetOrCreate(textfield_);

  // Focus and verify.
  ASSERT_FALSE(textfield_->HasFocus());
  ui::AXActionData action_data;
  action_data.action = ui::AX_ACTION_FOCUS;
  action_data.target_node_id = textfield_wrapper->GetID();
  textfield_wrapper->HandleAccessibleAction(action_data);
  ASSERT_TRUE(textfield_->HasFocus());
}

TEST_F(AXTreeSourceAuraTest, Serialize) {
  AXTreeSourceAura ax_tree;
  AuraAXTreeSerializer ax_serializer(&ax_tree);
  ui::AXTreeUpdate out_update;

  // This is the initial serialization.
  ax_serializer.SerializeChanges(ax_tree.GetRoot(), &out_update);

  // The update should just be the desktop node and the fake alert window we use
  // to handle posting text alerts.
  ASSERT_EQ(2U, out_update.nodes.size());

  // Try removing some child views and re-adding which should fire some events.
  content_->RemoveAllChildViews(false /* delete_children */);
  content_->AddChildView(textfield_);

  // Grab the textfield since serialization only walks up the tree (not down
  // from root).
  AXAuraObjWrapper* textfield_wrapper =
      AXAuraObjCache::GetInstance()->GetOrCreate(textfield_);

  // Now, re-serialize.
  ui::AXTreeUpdate out_update2;
  ax_serializer.SerializeChanges(textfield_wrapper, &out_update2);

  size_t node_count = out_update2.nodes.size();

  // We should have far more updates this time around.
  ASSERT_GE(node_count, 10U);

  int text_field_update_index = -1;
  for (size_t i = 0; i < node_count; ++i) {
    if (textfield_wrapper->GetID() == out_update2.nodes[i].id)
      text_field_update_index = i;
  }

  ASSERT_NE(-1, text_field_update_index);
  ASSERT_EQ(ui::AX_ROLE_TEXT_FIELD,
            out_update2.nodes[text_field_update_index].role);
}
