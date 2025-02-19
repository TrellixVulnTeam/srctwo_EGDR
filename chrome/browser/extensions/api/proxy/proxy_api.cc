// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Implementation of the Chrome Extensions Proxy Settings API.

#include "chrome/browser/extensions/api/proxy/proxy_api.h"

#include <memory>
#include <utility>

#include "base/json/json_writer.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "chrome/browser/extensions/api/proxy/proxy_api_constants.h"
#include "chrome/browser/extensions/api/proxy/proxy_api_helpers.h"
#include "chrome/browser/extensions/event_router_forwarder.h"
#include "chrome/browser/extensions/extension_service.h"
#include "components/proxy_config/proxy_config_dictionary.h"
#include "net/base/net_errors.h"

namespace extensions {

namespace helpers = proxy_api_helpers;
namespace keys = proxy_api_constants;

// static
ProxyEventRouter* ProxyEventRouter::GetInstance() {
  return base::Singleton<ProxyEventRouter>::get();
}

ProxyEventRouter::ProxyEventRouter() {
}

ProxyEventRouter::~ProxyEventRouter() {
}

void ProxyEventRouter::OnProxyError(
    EventRouterForwarder* event_router,
    void* profile,
    int error_code) {
  std::unique_ptr<base::ListValue> args(new base::ListValue());
  std::unique_ptr<base::DictionaryValue> dict(new base::DictionaryValue());
  dict->SetBoolean(keys::kProxyEventFatal, true);
  dict->SetString(keys::kProxyEventError, net::ErrorToString(error_code));
  dict->SetString(keys::kProxyEventDetails, std::string());
  args->Append(std::move(dict));

  if (profile) {
    event_router->DispatchEventToRenderers(
        events::PROXY_ON_PROXY_ERROR, keys::kProxyEventOnProxyError,
        std::move(args), profile, true, GURL());
  } else {
    event_router->BroadcastEventToRenderers(events::PROXY_ON_PROXY_ERROR,
                                            keys::kProxyEventOnProxyError,
                                            std::move(args), GURL());
  }
}

void ProxyEventRouter::OnPACScriptError(
    EventRouterForwarder* event_router,
    void* profile,
    int line_number,
    const base::string16& error) {
  std::unique_ptr<base::ListValue> args(new base::ListValue());
  std::unique_ptr<base::DictionaryValue> dict(new base::DictionaryValue());
  dict->SetBoolean(keys::kProxyEventFatal, false);
  dict->SetString(keys::kProxyEventError,
                  net::ErrorToString(net::ERR_PAC_SCRIPT_FAILED));
  std::string error_msg;
  if (line_number != -1) {
    base::SStringPrintf(&error_msg,
                        "line: %d: %s",
                        line_number, base::UTF16ToUTF8(error).c_str());
  } else {
    error_msg = base::UTF16ToUTF8(error);
  }
  dict->SetString(keys::kProxyEventDetails, error_msg);
  args->Append(std::move(dict));

  if (profile) {
    event_router->DispatchEventToRenderers(
        events::PROXY_ON_PROXY_ERROR, keys::kProxyEventOnProxyError,
        std::move(args), profile, true, GURL());
  } else {
    event_router->BroadcastEventToRenderers(events::PROXY_ON_PROXY_ERROR,
                                            keys::kProxyEventOnProxyError,
                                            std::move(args), GURL());
  }
}

ProxyPrefTransformer::ProxyPrefTransformer() {
}

ProxyPrefTransformer::~ProxyPrefTransformer() {
}

std::unique_ptr<base::Value> ProxyPrefTransformer::ExtensionToBrowserPref(
    const base::Value* extension_pref,
    std::string* error,
    bool* bad_message) {
  // When ExtensionToBrowserPref is called, the format of |extension_pref|
  // has been verified already by the extension API to match the schema
  // defined in the extension API JSON.
  CHECK(extension_pref->IsType(base::Value::Type::DICTIONARY));
  const base::DictionaryValue* config =
      static_cast<const base::DictionaryValue*>(extension_pref);

  // Extract the various pieces of information passed to
  // chrome.proxy.settings.set(). Several of these strings will
  // remain blank no respective values have been passed to set().
  // If a values has been passed to set but could not be parsed, we bail
  // out and return NULL.
  ProxyPrefs::ProxyMode mode_enum;
  bool pac_mandatory;
  std::string pac_url;
  std::string pac_data;
  std::string proxy_rules_string;
  std::string bypass_list;
  if (!helpers::GetProxyModeFromExtensionPref(
          config, &mode_enum, error, bad_message) ||
      !helpers::GetPacMandatoryFromExtensionPref(
          config, &pac_mandatory, error, bad_message) ||
      !helpers::GetPacUrlFromExtensionPref(
          config, &pac_url, error, bad_message) ||
      !helpers::GetPacDataFromExtensionPref(
          config, &pac_data, error, bad_message) ||
      !helpers::GetProxyRulesStringFromExtensionPref(
          config, &proxy_rules_string, error, bad_message) ||
      !helpers::GetBypassListFromExtensionPref(
          config, &bypass_list, error, bad_message)) {
    return nullptr;
  }

  return helpers::CreateProxyConfigDict(mode_enum, pac_mandatory, pac_url,
                                        pac_data, proxy_rules_string,
                                        bypass_list, error);
}

std::unique_ptr<base::Value> ProxyPrefTransformer::BrowserToExtensionPref(
    const base::Value* browser_pref) {
  CHECK(browser_pref->IsType(base::Value::Type::DICTIONARY));

  // This is a dictionary wrapper that exposes the proxy configuration stored in
  // the browser preferences.
  ProxyConfigDictionary config(
      static_cast<const base::DictionaryValue*>(browser_pref)
          ->CreateDeepCopy());

  ProxyPrefs::ProxyMode mode;
  if (!config.GetMode(&mode)) {
    LOG(ERROR) << "Cannot determine proxy mode.";
    return nullptr;
  }

  // Build a new ProxyConfig instance as defined in the extension API.
  std::unique_ptr<base::DictionaryValue> extension_pref(
      new base::DictionaryValue);

  extension_pref->SetString(keys::kProxyConfigMode,
                            ProxyPrefs::ProxyModeToString(mode));

  switch (mode) {
    case ProxyPrefs::MODE_DIRECT:
    case ProxyPrefs::MODE_AUTO_DETECT:
    case ProxyPrefs::MODE_SYSTEM:
      // These modes have no further parameters.
      break;
    case ProxyPrefs::MODE_PAC_SCRIPT: {
      // A PAC URL either point to a PAC script or contain a base64 encoded
      // PAC script. In either case we build a PacScript dictionary as defined
      // in the extension API.
      std::unique_ptr<base::DictionaryValue> pac_dict =
          helpers::CreatePacScriptDict(config);
      if (!pac_dict)
        return nullptr;
      extension_pref->Set(keys::kProxyConfigPacScript, std::move(pac_dict));
      break;
    }
    case ProxyPrefs::MODE_FIXED_SERVERS: {
      // Build ProxyRules dictionary according to the extension API.
      std::unique_ptr<base::DictionaryValue> proxy_rules_dict =
          helpers::CreateProxyRulesDict(config);
      if (!proxy_rules_dict)
        return nullptr;
      extension_pref->Set(keys::kProxyConfigRules, std::move(proxy_rules_dict));
      break;
    }
    case ProxyPrefs::kModeCount:
      NOTREACHED();
  }
  // TODO(crbug.com/703565): remove std::move() once compiler support improves.
  return std::move(extension_pref);
}

}  // namespace extensions
