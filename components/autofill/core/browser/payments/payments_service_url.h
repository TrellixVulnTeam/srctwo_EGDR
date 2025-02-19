// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_AUTOFILL_CORE_BROWSER_PAYMENTS_PAYMENTS_SERVICE_URL_H_
#define COMPONENTS_AUTOFILL_CORE_BROWSER_PAYMENTS_PAYMENTS_SERVICE_URL_H_

#include <stddef.h>

class GURL;

namespace autofill {
namespace payments {

// Returns true if production Payments URLs should be used or false if sandbox
// should be used.
bool IsPaymentsProductionEnabled();

// Returns the base URL to use for all Google Payments activity (RPCs and/or
// navigation).
GURL GetBaseSecureUrl();

// Returns the URL to navigate to in order to allow the user to edit or delete
// payment instruments (credit cards) or addresses, respectively. |user_index|
// is the index into the list of signed-in GAIA profiles for which this request
// is being made.
GURL GetManageInstrumentsUrl(size_t user_index);
GURL GetManageAddressesUrl(size_t user_index);

}  // namespace payments
}  // namespace autofill

#endif  // COMPONENTS_AUTOFILL_CORE_BROWSER_PAYMENTS_PAYMENTS_SERVICE_URL_H_
