// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/autocomplete/chrome_autocomplete_scheme_classifier.h"

#include "base/strings/string_util.h"
#include "chrome/browser/custom_handlers/protocol_handler_registry_factory.h"
#include "chrome/browser/external_protocol/external_protocol_handler.h"
#include "chrome/browser/profiles/profile_io_data.h"
#include "content/public/common/url_constants.h"
#include "url/url_util.h"

ChromeAutocompleteSchemeClassifier::ChromeAutocompleteSchemeClassifier(
    Profile* profile)
    : profile_(profile) {
}

ChromeAutocompleteSchemeClassifier::~ChromeAutocompleteSchemeClassifier() {
}

metrics::OmniboxInputType
ChromeAutocompleteSchemeClassifier::GetInputTypeForScheme(
    const std::string& scheme) const {
  if (base::IsStringASCII(scheme) &&
      (ProfileIOData::IsHandledProtocol(scheme) ||
       base::LowerCaseEqualsASCII(scheme, content::kViewSourceScheme) ||
       base::LowerCaseEqualsASCII(scheme, url::kJavaScriptScheme) ||
       base::LowerCaseEqualsASCII(scheme, url::kDataScheme))) {
    return metrics::OmniboxInputType::URL;
  }

  // Also check for schemes registered via registerProtocolHandler(), which
  // can be handled by web pages/apps.
  ProtocolHandlerRegistry* registry = profile_ ?
      ProtocolHandlerRegistryFactory::GetForBrowserContext(profile_) : NULL;
  if (registry && registry->IsHandledProtocol(scheme))
    return metrics::OmniboxInputType::URL;

  // Not an internal protocol; check if it's an external protocol, i.e. one
  // that's registered on the user's OS and will shell out to another program.
  //
  // We need to do this after the checks above because some internally
  // handlable schemes (e.g. "javascript") may be treated as "blocked" by the
  // external protocol handler because we don't want pages to open them, but
  // users still can.
  const ExternalProtocolHandler::BlockState block_state =
      ExternalProtocolHandler::GetBlockState(scheme, profile_);
  switch (block_state) {
    case ExternalProtocolHandler::DONT_BLOCK:
      return metrics::OmniboxInputType::URL;

    case ExternalProtocolHandler::BLOCK:
      // If we don't want the user to open the URL, don't let it be navigated
      // to at all.
      return metrics::OmniboxInputType::QUERY;

    default:
      return metrics::OmniboxInputType::INVALID;
  }
}
