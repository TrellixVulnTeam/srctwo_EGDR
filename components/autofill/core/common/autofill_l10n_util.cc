// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/core/common/autofill_l10n_util.h"

#include <string>
#include <utility>

#include "base/i18n/string_compare.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/metrics/histogram_macros.h"

namespace autofill {
namespace l10n {

std::unique_ptr<icu::Collator> GetCollatorForLocale(const icu::Locale& locale) {
  UErrorCode ignored = U_ZERO_ERROR;
  std::unique_ptr<icu::Collator> collator(
      icu::Collator::createInstance(locale, ignored));
  if (!collator) {
    // On some systems, the default locale is invalid to the eyes of the ICU
    // library. This could be due to a device-specific issue (has been seen in
    // the wild on Android and iOS devices). In the failure case, |collator|
    // will be null. See http://crbug.com/558625.
    icu::UnicodeString name;
    std::string locale_name;
    locale.getDisplayName(name).toUTF8String(locale_name);
    LOG(ERROR) << "Failed to initialize the ICU Collator with locale "
               << locale_name;

    // Attempt to load the English locale.
    collator = base::WrapUnique(
        icu::Collator::createInstance(icu::Locale::getEnglish(), ignored));
    if (!collator) {
      LOG(ERROR) << "Failed to initialize the ICU Collator with the English "
                 << "locale.";
    }
  }

  UMA_HISTOGRAM_BOOLEAN("Autofill.IcuCollatorCreationSuccess", !!collator);
  return collator;
}

CaseInsensitiveCompare::CaseInsensitiveCompare()
    : CaseInsensitiveCompare(icu::Locale::getDefault()) {}

CaseInsensitiveCompare::CaseInsensitiveCompare(const icu::Locale& locale)
    : collator_(GetCollatorForLocale(locale)) {
  if (collator_)
    collator_->setStrength(icu::Collator::PRIMARY);
}

CaseInsensitiveCompare::~CaseInsensitiveCompare() {
}

bool CaseInsensitiveCompare::StringsEqual(const base::string16& lhs,
                                          const base::string16& rhs) const {
  if (collator_) {
    return base::i18n::CompareString16WithCollator(*collator_, lhs, rhs) ==
           UCOL_EQUAL;
  }
  return lhs == rhs;
}

}  // namespace l10n
}  // namespace autofill
