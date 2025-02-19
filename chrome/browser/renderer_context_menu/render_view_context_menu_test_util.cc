// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <vector>

#include "chrome/app/chrome_command_ids.h"
#include "chrome/browser/renderer_context_menu/render_view_context_menu_test_util.h"
#include "content/public/browser/web_contents.h"
#include "ui/base/models/menu_model.h"

using ui::MenuModel;

TestRenderViewContextMenu::TestRenderViewContextMenu(
    content::RenderFrameHost* render_frame_host,
    content::ContextMenuParams params)
    : RenderViewContextMenu(render_frame_host, params) {}

TestRenderViewContextMenu::~TestRenderViewContextMenu() {}

// static
std::unique_ptr<TestRenderViewContextMenu> TestRenderViewContextMenu::Create(
    content::WebContents* web_contents,
    const GURL& page_url,
    const GURL& link_url,
    const GURL& frame_url) {
  content::ContextMenuParams params;
  params.page_url = page_url;
  params.link_url = link_url;
  params.frame_url = frame_url;
  auto menu = base::MakeUnique<TestRenderViewContextMenu>(
      web_contents->GetMainFrame(), params);
  menu->Init();
  return menu;
}

bool TestRenderViewContextMenu::IsItemPresent(int command_id) const {
  return menu_model_.GetIndexOfCommandId(command_id) != -1;
}

bool TestRenderViewContextMenu::IsItemInRangePresent(
    int command_id_first,
    int command_id_last) const {
  DCHECK_LE(command_id_first, command_id_last);
  for (int command_id = command_id_first; command_id <= command_id_last;
       ++command_id) {
    if (IsItemPresent(command_id))
      return true;
  }
  return false;
}

bool TestRenderViewContextMenu::GetMenuModelAndItemIndex(
    int command_id,
    MenuModel** found_model,
    int* found_index) {
  std::vector<MenuModel*> models_to_search;
  models_to_search.push_back(&menu_model_);

  while (!models_to_search.empty()) {
    MenuModel* model = models_to_search.back();
    models_to_search.pop_back();
    for (int i = 0; i < model->GetItemCount(); i++) {
      if (model->GetCommandIdAt(i) == command_id) {
        *found_model = model;
        *found_index = i;
        return true;
      }
      if (model->GetTypeAt(i) == MenuModel::TYPE_SUBMENU) {
        models_to_search.push_back(model->GetSubmenuModelAt(i));
      }
    }
  }

  return false;
}

int TestRenderViewContextMenu::GetCommandIDByProfilePath(
    const base::FilePath& path) const {
  size_t count = profile_link_paths_.size();
  for (size_t i = 0; i < count; ++i) {
    if (profile_link_paths_[i] == path)
      return IDC_OPEN_LINK_IN_PROFILE_FIRST + static_cast<int>(i);
  }
  return -1;
}

void TestRenderViewContextMenu::Show() {
}
