/*
 * Copyright 2016 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* global PaymentRequest:false */

/**
 * Launches the PaymentRequest UI with Bob Pay and credit cards as payment
 * methods.
 */
function buy() {  // eslint-disable-line no-unused-vars
  try {
    new PaymentRequest(
        [{supportedMethods: ['https://bobpay.com', 'visa', 'mastercard']}],
        {total: {label: 'Total', amount: {currency: 'USD', value: '5.00'}}})
        .show()
        .then(function(resp) {
          resp.complete('success')
              .then(function() {
                print(
                    resp.methodName + '<br>' +
                    JSON.stringify(resp.details, undefined, 2));
              })
              .catch(function(error) {
                print(error.message);
              });
        })
        .catch(function(error) {
          print(error.message);
        });
  } catch (error) {
    print(error.message);
  }
}
