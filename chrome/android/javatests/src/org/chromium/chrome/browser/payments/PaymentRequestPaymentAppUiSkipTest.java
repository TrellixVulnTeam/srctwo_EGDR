// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.payments;

import static org.chromium.chrome.browser.payments.PaymentRequestTestRule.DELAYED_CREATION;
import static org.chromium.chrome.browser.payments.PaymentRequestTestRule.DELAYED_RESPONSE;
import static org.chromium.chrome.browser.payments.PaymentRequestTestRule.HAVE_INSTRUMENTS;
import static org.chromium.chrome.browser.payments.PaymentRequestTestRule.IMMEDIATE_RESPONSE;

import android.support.test.filters.MediumTest;

import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.chromium.base.test.util.CommandLineFlags;
import org.chromium.base.test.util.Feature;
import org.chromium.chrome.browser.ChromeSwitches;
import org.chromium.chrome.test.ChromeActivityTestRule;
import org.chromium.chrome.test.ChromeJUnit4ClassRunner;

import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeoutException;

/**
 * A payment integration test for a merchant that requests payment via Bob Pay when performing the
 * single payment app UI skip optimization.
 */
@RunWith(ChromeJUnit4ClassRunner.class)
@CommandLineFlags.Add({
        ChromeSwitches.DISABLE_FIRST_RUN_EXPERIENCE,
        ChromeActivityTestRule.DISABLE_NETWORK_PREDICTION_FLAG,
})
public class PaymentRequestPaymentAppUiSkipTest {
    @Rule
    public PaymentRequestTestRule mPaymentRequestTestRule =
            new PaymentRequestTestRule("payment_request_bobpay_ui_skip_test.html");

    /**
     * If Bob Pay is supported and installed, user should be able to pay with it. Here Bob Pay
     * responds to Chrome immediately.
     */
    @Test
    @MediumTest
    @Feature({"Payments"})
    public void testPayViaFastBobPay()
            throws InterruptedException, ExecutionException, TimeoutException {
        mPaymentRequestTestRule.installPaymentApp(HAVE_INSTRUMENTS, IMMEDIATE_RESPONSE);
        mPaymentRequestTestRule.openPageAndClickBuyAndWait(mPaymentRequestTestRule.getDismissed());
        mPaymentRequestTestRule.expectResultContains(
                new String[] {"https://bobpay.com", "\"transaction\"", "1337"});
    }

    /**
     * If Bob Pay is supported and installed, user should be able to pay with it. Here Bob Pay
     * responds to Chrome after a slight delay.
     */
    @Test
    @MediumTest
    @Feature({"Payments"})
    public void testPayViaSlowBobPay()
            throws InterruptedException, ExecutionException, TimeoutException {
        mPaymentRequestTestRule.installPaymentApp(HAVE_INSTRUMENTS, DELAYED_RESPONSE);
        mPaymentRequestTestRule.openPageAndClickBuyAndWait(mPaymentRequestTestRule.getDismissed());
        mPaymentRequestTestRule.expectResultContains(
                new String[] {"https://bobpay.com", "\"transaction\"", "1337"});
    }

    /**
     * Test payment with a Bob Pay that is created with a delay, but responds immediately
     * to getInstruments.
     */
    @Test
    @MediumTest
    @Feature({"Payments"})
    public void testPayViaDelayedFastBobPay()
            throws InterruptedException, ExecutionException, TimeoutException {
        mPaymentRequestTestRule.installPaymentApp(
                "https://bobpay.com", HAVE_INSTRUMENTS, IMMEDIATE_RESPONSE, DELAYED_CREATION);
        mPaymentRequestTestRule.openPageAndClickBuyAndWait(mPaymentRequestTestRule.getDismissed());
        mPaymentRequestTestRule.expectResultContains(
                new String[] {"https://bobpay.com", "\"transaction\"", "1337"});
    }

    /**
     * Test payment with a Bob Pay that is created with a delay, and responds slowly to
     * getInstruments.
     */
    @Test
    @MediumTest
    @Feature({"Payments"})
    public void testPayViaDelayedSlowBobPay()
            throws InterruptedException, ExecutionException, TimeoutException {
        mPaymentRequestTestRule.installPaymentApp(
                "https://bobpay.com", HAVE_INSTRUMENTS, DELAYED_RESPONSE, DELAYED_CREATION);
        mPaymentRequestTestRule.openPageAndClickBuyAndWait(mPaymentRequestTestRule.getDismissed());
        mPaymentRequestTestRule.expectResultContains(
                new String[] {"https://bobpay.com", "\"transaction\"", "1337"});
    }
}
