// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/signin/core/browser/signin_investigator.h"

#include "base/logging.h"
#include "base/metrics/histogram_macros.h"
#include "components/signin/core/browser/signin_metrics.h"
#include "components/signin/core/common/signin_pref_names.h"
#include "google_apis/gaia/gaia_auth_util.h"

using signin_metrics::AccountEquality;
using signin_metrics::LogAccountEquality;

namespace {
void LogSigninScenario(InvestigatedScenario scenario) {
  UMA_HISTOGRAM_ENUMERATION(
      "Signin.InvestigatedScenario", static_cast<int>(scenario),
      static_cast<int>(InvestigatedScenario::HISTOGRAM_COUNT));
}
}  // namespace

SigninInvestigator::SigninInvestigator(const std::string& current_email,
                                       const std::string& current_id,
                                       DependencyProvider* provider)
    : current_email_(current_email),
      current_id_(current_id),
      provider_(provider) {
  DCHECK(!current_email_.empty());
  DCHECK(provider);
}

SigninInvestigator::~SigninInvestigator() {}

bool SigninInvestigator::AreAccountsEqualWithFallback() {
  const std::string last_id =
      provider_->GetPrefs()->GetString(prefs::kGoogleServicesLastAccountId);
  bool same_email = gaia::AreEmailsSame(
      current_email_,
      provider_->GetPrefs()->GetString(prefs::kGoogleServicesLastUsername));
  if (!current_id_.empty() && !last_id.empty()) {
    bool same_id = current_id_ == last_id;
    if (same_email && same_id) {
      LogAccountEquality(AccountEquality::BOTH_EQUAL);
    } else if (same_email) {
      LogAccountEquality(AccountEquality::ONLY_SAME_EMAIL);
    } else if (same_id) {
      LogAccountEquality(AccountEquality::ONLY_SAME_ID);
    } else {
      LogAccountEquality(AccountEquality::BOTH_DIFFERENT);
    }
    return same_id;
  } else {
    LogAccountEquality(AccountEquality::EMAIL_FALLBACK);
    return same_email;
  }
}

InvestigatedScenario SigninInvestigator::Investigate() {
  InvestigatedScenario scenario;
  if (provider_->GetPrefs()
          ->GetString(prefs::kGoogleServicesLastUsername)
          .empty()) {
    scenario = IsUpgradeHighRisk() ? InvestigatedScenario::UPGRADE_HIGH_RISK
                                   : InvestigatedScenario::UPGRADE_LOW_RISK;
  } else if (AreAccountsEqualWithFallback()) {
    scenario = InvestigatedScenario::SAME_ACCOUNT;
  } else {
    scenario = InvestigatedScenario::DIFFERENT_ACCOUNT;
  }

  LogSigninScenario(scenario);
  return scenario;
}

bool SigninInvestigator::IsUpgradeHighRisk() {
  // TODO(skym): Add logic to make this decision, crbug.com/572754.
  return false;
}
