// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/chrome/browser/autocomplete/autocomplete_classifier_factory.h"

#include "base/memory/ptr_util.h"
#include "base/memory/singleton.h"
#include "components/keyed_service/ios/browser_state_dependency_manager.h"
#include "components/omnibox/browser/autocomplete_classifier.h"
#include "components/omnibox/browser/autocomplete_controller.h"
#include "ios/chrome/browser/autocomplete/autocomplete_provider_client_impl.h"
#include "ios/chrome/browser/autocomplete/autocomplete_scheme_classifier_impl.h"
#include "ios/chrome/browser/autocomplete/in_memory_url_index_factory.h"
#include "ios/chrome/browser/autocomplete/shortcuts_backend_factory.h"
#include "ios/chrome/browser/browser_state/browser_state_otr_helper.h"
#include "ios/chrome/browser/browser_state/chrome_browser_state.h"
#include "ios/chrome/browser/search_engines/template_url_service_factory.h"

namespace ios {
namespace {

std::unique_ptr<KeyedService> BuildAutocompleteClassifier(
    web::BrowserState* context) {
  ios::ChromeBrowserState* browser_state =
      ios::ChromeBrowserState::FromBrowserState(context);
  return base::MakeUnique<AutocompleteClassifier>(
      base::WrapUnique(new AutocompleteController(
          base::WrapUnique(new AutocompleteProviderClientImpl(browser_state)),
          nullptr, AutocompleteClassifier::DefaultOmniboxProviders())),
      base::WrapUnique(new AutocompleteSchemeClassifierImpl));
}

}  // namespace

// static
AutocompleteClassifier* AutocompleteClassifierFactory::GetForBrowserState(
    ios::ChromeBrowserState* browser_state) {
  return static_cast<AutocompleteClassifier*>(
      GetInstance()->GetServiceForBrowserState(browser_state, true));
}

// static
AutocompleteClassifierFactory* AutocompleteClassifierFactory::GetInstance() {
  return base::Singleton<AutocompleteClassifierFactory>::get();
}

// static
BrowserStateKeyedServiceFactory::TestingFactoryFunction
AutocompleteClassifierFactory::GetDefaultFactory() {
  return &BuildAutocompleteClassifier;
}

AutocompleteClassifierFactory::AutocompleteClassifierFactory()
    : BrowserStateKeyedServiceFactory(
          "AutocompleteClassifier",
          BrowserStateDependencyManager::GetInstance()) {
  DependsOn(ios::InMemoryURLIndexFactory::GetInstance());
  DependsOn(ios::TemplateURLServiceFactory::GetInstance());
  DependsOn(ios::ShortcutsBackendFactory::GetInstance());
}

AutocompleteClassifierFactory::~AutocompleteClassifierFactory() {}

std::unique_ptr<KeyedService>
AutocompleteClassifierFactory::BuildServiceInstanceFor(
    web::BrowserState* context) const {
  return BuildAutocompleteClassifier(context);
}

web::BrowserState* AutocompleteClassifierFactory::GetBrowserStateToUse(
    web::BrowserState* context) const {
  return GetBrowserStateRedirectedInIncognito(context);
}

bool AutocompleteClassifierFactory::ServiceIsNULLWhileTesting() const {
  return true;
}

}  // namespace ios
