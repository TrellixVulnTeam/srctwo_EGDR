/*
 * Copyright 2016 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* global PaymentRequest:false */

/**
 * Launches the PaymentRequest UI that requests email address and phone number.
 */
function buy() {  // eslint-disable-line no-unused-vars
  try {
    new PaymentRequest(
        [{supportedMethods: ['https://bobpay.com', 'visa']}],
        {total: {label: 'Total', amount: {currency: 'USD', value: '5.00'}}},
        {requestPayerEmail: true, requestPayerPhone: true})
        .show()
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
