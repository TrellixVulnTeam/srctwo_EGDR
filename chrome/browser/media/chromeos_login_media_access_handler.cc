// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/media/chromeos_login_media_access_handler.h"

#include <string>

#include "base/logging.h"
#include "base/values.h"
#include "chrome/browser/chromeos/login/ui/login_display_host.h"
#include "chrome/browser/chromeos/login/ui/webui_login_view.h"
#include "chrome/browser/chromeos/settings/cros_settings.h"
#include "chrome/common/url_constants.h"
#include "chromeos/settings/cros_settings_names.h"
#include "components/content_settings/core/common/content_settings_pattern.h"
#include "url/gurl.h"

ChromeOSLoginMediaAccessHandler::ChromeOSLoginMediaAccessHandler() {}

ChromeOSLoginMediaAccessHandler::~ChromeOSLoginMediaAccessHandler() {}

bool ChromeOSLoginMediaAccessHandler::SupportsStreamType(
    content::WebContents* web_contents,
    const content::MediaStreamType type,
    const extensions::Extension* extension) {
  if (!web_contents)
    return false;
  chromeos::LoginDisplayHost* login_display_host =
      chromeos::LoginDisplayHost::default_host();
  chromeos::WebUILoginView* webui_login_view =
      login_display_host ? login_display_host->GetWebUILoginView() : nullptr;
  content::WebContents* login_web_contents =
      webui_login_view ? webui_login_view->GetWebContents() : nullptr;
  return web_contents == login_web_contents;
}

bool ChromeOSLoginMediaAccessHandler::CheckMediaAccessPermission(
    content::WebContents* web_contents,
    const GURL& security_origin,
    content::MediaStreamType type,
    const extensions::Extension* extension) {
  if (type != content::MEDIA_DEVICE_VIDEO_CAPTURE)
    return false;

  // When creating new user (including supervised user), we must be able to use
  // the camera to capture a user image.
  if (security_origin.spec() == chrome::kChromeUIOobeURL)
    return true;

  const chromeos::CrosSettings* const settings = chromeos::CrosSettings::Get();
  if (!settings)
    return false;

  // The following checks are for SAML logins.
  const base::Value* const raw_list_value =
      settings->GetPref(chromeos::kLoginVideoCaptureAllowedUrls);
  if (!raw_list_value)
    return false;

  const base::ListValue* list_value;
  const bool is_list = raw_list_value->GetAsList(&list_value);
  DCHECK(is_list);
  for (const auto& base_value : *list_value) {
    std::string value;
    if (base_value.GetAsString(&value)) {
      const ContentSettingsPattern pattern =
          ContentSettingsPattern::FromString(value);
      // Force administrators to specify more-specific patterns by ignoring the
      // global wildcard pattern.
      if (pattern == ContentSettingsPattern::Wildcard()) {
        VLOG(1) << "Ignoring wildcard URL pattern: " << value;
        continue;
      }
      if (pattern.IsValid() && pattern.Matches(security_origin))
        return true;
    }
  }
  return false;
}

void ChromeOSLoginMediaAccessHandler::HandleRequest(
    content::WebContents* web_contents,
    const content::MediaStreamRequest& request,
    const content::MediaResponseCallback& callback,
    const extensions::Extension* extension) {
  bool audio_allowed = false;
  bool video_allowed =
      request.video_type == content::MEDIA_DEVICE_VIDEO_CAPTURE &&
      CheckMediaAccessPermission(web_contents, request.security_origin,
                                 content::MEDIA_DEVICE_VIDEO_CAPTURE,
                                 extension);

  CheckDevicesAndRunCallback(web_contents, request, callback, audio_allowed,
                             video_allowed);
}
