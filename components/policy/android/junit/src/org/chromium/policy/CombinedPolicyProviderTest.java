// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.policy;

import static org.junit.Assert.assertEquals;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyLong;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import android.os.Bundle;

import org.chromium.testing.local.LocalRobolectricTestRunner;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.annotation.Config;

/**
 * Robolectric tests for CombinedPolicyProvider
 */
@RunWith(LocalRobolectricTestRunner.class)
@Config(manifest = Config.NONE)
public class CombinedPolicyProviderTest {
    private PolicyConverter mPolicyConverter;

    @Before
    public void setup() {
        // Stub out the native calls
        CombinedPolicyProvider provider = spy(new CombinedPolicyProvider());
        mPolicyConverter = mock(PolicyConverter.class);
        doNothing().when(mPolicyConverter).setPolicy(anyString(), any());
        doNothing().when(provider).nativeFlushPolicies(anyLong());
        CombinedPolicyProvider.set(provider);
    }

    /**
     * Dummy concrete class. Needed because PolicyProvider has final functions that cannot be
     * stubbed and is abstract so can't be directly instantiated to be spied upon.
     */
    class DummyPolicyProvider extends PolicyProvider {
        public DummyPolicyProvider() {}

        @Override
        public void refresh() {
            // Do nothing
        }
    }

    @Test
    public void testRegisterProvider() {
        // Have to spy not mock here so that the real constructor gets called, hence avoiding
        // an assert in PolicyProvider.setManagerAndSource.
        PolicyProvider provider = spy(new DummyPolicyProvider());
        CombinedPolicyProvider.get().registerProvider(provider);
        // Can't verify PolicyProvider.setManagerAndSource directly because it is final.
        // This, at least, demonstrates that it has been called.
        verify(provider).startListeningForPolicyChanges();
        verify(provider, never()).refresh();
        assertEquals(CombinedPolicyProvider.get(),
                CombinedPolicyProvider.linkNative(1234, mPolicyConverter));
        verify(provider).refresh();
        PolicyProvider provider2 = spy(new DummyPolicyProvider());
        CombinedPolicyProvider.get().registerProvider(provider2);
        verify(provider2).startListeningForPolicyChanges();
        verify(provider2).refresh();
    }

    @Test
    public void testOnSettingsAvailable_noNative() {
        // No native policy manager
        PolicyProvider provider = new DummyPolicyProvider();
        CombinedPolicyProvider.get().registerProvider(provider);
        Bundle b = new Bundle();
        b.putBoolean("BoolPolicy", true);
        CombinedPolicyProvider.get().onSettingsAvailable(0, b);
        verify(mPolicyConverter, never()).setPolicy(anyString(), any());
        verify(CombinedPolicyProvider.get(), never()).nativeFlushPolicies(anyInt());
    }

    @Test
    public void testOnSettingsAvailable_oneProvider() {
        CombinedPolicyProvider.linkNative(1234, mPolicyConverter);
        PolicyProvider provider = new DummyPolicyProvider();
        CombinedPolicyProvider.get().registerProvider(provider);
        Bundle b = new Bundle();
        b.putBoolean("BoolPolicy", false);
        b.putInt("IntPolicy", 42);
        b.putString("StringPolicy", "A string");
        b.putStringArray("StringArrayPolicy", new String[] {"String1", "String2"});
        CombinedPolicyProvider.get().onSettingsAvailable(0, b);
        verify(mPolicyConverter).setPolicy("BoolPolicy", false);
        verify(mPolicyConverter).setPolicy("IntPolicy", 42);
        verify(mPolicyConverter).setPolicy("StringPolicy", "A string");
        verify(mPolicyConverter)
                .setPolicy("StringArrayPolicy", new String[] {"String1", "String2"});
        verify(CombinedPolicyProvider.get()).nativeFlushPolicies(1234);
    }

    @Test
    public void testOnSettingsAvailable_secondProvider() {
        CombinedPolicyProvider.linkNative(1234, mPolicyConverter);
        PolicyProvider provider = new DummyPolicyProvider();
        CombinedPolicyProvider.get().registerProvider(provider);
        Bundle b = new Bundle();
        CombinedPolicyProvider.get().onSettingsAvailable(0, b);
        verify(CombinedPolicyProvider.get()).nativeFlushPolicies(1234);

        // Second policy provider registered but no settings.
        PolicyProvider provider2 = new DummyPolicyProvider();
        CombinedPolicyProvider.get().registerProvider(provider2);
        b = new Bundle();
        b.putBoolean("BoolPolicy", true);
        CombinedPolicyProvider.get().onSettingsAvailable(0, b);

        // Second call should have been ignored, so nothing should have been set
        verify(mPolicyConverter, never()).setPolicy(anyString(), anyBoolean());
        // and flush should have been called precisely once.
        verify(CombinedPolicyProvider.get()).nativeFlushPolicies(1234);

        // Empty but valid bundle from second policy provider should set the policy and push it
        // to the native code
        b = new Bundle();
        CombinedPolicyProvider.get().onSettingsAvailable(1, b);
        verify(mPolicyConverter).setPolicy("BoolPolicy", true);
        verify(CombinedPolicyProvider.get(), times(2)).nativeFlushPolicies(1234);
    }

    @Test
    public void testRefreshPolicies() {
        CombinedPolicyProvider.linkNative(1234, mPolicyConverter);
        PolicyProvider provider = new DummyPolicyProvider();
        PolicyProvider provider2 = new DummyPolicyProvider();
        CombinedPolicyProvider.get().registerProvider(provider);
        CombinedPolicyProvider.get().registerProvider(provider2);
        Bundle b = new Bundle();
        b.putBoolean("BoolPolicy", true);
        CombinedPolicyProvider.get().onSettingsAvailable(0, b);
        CombinedPolicyProvider.get().onSettingsAvailable(1, b);
        verify(CombinedPolicyProvider.get(), times(1)).nativeFlushPolicies(1234);

        CombinedPolicyProvider.get().refreshPolicies();
        // This should have cleared the cached policies, so onSettingsAvailable should now do
        // nothing until both providers have settings.
        CombinedPolicyProvider.get().onSettingsAvailable(0, b);
        // Still only one call.
        verify(CombinedPolicyProvider.get(), times(1)).nativeFlushPolicies(1234);
        b = new Bundle();
        b.putBoolean("BoolPolicy", false);
        CombinedPolicyProvider.get().onSettingsAvailable(1, b);
        // That should have caused the second flush.
        verify(CombinedPolicyProvider.get(), times(2)).nativeFlushPolicies(1234);
        // And the policy should have been set to the new value.
        verify(mPolicyConverter).setPolicy("BoolPolicy", false);
    }

    @Test
    public void testTerminateIncognitoSession() {
        CombinedPolicyProvider.PolicyChangeListener l =
                mock(CombinedPolicyProvider.PolicyChangeListener.class);
        CombinedPolicyProvider.get().addPolicyChangeListener(l);
        CombinedPolicyProvider.get().terminateIncognitoSession();
        verify(l).terminateIncognitoSession();
        CombinedPolicyProvider.get().removePolicyChangeListener(l);
        CombinedPolicyProvider.get().terminateIncognitoSession();
        // Should still have only called the listener once
        verify(l).terminateIncognitoSession();
    }

    @Test
    public void testDestroy() {
        PolicyProvider provider = spy(new DummyPolicyProvider());
        CombinedPolicyProvider.get().registerProvider(provider);
        CombinedPolicyProvider.get().destroy();
        verify(provider).destroy();
    }
}
