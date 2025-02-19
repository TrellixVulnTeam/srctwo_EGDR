// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/chrome/browser/passwords/credential_manager_util.h"

#include "components/security_state/core/security_state.h"
#include "ios/chrome/browser/payments/origin_security_checker.h"
#include "ios/chrome/browser/ssl/ios_security_state_tab_helper.h"
#import "ios/web/public/origin_util.h"
#include "url/origin.h"

using password_manager::CredentialManagerError;
using password_manager::CredentialInfo;
using password_manager::CredentialType;
using password_manager::CredentialMediationRequirement;

// TODO(crbug.com/435048): This file should not depend on payments. As soon as
// https://chromium-review.googlesource.com/c/chromium/src/+/631881 is landed
// make sure there are no payments dependencies.
using payments::OriginSecurityChecker;

namespace credential_manager {

namespace {

security_state::SecurityLevel GetSecurityLevelForWebState(
    const web::WebState* web_state) {
  if (!web_state) {
    return security_state::NONE;
  }
  auto* client = IOSSecurityStateTabHelper::FromWebState(web_state);
  if (!client) {
    return security_state::NONE;
  }
  security_state::SecurityInfo result;
  client->GetSecurityInfo(&result);
  return result.security_level;
}

}  // namespace

const char kCredentialIdKey[] = "id";
const char kCredentialTypeKey[] = "type";
const char kCredentialNameKey[] = "name";
const char kCredentialIconKey[] = "iconURL";
const char kPasswordCredentialPasswordKey[] = "password";
const char kFederatedCredentialProviderKey[] = "provider";
const char kCredentialRequestMediationKey[] = "mediation";
const char kCredentialRequestPasswordKey[] = "password";
const char kCredentialRequestProvidersKey[] = "providers";
const char kMediationRequirementSilent[] = "silent";
const char kMediationRequirementRequired[] = "required";
const char kMediationRequirementOptional[] = "optional";
const char kCredentialTypePassword[] = "PasswordCredential";
const char kCredentialTypeFederated[] = "FederatedCredential";

bool ParseMediationRequirement(const base::DictionaryValue& json,
                               CredentialMediationRequirement* mediation) {
  if (!json.HasKey(kCredentialRequestMediationKey)) {
    *mediation = CredentialMediationRequirement::kOptional;
    return true;
  }
  std::string mediation_str;
  if (!json.GetString(kCredentialRequestMediationKey, &mediation_str)) {
    // Dictionary contains |mediation| field but it is not a valid string.
    return false;
  }
  if (mediation_str == kMediationRequirementSilent) {
    *mediation = CredentialMediationRequirement::kSilent;
    return true;
  }
  if (mediation_str == kMediationRequirementRequired) {
    *mediation = CredentialMediationRequirement::kRequired;
    return true;
  }
  if (mediation_str == kMediationRequirementOptional) {
    *mediation = CredentialMediationRequirement::kOptional;
    return true;
  }
  // |mediation| value is not in {"silent", "required", "optional"}.
  // Dictionary is invalid then.
  return false;
}

bool ParseIncludePasswords(const base::DictionaryValue& json,
                           bool* include_passwords) {
  if (!json.HasKey(kCredentialRequestPasswordKey)) {
    *include_passwords = false;
    return true;
  }
  return json.GetBoolean(kCredentialRequestPasswordKey, include_passwords);
}

bool ParseFederations(const base::DictionaryValue& json,
                      std::vector<GURL>* federations) {
  federations->clear();
  if (!json.HasKey(kCredentialRequestProvidersKey)) {
    // No |providers| list.
    return true;
  }
  const base::ListValue* lst = nullptr;
  if (!json.GetList(kCredentialRequestProvidersKey, &lst)) {
    // Dictionary has |providers| field but it is not a list. That means
    // dictionary is invalid.
    return false;
  }
  for (size_t i = 0; i < lst->GetSize(); i++) {
    std::string s;
    if (!lst->GetString(i, &s)) {
      // Element of |providers| is invalid string.
      return false;
    }
    GURL gurl(s);
    if (gurl.is_valid()) {
      // Skip the invalid URLs. See
      // https://w3c.github.io/webappsec-credential-management/#provider-identification
      federations->push_back(std::move(gurl));
    }
  }
  return true;
}

bool ParseCredentialType(const base::DictionaryValue& json,
                         CredentialType* credential_type) {
  std::string str;
  if (!json.GetString(kCredentialTypeKey, &str)) {
    // Credential must contain |type|
    return false;
  }
  if (str == kCredentialTypePassword) {
    *credential_type = CredentialType::CREDENTIAL_TYPE_PASSWORD;
    return true;
  }
  if (str == kCredentialTypeFederated) {
    *credential_type = CredentialType::CREDENTIAL_TYPE_FEDERATED;
    return true;
  }
  return false;
}

bool ParseCredentialDictionary(const base::DictionaryValue& json,
                               CredentialInfo* credential) {
  if (!json.GetString(kCredentialIdKey, &credential->id)) {
    // |id| is required.
    return false;
  }
  json.GetString(kCredentialNameKey, &credential->name);
  std::string iconUrl;
  if (json.GetString(kCredentialIconKey, &iconUrl) && !iconUrl.empty()) {
    credential->icon = GURL(iconUrl);
    if (!credential->icon.is_valid() ||
        !web::IsOriginSecure(credential->icon)) {
      // |iconUrl| is either not a valid URL or not a secure URL.
      return false;
    }
  }
  if (!ParseCredentialType(json, &credential->type)) {
    // Credential has invalid |type|
    return false;
  }
  if (credential->type == CredentialType::CREDENTIAL_TYPE_PASSWORD) {
    if (!json.GetString(kPasswordCredentialPasswordKey,
                        &credential->password) ||
        credential->password.empty()) {
      // |password| field is required for PasswordCredential.
      return false;
    }
  }
  if (credential->type == CredentialType::CREDENTIAL_TYPE_FEDERATED) {
    std::string federation;
    json.GetString(kFederatedCredentialProviderKey, &federation);
    if (!GURL(federation).is_valid()) {
      // |provider| field must be a valid URL. See
      // https://w3c.github.io/webappsec-credential-management/#provider-identification
      return false;
    }
    credential->federation = url::Origin(GURL(federation));
  }
  return true;
}

bool WebStateContentIsSecureHtml(const web::WebState* web_state) {
  if (!web_state) {
    return false;
  }

  if (!web_state->ContentIsHTML()) {
    return false;
  }

  const GURL last_committed_url = web_state->GetLastCommittedURL();

  if (!OriginSecurityChecker::IsContextSecure(last_committed_url)) {
    return false;
  }

  // If scheme is not cryptographic, the origin must be either localhost or a
  // file.
  if (!OriginSecurityChecker::IsSchemeCryptographic(last_committed_url)) {
    return OriginSecurityChecker::IsOriginLocalhostOrFile(last_committed_url);
  }

  // If scheme is cryptographic, valid SSL certificate is required.
  security_state::SecurityLevel security_level =
      GetSecurityLevelForWebState(web_state);
  return OriginSecurityChecker::IsSSLCertificateValid(security_level);
}

}  // namespace credential_manager
