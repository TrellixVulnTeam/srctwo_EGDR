// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/browser_instant_controller.h"

#include "base/bind.h"
#include "chrome/browser/infobars/infobar_service.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/search/instant_service.h"
#include "chrome/browser/search/instant_service_factory.h"
#include "chrome/browser/search/search.h"
#include "chrome/browser/search_engines/template_url_service_factory.h"
#include "chrome/browser/search_engines/ui_thread_search_terms_data.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/search/search_model.h"
#include "chrome/browser/ui/search/search_tab_helper.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/common/search/instant_types.h"
#include "chrome/common/url_constants.h"
#include "components/omnibox/browser/omnibox_popup_model.h"
#include "components/omnibox/browser/omnibox_view.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_user_data.h"
#include "content/public/common/referrer.h"
#include "ui/base/page_transition_types.h"

// Helpers --------------------------------------------------------------------

namespace {

// Helper class for posting a task to reload a tab, to avoid doing a re-entrant
// navigation, since it can be called when starting a navigation. This class
// makes sure to only execute the reload if the WebContents still exists.
class TabReloader : public content::WebContentsUserData<TabReloader> {
 public:
  ~TabReloader() override {}

  static void Reload(content::WebContents* web_contents) {
    TabReloader::CreateForWebContents(web_contents);
  }

 private:
  friend class content::WebContentsUserData<TabReloader>;

  explicit TabReloader(content::WebContents* web_contents)
      : web_contents_(web_contents), weak_ptr_factory_(this) {
    content::BrowserThread::PostTask(
        content::BrowserThread::UI, FROM_HERE,
        base::BindOnce(&TabReloader::ReloadImpl,
                       weak_ptr_factory_.GetWeakPtr()));
  }

  void ReloadImpl() {
    web_contents_->GetController().Reload(content::ReloadType::NORMAL, false);

    // As the reload was not triggered by the user we don't want to close any
    // infobars. We have to tell the InfoBarService after the reload,
    // otherwise it would ignore this call when
    // WebContentsObserver::DidStartNavigationToPendingEntry is invoked.
    InfoBarService::FromWebContents(web_contents_)->set_ignore_next_reload();

    web_contents_->RemoveUserData(UserDataKey());
  }

  content::WebContents* web_contents_;
  base::WeakPtrFactory<TabReloader> weak_ptr_factory_;
};

}  // namespace

DEFINE_WEB_CONTENTS_USER_DATA_KEY(TabReloader);


// BrowserInstantController ---------------------------------------------------

BrowserInstantController::BrowserInstantController(Browser* browser)
    : browser_(browser),
      instant_(this) {
  browser_->search_model()->AddObserver(this);

  TemplateURLService* template_url_service =
      TemplateURLServiceFactory::GetForProfile(profile());
  // TemplateURLService can be null in tests.
  if (template_url_service) {
    search_engine_base_url_tracker_ =
        base::MakeUnique<SearchEngineBaseURLTracker>(
            template_url_service,
            base::MakeUnique<UIThreadSearchTermsData>(profile()),
            base::Bind(&BrowserInstantController::OnSearchEngineBaseURLChanged,
                       base::Unretained(this)));
  }
}

BrowserInstantController::~BrowserInstantController() {
  browser_->search_model()->RemoveObserver(this);
}

Profile* BrowserInstantController::profile() const {
  return browser_->profile();
}

content::WebContents* BrowserInstantController::GetActiveWebContents() const {
  return browser_->tab_strip_model()->GetActiveWebContents();
}

void BrowserInstantController::ActiveTabChanged() {
  instant_.ActiveTabChanged();
}

void BrowserInstantController::ModelChanged(SearchModel::Origin old_origin,
                                            SearchModel::Origin new_origin) {
  instant_.SearchModeChanged(old_origin, new_origin);
}

void BrowserInstantController::OnSearchEngineBaseURLChanged(
    SearchEngineBaseURLTracker::ChangeReason change_reason) {
  InstantService* instant_service =
      InstantServiceFactory::GetForProfile(profile());
  if (!instant_service)
    return;

  TabStripModel* tab_model = browser_->tab_strip_model();
  int count = tab_model->count();
  for (int index = 0; index < count; ++index) {
    content::WebContents* contents = tab_model->GetWebContentsAt(index);
    if (!contents)
      continue;

    // Send new search URLs to the renderer.
    content::RenderProcessHost* rph = contents->GetRenderProcessHost();
    instant_service->SendSearchURLsToRenderer(rph);

    if (!instant_service->IsInstantProcess(rph->GetID()))
      continue;

    bool google_base_url_domain_changed =
        change_reason ==
        SearchEngineBaseURLTracker::ChangeReason::GOOGLE_BASE_URL;
    SearchModel* model = SearchTabHelper::FromWebContents(contents)->model();
    if (google_base_url_domain_changed &&
        model->origin() == SearchModel::Origin::NTP) {
      GURL local_ntp_url(chrome::kChromeSearchLocalNtpUrl);
      // Replace the server NTP with the local NTP.
      content::NavigationController::LoadURLParams params(local_ntp_url);
      params.should_replace_current_entry = true;
      params.referrer = content::Referrer();
      params.transition_type = ui::PAGE_TRANSITION_RELOAD;
      contents->GetController().LoadURLWithParams(params);
    } else {
      // Reload the contents to ensure that it gets assigned to a
      // non-privileged renderer.
      TabReloader::Reload(contents);
    }
  }
}
