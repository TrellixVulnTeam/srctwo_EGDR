// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/payments/core/payment_shipping_option.h"

#include "base/values.h"

namespace payments {

namespace {

// These are defined as part of the spec at:
// https://w3c.github.io/browser-payment-api/#dom-paymentshippingoption
static const char kPaymentShippingOptionAmount[] = "amount";
static const char kPaymentShippingOptionId[] = "id";
static const char kPaymentShippingOptionLabel[] = "label";
static const char kPaymentShippingOptionSelected[] = "selected";

}  // namespace

PaymentShippingOption::PaymentShippingOption() : selected(false) {}
PaymentShippingOption::PaymentShippingOption(
    const PaymentShippingOption& other) = default;
PaymentShippingOption::~PaymentShippingOption() = default;

bool PaymentShippingOption::operator==(
    const PaymentShippingOption& other) const {
  return this->id == other.id && this->label == other.label &&
         this->amount == other.amount && this->selected == other.selected;
}

bool PaymentShippingOption::operator!=(
    const PaymentShippingOption& other) const {
  return !(*this == other);
}

bool PaymentShippingOption::FromDictionaryValue(
    const base::DictionaryValue& value) {
  if (!value.GetString(kPaymentShippingOptionId, &this->id)) {
    return false;
  }

  if (!value.GetString(kPaymentShippingOptionLabel, &this->label)) {
    return false;
  }

  const base::DictionaryValue* amount_dict = nullptr;
  if (!value.GetDictionary(kPaymentShippingOptionAmount, &amount_dict)) {
    return false;
  }
  if (!this->amount.FromDictionaryValue(*amount_dict)) {
    return false;
  }

  // Selected is optional.
  value.GetBoolean(kPaymentShippingOptionSelected, &this->selected);

  return true;
}

}  // namespace payments
