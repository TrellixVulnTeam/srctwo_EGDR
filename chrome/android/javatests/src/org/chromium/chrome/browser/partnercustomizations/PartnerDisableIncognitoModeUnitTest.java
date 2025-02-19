// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.partnercustomizations;

import static org.chromium.chrome.browser.partnercustomizations.BasePartnerBrowserCustomizationUnitTestRule.DEFAULT_TIMEOUT_MS;
import static org.chromium.chrome.browser.partnercustomizations.BasePartnerBrowserCustomizationUnitTestRule.PARTNER_BROWSER_CUSTOMIZATIONS_DELAYED_PROVIDER;
import static org.chromium.chrome.browser.partnercustomizations.BasePartnerBrowserCustomizationUnitTestRule.PARTNER_BROWSER_CUSTOMIZATIONS_NO_PROVIDER;
import static org.chromium.chrome.browser.partnercustomizations.BasePartnerBrowserCustomizationUnitTestRule.PARTNER_BROWSER_CUSTOMIZATIONS_PROVIDER;

import android.net.Uri;
import android.os.Bundle;
import android.support.test.filters.SmallTest;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.chromium.base.ThreadUtils;
import org.chromium.base.test.BaseJUnit4ClassRunner;
import org.chromium.base.test.util.Feature;
import org.chromium.chrome.test.partnercustomizations.TestPartnerBrowserCustomizationsProvider;

/**
 * Unit tests for the partner disabling incognito mode functionality.
 */
@RunWith(BaseJUnit4ClassRunner.class)
public class PartnerDisableIncognitoModeUnitTest {
    @Rule
    public BasePartnerBrowserCustomizationUnitTestRule mTestRule =
            new BasePartnerBrowserCustomizationUnitTestRule();

    private void setParentalControlsEnabled(boolean enabled) {
        Uri uri = PartnerBrowserCustomizations.buildQueryUri(
                PartnerBrowserCustomizations.PARTNER_DISABLE_INCOGNITO_MODE_PATH);
        Bundle bundle = new Bundle();
        bundle.putBoolean(
                TestPartnerBrowserCustomizationsProvider.INCOGNITO_MODE_DISABLED_KEY, enabled);
        mTestRule.getContextWrapper().getContentResolver().call(
                uri, "setIncognitoModeDisabled", null, bundle);
    }

    @Test
    @SmallTest
    @Feature({"ParentalControls"})
    public void testProviderNotFromSystemPackage() throws InterruptedException {
        // Note that unlike other tests in this file, we do not call
        // PartnerBrowserCustomizations.ignoreBrowserProviderSystemPackageCheckForTests(true);
        // here to test if Chrome ignores a customizations provider that is not from
        // a system package.
        PartnerBrowserCustomizations.setProviderAuthorityForTests(
                PARTNER_BROWSER_CUSTOMIZATIONS_PROVIDER);
        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                PartnerBrowserCustomizations.initializeAsync(
                        mTestRule.getContextWrapper(), DEFAULT_TIMEOUT_MS);
            }
        });
        PartnerBrowserCustomizations.setOnInitializeAsyncFinished(
                mTestRule.getCallback(), DEFAULT_TIMEOUT_MS);

        mTestRule.getCallbackLock().acquire();

        Assert.assertTrue(PartnerBrowserCustomizations.isInitialized());
        Assert.assertFalse(PartnerBrowserCustomizations.isIncognitoDisabled());
    }

    @Test
    @SmallTest
    @Feature({"ParentalControls"})
    public void testNoProvider() throws InterruptedException {
        PartnerBrowserCustomizations.ignoreBrowserProviderSystemPackageCheckForTests(true);
        PartnerBrowserCustomizations.setProviderAuthorityForTests(
                PARTNER_BROWSER_CUSTOMIZATIONS_NO_PROVIDER);
        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                PartnerBrowserCustomizations.initializeAsync(
                        mTestRule.getContextWrapper(), DEFAULT_TIMEOUT_MS);
            }
        });
        PartnerBrowserCustomizations.setOnInitializeAsyncFinished(
                mTestRule.getCallback(), DEFAULT_TIMEOUT_MS);
        mTestRule.getCallbackLock().acquire();

        Assert.assertTrue(PartnerBrowserCustomizations.isInitialized());
        Assert.assertFalse(PartnerBrowserCustomizations.isIncognitoDisabled());
    }

    @Test
    @SmallTest
    @Feature({"ParentalControls"})
    public void testParentalControlsNotEnabled() throws InterruptedException {
        PartnerBrowserCustomizations.ignoreBrowserProviderSystemPackageCheckForTests(true);
        PartnerBrowserCustomizations.setProviderAuthorityForTests(
                PARTNER_BROWSER_CUSTOMIZATIONS_PROVIDER);
        setParentalControlsEnabled(false);
        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                PartnerBrowserCustomizations.initializeAsync(
                        mTestRule.getContextWrapper(), DEFAULT_TIMEOUT_MS);
            }
        });
        PartnerBrowserCustomizations.setOnInitializeAsyncFinished(
                mTestRule.getCallback(), DEFAULT_TIMEOUT_MS);

        mTestRule.getCallbackLock().acquire();

        Assert.assertTrue(PartnerBrowserCustomizations.isInitialized());
        Assert.assertFalse(PartnerBrowserCustomizations.isIncognitoDisabled());
    }

    @Test
    @SmallTest
    @Feature({"ParentalControls"})
    public void testParentalControlsEnabled() throws InterruptedException {
        PartnerBrowserCustomizations.ignoreBrowserProviderSystemPackageCheckForTests(true);
        PartnerBrowserCustomizations.setProviderAuthorityForTests(
                PARTNER_BROWSER_CUSTOMIZATIONS_PROVIDER);
        setParentalControlsEnabled(true);
        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                PartnerBrowserCustomizations.initializeAsync(
                        mTestRule.getContextWrapper(), DEFAULT_TIMEOUT_MS);
            }
        });
        PartnerBrowserCustomizations.setOnInitializeAsyncFinished(
                mTestRule.getCallback(), DEFAULT_TIMEOUT_MS);

        mTestRule.getCallbackLock().acquire();

        Assert.assertTrue(PartnerBrowserCustomizations.isInitialized());
        Assert.assertTrue(PartnerBrowserCustomizations.isIncognitoDisabled());
    }

    @Test
    @SmallTest
    @Feature({"ParentalControls"})
    public void testParentalControlsProviderDelayed() throws InterruptedException {
        PartnerBrowserCustomizations.ignoreBrowserProviderSystemPackageCheckForTests(true);
        PartnerBrowserCustomizations.setProviderAuthorityForTests(
                PARTNER_BROWSER_CUSTOMIZATIONS_DELAYED_PROVIDER);
        mTestRule.setDelayProviderUriPathForDelay(
                PartnerBrowserCustomizations.PARTNER_DISABLE_INCOGNITO_MODE_PATH);
        setParentalControlsEnabled(true);
        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                PartnerBrowserCustomizations.initializeAsync(mTestRule.getContextWrapper(), 2000);
            }
        });
        PartnerBrowserCustomizations.setOnInitializeAsyncFinished(mTestRule.getCallback(), 300);

        mTestRule.getCallbackLock().acquire();

        Assert.assertFalse(PartnerBrowserCustomizations.isInitialized());
        Assert.assertFalse(PartnerBrowserCustomizations.isIncognitoDisabled());

        PartnerBrowserCustomizations.setOnInitializeAsyncFinished(mTestRule.getCallback(), 3000);

        mTestRule.getCallbackLock().acquire();

        Assert.assertTrue(PartnerBrowserCustomizations.isInitialized());
        Assert.assertTrue(PartnerBrowserCustomizations.isIncognitoDisabled());
    }
}
