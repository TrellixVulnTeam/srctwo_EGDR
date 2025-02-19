// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/safe_browsing/trigger_creator.h"

#include "chrome/browser/browser_process.h"
#include "chrome/browser/history/history_service_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/safe_browsing/safe_browsing_service.h"
#include "components/prefs/pref_service.h"
#include "components/safe_browsing/triggers/ad_sampler_trigger.h"
#include "components/safe_browsing/triggers/trigger_manager.h"
#include "components/safe_browsing/triggers/trigger_throttler.h"
#include "components/security_interstitials/core/base_safe_browsing_error_ui.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "net/url_request/url_request_context_getter.h"

namespace safe_browsing {

using SBErrorOptions =
    security_interstitials::BaseSafeBrowsingErrorUI::SBErrorDisplayOptions;

void TriggerCreator::MaybeCreateTriggersForWebContents(
    Profile* profile,
    content::WebContents* web_contents) {
  if (!g_browser_process->safe_browsing_service() ||
      !g_browser_process->safe_browsing_service()->trigger_manager()) {
    return;
  }

  // We only start triggers for this tab if they are eligible to collect data
  // (eg: because of opt-ins, available quota, etc). If we skip a trigger but
  // later opt-in changes or quota becomes available, the trigger won't be
  // running on old tabs, but that's acceptable. The trigger will be started for
  // new tabs.
  TriggerManager* trigger_manager =
      g_browser_process->safe_browsing_service()->trigger_manager();
  SBErrorOptions options = TriggerManager::GetSBErrorDisplayOptions(
      *profile->GetPrefs(), *web_contents);
  if (trigger_manager->CanStartDataCollection(options,
                                              TriggerType::AD_SAMPLE)) {
    safe_browsing::AdSamplerTrigger::CreateForWebContents(
        web_contents, trigger_manager, profile->GetPrefs(),
        profile->GetRequestContext(),
        HistoryServiceFactory::GetForProfile(
            profile, ServiceAccessType::EXPLICIT_ACCESS));
  }
}

}  // namespace safe_browsing
