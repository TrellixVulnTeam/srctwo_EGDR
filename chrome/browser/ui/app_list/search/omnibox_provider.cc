// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/app_list/search/omnibox_provider.h"

#include "base/memory/ptr_util.h"
#include "chrome/browser/autocomplete/chrome_autocomplete_provider_client.h"
#include "chrome/browser/autocomplete/chrome_autocomplete_scheme_classifier.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/search_engines/template_url_service_factory.h"
#include "chrome/browser/ui/app_list/app_list_controller_delegate.h"
#include "chrome/browser/ui/app_list/search/omnibox_result.h"
#include "components/metrics/proto/omnibox_event.pb.h"
#include "components/omnibox/browser/autocomplete_classifier.h"
#include "components/omnibox/browser/autocomplete_controller.h"
#include "components/omnibox/browser/autocomplete_input.h"
#include "ui/app_list/search_result.h"
#include "url/gurl.h"

namespace app_list {

OmniboxProvider::OmniboxProvider(Profile* profile,
                                 AppListControllerDelegate* list_controller)
    : profile_(profile),
      list_controller_(list_controller),
      controller_(new AutocompleteController(
          base::WrapUnique(new ChromeAutocompleteProviderClient(profile)),
          this,
          AutocompleteClassifier::DefaultOmniboxProviders() &
              ~AutocompleteProvider::TYPE_ZERO_SUGGEST)),
      is_voice_query_(false) {}

OmniboxProvider::~OmniboxProvider() {}

void OmniboxProvider::Start(bool is_voice_query, const base::string16& query) {
  is_voice_query_ = is_voice_query;
  controller_->Start(AutocompleteInput(
      query, base::string16::npos, std::string(), GURL(), base::string16(),
      metrics::OmniboxEventProto::INVALID_SPEC, false, false, true, true, false,
      ChromeAutocompleteSchemeClassifier(profile_)));
}

void OmniboxProvider::Stop() {
  controller_->Stop(false);
  is_voice_query_ = false;
}

void OmniboxProvider::PopulateFromACResult(const AutocompleteResult& result) {
  SearchProvider::Results new_results;
  new_results.reserve(result.size());
  for (const AutocompleteMatch& match : result) {
    if (!match.destination_url.is_valid())
      continue;

    new_results.emplace_back(base::MakeUnique<OmniboxResult>(
        profile_, list_controller_, controller_.get(), is_voice_query_, match));
  }
  SwapResults(&new_results);
}

void OmniboxProvider::OnResultChanged(bool default_match_changed) {
  PopulateFromACResult(controller_->result());
}

}  // namespace app_list
