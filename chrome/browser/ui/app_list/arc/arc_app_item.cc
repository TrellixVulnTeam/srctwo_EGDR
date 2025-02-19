// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/app_list/arc/arc_app_item.h"

#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/app_list/app_list_controller_delegate.h"
#include "chrome/browser/ui/app_list/arc/arc_app_context_menu.h"
#include "chrome/browser/ui/app_list/arc/arc_app_utils.h"
#include "components/arc/arc_bridge_service.h"
#include "content/public/browser/browser_thread.h"
#include "extensions/browser/app_sorting.h"
#include "ui/app_list/app_list_constants.h"
#include "ui/gfx/image/image_skia.h"

// static
const char ArcAppItem::kItemType[] = "ArcAppItem";

ArcAppItem::ArcAppItem(
    Profile* profile,
    const app_list::AppListSyncableService::SyncItem* sync_item,
    const std::string& id,
    const std::string& name)
    : ChromeAppListItem(profile, id) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  arc_app_icon_.reset(new ArcAppIcon(profile,
                                     id,
                                     app_list::kGridIconDimension,
                                     this));

  SetName(name);
  UpdateIcon();
  if (sync_item && sync_item->item_ordinal.IsValid())
    UpdateFromSync(sync_item);
  else
    SetDefaultPositionIfApplicable();
}

ArcAppItem::~ArcAppItem() {
}

const char* ArcAppItem::GetItemType() const {
  return ArcAppItem::kItemType;
}

void ArcAppItem::Activate(int event_flags) {
  if (!arc::LaunchApp(profile(), id(), event_flags))
    return;

  // Manually close app_list view because focus is not changed on ARC app start,
  // and current view remains active.
  GetController()->DismissView();
}

void ArcAppItem::ExecuteLaunchCommand(int event_flags) {
  Activate(event_flags);
}

void ArcAppItem::SetName(const std::string& name) {
  SetNameAndShortName(name, name);
}

void ArcAppItem::UpdateIcon() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);
  SetIcon(arc_app_icon_->image_skia());
}

void ArcAppItem::OnIconUpdated(ArcAppIcon* icon) {
  UpdateIcon();
}

ui::MenuModel* ArcAppItem::GetContextMenuModel() {
  context_menu_.reset(new ArcAppContextMenu(this,
                                            profile(),
                                            id(),
                                            GetController()));
  return context_menu_->GetMenuModel();
}
