// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/external_component_loader.h"

#include "base/command_line.h"
#include "build/build_config.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/extensions/component_extensions_whitelist/whitelist.h"
#include "chrome/browser/media/router/media_router_feature.h"
#include "chrome/browser/search/hotword_service.h"
#include "chrome/browser/search/hotword_service_factory.h"
#include "chrome/browser/signin/signin_manager_factory.h"
#include "chrome/common/extensions/extension_constants.h"
#include "chrome/common/features.h"
#include "components/signin/core/browser/signin_manager.h"
#include "extensions/common/extension_urls.h"
#include "extensions/common/feature_switch.h"
#include "extensions/common/manifest.h"

#if defined(OS_CHROMEOS)
#include "base/command_line.h"
#include "chromeos/chromeos_switches.h"
#endif

#if BUILDFLAG(ENABLE_APP_LIST) && defined(OS_CHROMEOS)
#include "chrome/browser/ui/app_list/google_now_extension.h"
#endif

namespace extensions {

ExternalComponentLoader::ExternalComponentLoader(Profile* profile)
    : profile_(profile) {
}

ExternalComponentLoader::~ExternalComponentLoader() {}

void ExternalComponentLoader::StartLoading() {
  auto prefs = std::make_unique<base::DictionaryValue>();
#if defined(GOOGLE_CHROME_BUILD)
  AddExternalExtension(extension_misc::kInAppPaymentsSupportAppId, prefs.get());
#endif  // defined(GOOGLE_CHROME_BUILD)

  if (HotwordServiceFactory::IsHotwordAllowed(profile_))
    AddExternalExtension(extension_misc::kHotwordSharedModuleId, prefs.get());

#if defined(OS_CHROMEOS)
  {
    base::CommandLine* const command_line =
        base::CommandLine::ForCurrentProcess();
    if (!command_line->HasSwitch(chromeos::switches::kDisableNewZIPUnpacker))
      AddExternalExtension(extension_misc::kZIPUnpackerExtensionId,
                           prefs.get());
  }
#endif

  if (media_router::MediaRouterEnabled(profile_) &&
      FeatureSwitch::load_media_router_component_extension()->IsEnabled()) {
    AddExternalExtension(extension_misc::kMediaRouterStableExtensionId,
                         prefs.get());
  }

#if BUILDFLAG(ENABLE_APP_LIST) && defined(OS_CHROMEOS)
  std::string google_now_extension_id;
  if (GetGoogleNowExtensionId(&google_now_extension_id))
    AddExternalExtension(google_now_extension_id, prefs.get());
#endif

  LoadFinished(std::move(prefs));
}

void ExternalComponentLoader::AddExternalExtension(
    const std::string& extension_id,
    base::DictionaryValue* prefs) {
  if (!IsComponentExtensionWhitelisted(extension_id))
    return;

  prefs->SetString(extension_id + ".external_update_url",
                   extension_urls::GetWebstoreUpdateUrl().spec());
}

}  // namespace extensions
