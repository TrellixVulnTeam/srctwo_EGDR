// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.incognito;

import android.app.PendingIntent;
import android.app.PendingIntent.CanceledException;
import android.content.Context;
import android.support.test.filters.MediumTest;
import android.util.Pair;

import org.chromium.base.ThreadUtils;
import org.chromium.base.library_loader.LibraryLoader;
import org.chromium.base.test.util.Feature;
import org.chromium.base.test.util.RetryOnFailure;
import org.chromium.chrome.browser.TabState;
import org.chromium.chrome.browser.profiles.Profile;
import org.chromium.chrome.browser.tabmodel.TabModel.TabLaunchType;
import org.chromium.chrome.browser.tabmodel.TabPersistentStore;
import org.chromium.chrome.browser.tabmodel.TestTabModelDirectory;
import org.chromium.chrome.browser.tabmodel.TestTabModelDirectory.TabStateInfo;
import org.chromium.chrome.test.ChromeTabbedActivityTestBase;
import org.chromium.content.browser.test.util.Criteria;
import org.chromium.content.browser.test.util.CriteriaHelper;
import org.chromium.content_public.browser.LoadUrlParams;

import java.io.File;
import java.util.concurrent.Callable;

/**
 * Tests for the Incognito Notification service.
 */
public class IncognitoNotificationServiceTest extends ChromeTabbedActivityTestBase {

    @Override
    public void startMainActivity() throws InterruptedException {
        // Each test will start its own activity as needed.
    }

    private void createTabOnUiThread() {
        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                getActivity().getTabCreator(true).createNewTab(new LoadUrlParams("about:blank"),
                        TabLaunchType.FROM_CHROME_UI, null);
            }
        });
    }

    private void sendClearIncognitoIntent() throws CanceledException {
        PendingIntent clearIntent =
                IncognitoNotificationService.getRemoveAllIncognitoTabsIntent(
                        getInstrumentation().getTargetContext());
        clearIntent.send();

    }

    @Feature("Incognito")
    @MediumTest
    public void testSingleRunningChromeTabbedActivity()
            throws InterruptedException, CanceledException {
        startMainActivityOnBlankPage();

        createTabOnUiThread();
        createTabOnUiThread();

        CriteriaHelper.pollUiThread(Criteria.equals(2, new Callable<Integer>() {
            @Override
            public Integer call() throws Exception {
                return getActivity().getTabModelSelector().getModel(true).getCount();
            }
        }));

        final Profile incognitoProfile = ThreadUtils.runOnUiThreadBlockingNoException(
                new Callable<Profile>() {
                    @Override
                    public Profile call() throws Exception {
                        return getActivity().getTabModelSelector().getModel(true).getProfile();
                    }
                });
        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                assertTrue(incognitoProfile.isOffTheRecord());
                assertTrue(incognitoProfile.isNativeInitialized());
            }
        });

        sendClearIncognitoIntent();

        CriteriaHelper.pollUiThread(Criteria.equals(0, new Callable<Integer>() {
            @Override
            public Integer call() throws Exception {
                return getActivity().getTabModelSelector().getModel(true).getCount();
            }
        }));
        CriteriaHelper.pollUiThread(new Criteria() {
            @Override
            public boolean isSatisfied() {
                return !incognitoProfile.isNativeInitialized();
            }
        });
    }

    @Feature("Incognito")
    @MediumTest
    @RetryOnFailure
    public void testNoAliveProcess() throws Exception {
        Context context = getInstrumentation().getTargetContext();
        final TestTabModelDirectory tabbedModeDirectory = new TestTabModelDirectory(
                context, "tabs", String.valueOf(0));

        // Add a couple non-incognito tabs (their filenames use a different prefix, so we do not
        // need to worry about ID space collisions with the generated incognito tabs).
        tabbedModeDirectory.writeTabStateFile(TestTabModelDirectory.V2_DUCK_DUCK_GO);
        tabbedModeDirectory.writeTabStateFile(TestTabModelDirectory.V2_BAIDU);

        // Generate a few incognito tabs (using arbitrary data from an existing TabState
        // definition).
        for (int i = 0; i < 3; i++) {
            TabStateInfo incognitoInfo = new TabStateInfo(
                    true,
                    TestTabModelDirectory.V2_TEXTAREA.version,
                    i,
                    TestTabModelDirectory.V2_TEXTAREA.url,
                    TestTabModelDirectory.V2_TEXTAREA.title,
                    TestTabModelDirectory.V2_TEXTAREA.encodedTabState);
            tabbedModeDirectory.writeTabStateFile(incognitoInfo);
        }

        TabPersistentStore.setBaseStateDirectoryForTests(tabbedModeDirectory.getBaseDirectory());

        File[] tabbedModeFiles = tabbedModeDirectory.getDataDirectory().listFiles();
        assertNotNull(tabbedModeFiles);
        assertEquals(5, tabbedModeFiles.length);

        int incognitoCount = 0;
        int normalCount = 0;
        for (File tabbedModeFile : tabbedModeFiles) {
            Pair<Integer, Boolean> tabFileInfo =
                    TabState.parseInfoFromFilename(tabbedModeFile.getName());
            if (tabFileInfo.second) incognitoCount++;
            else normalCount++;
        }
        assertEquals(2, normalCount);
        assertEquals(3, incognitoCount);

        sendClearIncognitoIntent();

        CriteriaHelper.pollInstrumentationThread(Criteria.equals(0, new Callable<Integer>() {
            @Override
            public Integer call() throws Exception {
                File[] tabbedModeFiles = tabbedModeDirectory.getDataDirectory().listFiles();
                if (tabbedModeFiles == null) return 0;
                int incognitoCount = 0;
                for (File tabbedModeFile : tabbedModeFiles) {
                    Pair<Integer, Boolean> tabFileInfo =
                            TabState.parseInfoFromFilename(tabbedModeFile.getName());
                    if (tabFileInfo != null && tabFileInfo.second) incognitoCount++;
                }
                return incognitoCount;
            }
        }));

        CriteriaHelper.pollInstrumentationThread(Criteria.equals(2, new Callable<Integer>() {
            @Override
            public Integer call() throws Exception {
                File[] tabbedModeFiles = tabbedModeDirectory.getDataDirectory().listFiles();
                if (tabbedModeFiles == null) return 0;
                int normalCount = 0;
                for (File tabbedModeFile : tabbedModeFiles) {
                    Pair<Integer, Boolean> tabFileInfo =
                            TabState.parseInfoFromFilename(tabbedModeFile.getName());
                    if (tabFileInfo != null && !tabFileInfo.second) normalCount++;
                }
                return normalCount;
            }
        }));

        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                assertFalse(LibraryLoader.isInitialized());
            }
        });
    }

}
