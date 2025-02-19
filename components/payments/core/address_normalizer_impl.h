// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_PAYMENTS_CORE_ADDRESS_NORMALIZER_IMPL_H_
#define COMPONENTS_PAYMENTS_CORE_ADDRESS_NORMALIZER_IMPL_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/macros.h"
#include "components/payments/core/address_normalizer.h"

namespace autofill {
class AutofillProfile;
}

namespace i18n {
namespace addressinput {
class Source;
class Storage;
}
}

namespace payments {

// A class used to normalize addresses.
class AddressNormalizerImpl : public AddressNormalizer {
 public:
  AddressNormalizerImpl(std::unique_ptr<::i18n::addressinput::Source> source,
                        std::unique_ptr<::i18n::addressinput::Storage> storage);
  ~AddressNormalizerImpl() override;

  // AddressNormalizer implementation.
  void LoadRulesForRegion(const std::string& region_code) override;
  bool AreRulesLoadedForRegion(const std::string& region_code) override;
  void StartAddressNormalization(const autofill::AutofillProfile& profile,
                                 const std::string& region_code,
                                 int timeout_seconds,
                                 Delegate* requester) override;

 private:
  // Called when the validation rules for the |region_code| have finished
  // loading. Implementation of the LoadRulesListener interface.
  void OnAddressValidationRulesLoaded(const std::string& region_code,
                                      bool success) override;

  // Map associating a region code to pending normalizations.
  std::map<std::string, std::vector<std::unique_ptr<Request>>>
      pending_normalization_;

  // The address validator used to normalize addresses.
  autofill::AddressValidator address_validator_;

  DISALLOW_COPY_AND_ASSIGN(AddressNormalizerImpl);
};

}  // namespace payments

#endif  // COMPONENTS_PAYMENTS_CORE_ADDRESS_NORMALIZER_IMPL_H_
