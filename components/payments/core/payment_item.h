// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_PAYMENTS_CORE_PAYMENT_ITEM_H_
#define COMPONENTS_PAYMENTS_CORE_PAYMENT_ITEM_H_

#include <memory>
#include <string>

#include "components/payments/core/payment_currency_amount.h"

// C++ bindings for the PaymentRequest API PaymentItem. Conforms to the
// following spec:
// https://w3c.github.io/payment-request/#dom-paymentitem

namespace base {
class DictionaryValue;
}

namespace payments {

// Information indicating what the payment request is for and the value asked
// for.
class PaymentItem {
 public:
  PaymentItem();
  ~PaymentItem();

  bool operator==(const PaymentItem& other) const;
  bool operator!=(const PaymentItem& other) const;

  // Populates the properties of this PaymentItem from |value|. Returns true if
  // the required values are present.
  bool FromDictionaryValue(const base::DictionaryValue& value);

  // Creates a base::DictionaryValue with the properties of this
  // PaymentItem.
  std::unique_ptr<base::DictionaryValue> ToDictionaryValue() const;

  // A human-readable description of the item.
  std::string label;

  // The monetary amount for the item.
  PaymentCurrencyAmount amount;

  // When set to true this flag means that the amount field is not final. This
  // is commonly used to show items such as shipping or tax amounts that depend
  // upon selection of shipping address or shipping option.
  bool pending;
};

}  // namespace payments

#endif  // COMPONENTS_PAYMENTS_CORE_PAYMENT_ITEM_H_
