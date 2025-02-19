/*
 * Copyright 2016 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* global PaymentRequest:false */

/**
 * Launches the PaymentRequest UI that offers free shipping in California and
 * $5.00 shipping in US. Does not allow shipping outside of US.
 */
function buy() {  // eslint-disable-line no-unused-vars
  try {
    var details = {
      total: {label: 'Total', amount: {currency: 'USD', value: '5.00'}},
      displayItems: [
        {
          label: 'Pending shipping price',
          amount: {currency: 'USD', value: '0.00'},
          pending: true,
        },
        {label: 'Subtotal', amount: {currency: 'USD', value: '5.00'}},
      ],
    };

    var request = new PaymentRequest(
        [{supportedMethods: ['visa']}], details, {requestShipping: true});

    request.addEventListener('shippingaddresschange', function(evt) {
      evt.updateWith(new Promise(function(resolve) {
        resolve(updateDetails(details, request.shippingAddress));
      }));
    });

    request.show()
        .then(function(resp) {
          resp.complete('success')
              .then(function() {
                print(JSON.stringify(resp, undefined, 2));
              })
              .catch(function(error) {
                print(error);
              });
        })
        .catch(function(error) {
          print(error);
        });
  } catch (error) {
    print(error.message);
  }
}

/**
 * Updates the shopping cart with the appropriate shipping prices according to
 * the shipping address.
 * @param {object} details - The shopping cart.
 * @param {ShippingAddress} addr - The shipping address.
 * @return {object} The updated shopping cart.
 */
function updateDetails(details, addr) {
  if (addr.country === 'US') {
    var shippingOption = {
      id: '',
      label: '',
      amount: {currency: 'USD', value: '0.00'},
      selected: true,
    };
    if (addr.region === 'CA') {
      shippingOption.id = 'californiaShippingOption';
      shippingOption.label = 'Free shipping in California';
      details.total.amount.value = '5.00';
    } else {
      shippingOption.id = 'usShippingOption';
      shippingOption.label = 'Standard shipping in US';
      shippingOption.amount.value = '5.00';
      details.total.amount.value = '10.00';
    }
    details.displayItems.splice(0, 1, shippingOption);
    details.shippingOptions = [shippingOption];
  } else {
    delete details.shippingOptions;
    details.error = 'We do not ship to this address';
  }
  return details;
}
