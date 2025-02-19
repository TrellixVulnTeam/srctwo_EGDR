// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/apps/app_info_dialog/arc_app_info_links_panel.h"

#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/app_list/arc/arc_app_utils.h"
#include "chrome/browser/ui/views/harmony/chrome_layout_provider.h"
#include "chrome/grit/generated_resources.h"
#include "components/arc/common/app.mojom.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/gfx/geometry/insets.h"
#include "ui/views/controls/link.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/view.h"

namespace {
constexpr char kArcChromePackageName[] = "org.chromium.arc.intent_helper";
}

ArcAppInfoLinksPanel::ArcAppInfoLinksPanel(Profile* profile,
                                           const extensions::Extension* app)
    : AppInfoPanel(profile, app),
      app_list_observer_(this),
      manage_link_(nullptr) {
  SetLayoutManager(
      new views::BoxLayout(views::BoxLayout::kVertical, gfx::Insets(),
                           ChromeLayoutProvider::Get()->GetDistanceMetric(
                               views::DISTANCE_RELATED_CONTROL_VERTICAL)));
  manage_link_ = new views::Link(
      l10n_util::GetStringUTF16(IDS_ARC_APPLICATION_INFO_MANAGE_LINK));
  manage_link_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
  manage_link_->set_listener(this);
  AddChildView(manage_link_);

  ArcAppListPrefs* const arc_prefs = ArcAppListPrefs::Get(profile_);
  DCHECK(arc_prefs);
  app_list_observer_.Add(arc_prefs);

  std::unique_ptr<ArcAppListPrefs::AppInfo> app_info =
      ArcAppListPrefs::Get(profile)->GetApp(arc::kSettingsAppId);
  if (app_info)
    UpdateLink(app_info->ready);
}

ArcAppInfoLinksPanel::~ArcAppInfoLinksPanel() {}

void ArcAppInfoLinksPanel::LinkClicked(views::Link* source, int event_flags) {
  DCHECK_EQ(manage_link_, source);
  if (arc::ShowPackageInfoOnPage(
          kArcChromePackageName,
          arc::mojom::ShowPackageInfoPage::MANAGE_LINKS)) {
    Close();
  }
}

void ArcAppInfoLinksPanel::OnAppReadyChanged(const std::string& app_id,
                                             bool ready) {
  if (app_id == arc::kSettingsAppId)
    UpdateLink(ready);
}

void ArcAppInfoLinksPanel::OnAppRemoved(const std::string& app_id) {
  if (app_id == arc::kSettingsAppId)
    UpdateLink(false);
}

void ArcAppInfoLinksPanel::OnAppRegistered(
    const std::string& app_id,
    const ArcAppListPrefs::AppInfo& app_info) {
  if (app_id == arc::kSettingsAppId)
    UpdateLink(app_info.ready);
}

void ArcAppInfoLinksPanel::UpdateLink(bool enabled) {
  manage_link_->SetEnabled(enabled);
}
