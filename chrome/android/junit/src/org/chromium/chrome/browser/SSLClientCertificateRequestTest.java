// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser;

import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyZeroInteractions;

import android.content.ActivityNotFoundException;
import android.security.KeyChainAliasCallback;

import org.chromium.chrome.browser.SSLClientCertificateRequest.CertSelectionFailureDialog;
import org.chromium.chrome.browser.SSLClientCertificateRequest.KeyChainCertSelectionWrapper;
import org.chromium.testing.local.LocalRobolectricTestRunner;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import org.robolectric.annotation.Config;

/**
 * Unit tests for the SSLClientCertificateRequest class.
 */
@RunWith(LocalRobolectricTestRunner.class)
@Config(manifest = Config.NONE)
public class SSLClientCertificateRequestTest {
    @Mock private KeyChainCertSelectionWrapper mKeyChainMock;
    @Mock private KeyChainAliasCallback mCallbackMock;
    @Mock private CertSelectionFailureDialog mFailureDialogMock;

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);
    }

    @Test
    public void testSelectCertActivityNotFound() {
        doThrow(new ActivityNotFoundException()).when(mKeyChainMock).choosePrivateKeyAlias();

        SSLClientCertificateRequest.maybeShowCertSelection(mKeyChainMock, mCallbackMock,
                mFailureDialogMock);

        verify(mKeyChainMock).choosePrivateKeyAlias();
        verify(mCallbackMock).alias(null);
        verify(mFailureDialogMock).show();
    }

    @Test
    public void testSelectCertActivityFound() {
        SSLClientCertificateRequest.maybeShowCertSelection(mKeyChainMock, mCallbackMock,
                mFailureDialogMock);

        verify(mKeyChainMock).choosePrivateKeyAlias();
        verifyZeroInteractions(mFailureDialogMock);
    }
}
