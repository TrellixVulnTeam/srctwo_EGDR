// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/chrome/browser/ntp_tiles/ios_popular_sites_factory.h"

#include "base/bind.h"
#include "base/files/file_path.h"
#include "base/memory/ptr_util.h"
#include "components/ntp_tiles/json_unsafe_parser.h"
#include "components/ntp_tiles/popular_sites_impl.h"
#include "ios/chrome/browser/application_context.h"
#include "ios/chrome/browser/browser_state/chrome_browser_state.h"
#include "ios/chrome/browser/search_engines/template_url_service_factory.h"
#include "ios/web/public/web_thread.h"

std::unique_ptr<ntp_tiles::PopularSites>
IOSPopularSitesFactory::NewForBrowserState(
    ios::ChromeBrowserState* browser_state) {
  return base::MakeUnique<ntp_tiles::PopularSitesImpl>(
      browser_state->GetPrefs(),
      ios::TemplateURLServiceFactory::GetForBrowserState(browser_state),
      GetApplicationContext()->GetVariationsService(),
      browser_state->GetRequestContext(),
      base::Bind(ntp_tiles::JsonUnsafeParser::Parse));
}
