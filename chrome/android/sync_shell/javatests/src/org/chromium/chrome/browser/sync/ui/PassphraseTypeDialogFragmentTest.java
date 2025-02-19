// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.sync.ui;

import android.support.test.filters.SmallTest;
import android.widget.CheckedTextView;
import android.widget.ListView;

import org.chromium.base.test.util.Feature;
import org.chromium.base.test.util.FlakyTest;
import org.chromium.base.test.util.RetryOnFailure;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.sync.SyncTestBase;
import org.chromium.components.sync.PassphraseType;

/**
 * Tests to make sure that PassphraseTypeDialogFragment presents the correct options.
 */
@RetryOnFailure  // crbug.com/637448
public class PassphraseTypeDialogFragmentTest extends SyncTestBase {
    private static final String TAG = "PassphraseTypeDialogFragmentTest";

    private static final boolean ENABLED = true;
    private static final boolean DISABLED = false;
    private static final boolean CHECKED = true;
    private static final boolean UNCHECKED = false;

    private static class TypeOptions {
        public final PassphraseType type;
        public final boolean isEnabled;
        public final boolean isChecked;
        public TypeOptions(PassphraseType type, boolean isEnabled, boolean isChecked) {
            this.type = type;
            this.isEnabled = isEnabled;
            this.isChecked = isChecked;
        }
    }

    private PassphraseTypeDialogFragment mTypeFragment;

    @SmallTest
    @Feature({"Sync"})
    public void testKeystoreEncryptionOptions() throws Exception {
        createFragment(PassphraseType.KEYSTORE_PASSPHRASE, true);
        assertPassphraseTypeOptions(
                new TypeOptions(PassphraseType.CUSTOM_PASSPHRASE, ENABLED, UNCHECKED),
                new TypeOptions(PassphraseType.KEYSTORE_PASSPHRASE, ENABLED, CHECKED));
    }

    @SmallTest
    @Feature({"Sync"})
    public void testCustomEncryptionOptions() throws Exception {
        createFragment(PassphraseType.CUSTOM_PASSPHRASE, true);
        assertPassphraseTypeOptions(
                new TypeOptions(PassphraseType.CUSTOM_PASSPHRASE, DISABLED, CHECKED),
                new TypeOptions(PassphraseType.KEYSTORE_PASSPHRASE, DISABLED, UNCHECKED));
    }

    /*
     * @SmallTest
     * @Feature({"Sync"})
     */
    @FlakyTest(message = "crbug.com/588050")
    public void testFrozenImplicitEncryptionOptions() throws Exception {
        createFragment(PassphraseType.FROZEN_IMPLICIT_PASSPHRASE, true);
        assertPassphraseTypeOptions(
                new TypeOptions(PassphraseType.FROZEN_IMPLICIT_PASSPHRASE, DISABLED, CHECKED),
                new TypeOptions(PassphraseType.KEYSTORE_PASSPHRASE, DISABLED, UNCHECKED));
    }

    @SmallTest
    @Feature({"Sync"})
    public void testImplicitEncryptionOptions() throws Exception {
        createFragment(PassphraseType.IMPLICIT_PASSPHRASE, true);
        assertPassphraseTypeOptions(
                new TypeOptions(PassphraseType.CUSTOM_PASSPHRASE, ENABLED, UNCHECKED),
                new TypeOptions(PassphraseType.IMPLICIT_PASSPHRASE, ENABLED, CHECKED));
    }

    @SmallTest
    @Feature({"Sync"})
    public void testKeystoreEncryptionOptionsEncryptEverythingDisallowed() throws Exception {
        createFragment(PassphraseType.KEYSTORE_PASSPHRASE, false);
        assertPassphraseTypeOptions(
                new TypeOptions(PassphraseType.CUSTOM_PASSPHRASE, DISABLED, UNCHECKED),
                new TypeOptions(PassphraseType.KEYSTORE_PASSPHRASE, ENABLED, CHECKED));
    }

    @SmallTest
    @Feature({"Sync"})
    public void testImplicitEncryptionOptionsEncryptEverythingDisallowed() throws Exception {
        createFragment(PassphraseType.IMPLICIT_PASSPHRASE, false);
        assertPassphraseTypeOptions(
                new TypeOptions(PassphraseType.CUSTOM_PASSPHRASE, DISABLED, UNCHECKED),
                new TypeOptions(PassphraseType.IMPLICIT_PASSPHRASE, ENABLED, CHECKED));
    }

    public void createFragment(PassphraseType type, boolean isEncryptEverythingAllowed) {
        mTypeFragment = PassphraseTypeDialogFragment.create(type, 0, isEncryptEverythingAllowed);
        mTypeFragment.show(getActivity().getFragmentManager(), TAG);
        getInstrumentation().waitForIdleSync();
    }

    public void assertPassphraseTypeOptions(TypeOptions... optionsList) {
        ListView listView =
                (ListView) mTypeFragment.getDialog().findViewById(R.id.passphrase_type_list);
        assertEquals("Number of options doesn't match.", optionsList.length, listView.getCount());
        PassphraseTypeDialogFragment.Adapter adapter =
                (PassphraseTypeDialogFragment.Adapter) listView.getAdapter();

        for (int i = 0; i < optionsList.length; i++) {
            TypeOptions options = optionsList[i];
            assertEquals("Option " + i + " type is wrong.", options.type, adapter.getType(i));
            CheckedTextView checkedView = (CheckedTextView) listView.getChildAt(i);
            assertEquals("Option " + i + " enabled state is wrong.",
                    options.isEnabled, checkedView.isEnabled());
            assertEquals("Option " + i + " checked state is wrong.",
                    options.isChecked, checkedView.isChecked());
        }
    }
}
