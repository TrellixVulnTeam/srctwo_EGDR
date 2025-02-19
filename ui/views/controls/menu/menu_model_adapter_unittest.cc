// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/controls/menu/menu_model_adapter.h"

#include "base/macros.h"
#include "base/strings/utf_string_conversions.h"
#include "ui/base/models/menu_model.h"
#include "ui/base/models/menu_model_delegate.h"
#include "ui/views/controls/menu/menu_item_view.h"
#include "ui/views/controls/menu/menu_runner.h"
#include "ui/views/controls/menu/submenu_view.h"
#include "ui/views/test/views_test_base.h"

namespace {

// Base command id for test menu and its submenu.
const int kRootIdBase = 100;
const int kSubmenuIdBase = 200;

class MenuModelBase : public ui::MenuModel {
 public:
  explicit MenuModelBase(int command_id_base)
      : command_id_base_(command_id_base),
        last_activation_(-1) {
  }

  ~MenuModelBase() override {}

  // ui::MenuModel implementation:

  bool HasIcons() const override { return false; }

  int GetItemCount() const override { return static_cast<int>(items_.size()); }

  ItemType GetTypeAt(int index) const override { return items_[index].type; }

  ui::MenuSeparatorType GetSeparatorTypeAt(int index) const override {
    return ui::NORMAL_SEPARATOR;
  }

  int GetCommandIdAt(int index) const override {
    return index + command_id_base_;
  }

  base::string16 GetLabelAt(int index) const override {
    return items_[index].label;
  }

  bool IsItemDynamicAt(int index) const override { return false; }

  const gfx::FontList* GetLabelFontListAt(int index) const override {
    return NULL;
  }

  bool GetAcceleratorAt(int index,
                        ui::Accelerator* accelerator) const override {
    return false;
  }

  bool IsItemCheckedAt(int index) const override { return false; }

  int GetGroupIdAt(int index) const override { return 0; }

  bool GetIconAt(int index, gfx::Image* icon) override { return false; }

  ui::ButtonMenuItemModel* GetButtonMenuItemAt(int index) const override {
    return NULL;
  }

  bool IsEnabledAt(int index) const override { return true; }

  bool IsVisibleAt(int index) const override { return true; }

  MenuModel* GetSubmenuModelAt(int index) const override {
    return items_[index].submenu;
  }

  void HighlightChangedTo(int index) override {}

  void ActivatedAt(int index) override { set_last_activation(index); }

  void ActivatedAt(int index, int event_flags) override { ActivatedAt(index); }

  void MenuWillShow() override {}

  void MenuWillClose() override {}

  void SetMenuModelDelegate(ui::MenuModelDelegate* delegate) override {}

  ui::MenuModelDelegate* GetMenuModelDelegate() const override { return NULL; }

  // Item definition.
  struct Item {
    Item(ItemType item_type,
         const std::string& item_label,
         ui::MenuModel* item_submenu)
        : type(item_type),
          label(base::ASCIIToUTF16(item_label)),
          submenu(item_submenu) {
    }

    ItemType type;
    base::string16 label;
    ui::MenuModel* submenu;
  };

  const Item& GetItemDefinition(int index) {
    return items_[index];
  }

  // Access index argument to ActivatedAt().
  int last_activation() const { return last_activation_; }
  void set_last_activation(int last_activation) {
    last_activation_ = last_activation;
  }

 protected:
  std::vector<Item> items_;

 private:
  int command_id_base_;
  int last_activation_;

  DISALLOW_COPY_AND_ASSIGN(MenuModelBase);
};

class SubmenuModel : public MenuModelBase {
 public:
  SubmenuModel() : MenuModelBase(kSubmenuIdBase) {
    items_.push_back(Item(TYPE_COMMAND, "submenu item 0", NULL));
    items_.push_back(Item(TYPE_COMMAND, "submenu item 1", NULL));
  }

  ~SubmenuModel() override {}

 private:
  DISALLOW_COPY_AND_ASSIGN(SubmenuModel);
};

class RootModel : public MenuModelBase {
 public:
  RootModel() : MenuModelBase(kRootIdBase) {
    submenu_model_.reset(new SubmenuModel);

    items_.push_back(Item(TYPE_COMMAND, "command 0", NULL));
    items_.push_back(Item(TYPE_CHECK, "check 1", NULL));
    items_.push_back(Item(TYPE_SEPARATOR, "", NULL));
    items_.push_back(Item(TYPE_SUBMENU, "submenu 3", submenu_model_.get()));
    items_.push_back(Item(TYPE_RADIO, "radio 4", NULL));
  }

  ~RootModel() override {}

 private:
  std::unique_ptr<MenuModel> submenu_model_;

  DISALLOW_COPY_AND_ASSIGN(RootModel);
};

}  // namespace

namespace views {

typedef ViewsTestBase MenuModelAdapterTest;

TEST_F(MenuModelAdapterTest, BasicTest) {
  // Build model and adapter.
  RootModel model;
  views::MenuModelAdapter delegate(&model);

  // Create menu.  Build menu twice to check that rebuilding works properly.
  MenuItemView* menu = new views::MenuItemView(&delegate);
  // MenuRunner takes ownership of menu.
  std::unique_ptr<MenuRunner> menu_runner(new MenuRunner(menu, 0));
  delegate.BuildMenu(menu);
  delegate.BuildMenu(menu);
  EXPECT_TRUE(menu->HasSubmenu());

  // Check top level menu items.
  views::SubmenuView* item_container = menu->GetSubmenu();
  EXPECT_EQ(5, item_container->child_count());

  for (int i = 0; i < item_container->child_count(); ++i) {
    const MenuModelBase::Item& model_item = model.GetItemDefinition(i);

    const int id = i + kRootIdBase;
    MenuItemView* item = menu->GetMenuItemByID(id);
    if (!item) {
      EXPECT_EQ(ui::MenuModel::TYPE_SEPARATOR, model_item.type);
      continue;
    }

    // Check placement.
    EXPECT_EQ(i, menu->GetSubmenu()->GetIndexOf(item));

    // Check type.
    switch (model_item.type) {
      case ui::MenuModel::TYPE_COMMAND:
        EXPECT_EQ(views::MenuItemView::NORMAL, item->GetType());
        break;
      case ui::MenuModel::TYPE_CHECK:
        EXPECT_EQ(views::MenuItemView::CHECKBOX, item->GetType());
        break;
      case ui::MenuModel::TYPE_RADIO:
        EXPECT_EQ(views::MenuItemView::RADIO, item->GetType());
        break;
      case ui::MenuModel::TYPE_SEPARATOR:
      case ui::MenuModel::TYPE_BUTTON_ITEM:
        break;
      case ui::MenuModel::TYPE_SUBMENU:
        EXPECT_EQ(views::MenuItemView::SUBMENU, item->GetType());
        break;
    }

    // Check activation.
    static_cast<views::MenuDelegate*>(&delegate)->ExecuteCommand(id);
    EXPECT_EQ(i, model.last_activation());
    model.set_last_activation(-1);
  }

  // Check submenu items.
  views::MenuItemView* submenu = menu->GetMenuItemByID(103);
  views::SubmenuView* subitem_container = submenu->GetSubmenu();
  EXPECT_EQ(2, subitem_container->child_count());

  for (int i = 0; i < subitem_container->child_count(); ++i) {
    MenuModelBase* submodel = static_cast<MenuModelBase*>(
        model.GetSubmenuModelAt(3));
    EXPECT_TRUE(submodel);

    const MenuModelBase::Item& model_item = submodel->GetItemDefinition(i);

    const int id = i + kSubmenuIdBase;
    MenuItemView* item = menu->GetMenuItemByID(id);
    if (!item) {
      EXPECT_EQ(ui::MenuModel::TYPE_SEPARATOR, model_item.type);
      continue;
    }

    // Check placement.
    EXPECT_EQ(i, submenu->GetSubmenu()->GetIndexOf(item));

    // Check type.
    switch (model_item.type) {
      case ui::MenuModel::TYPE_COMMAND:
        EXPECT_EQ(views::MenuItemView::NORMAL, item->GetType());
        break;
      case ui::MenuModel::TYPE_CHECK:
        EXPECT_EQ(views::MenuItemView::CHECKBOX, item->GetType());
        break;
      case ui::MenuModel::TYPE_RADIO:
        EXPECT_EQ(views::MenuItemView::RADIO, item->GetType());
        break;
      case ui::MenuModel::TYPE_SEPARATOR:
      case ui::MenuModel::TYPE_BUTTON_ITEM:
        break;
      case ui::MenuModel::TYPE_SUBMENU:
        EXPECT_EQ(views::MenuItemView::SUBMENU, item->GetType());
        break;
    }

    // Check activation.
    static_cast<views::MenuDelegate*>(&delegate)->ExecuteCommand(id);
    EXPECT_EQ(i, submodel->last_activation());
    submodel->set_last_activation(-1);
  }

  // Check that selecting the root item is safe.  The MenuModel does
  // not care about the root so MenuModelAdapter should do nothing
  // (not hit the NOTREACHED check) when the root is selected.
  static_cast<views::MenuDelegate*>(&delegate)->SelectionChanged(menu);
}

}  // namespace views
