// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.media.ui;

import static org.chromium.base.test.util.Restriction.RESTRICTION_TYPE_NON_LOW_END_DEVICE;

import android.content.Context;
import android.media.AudioManager;
import android.support.test.InstrumentationRegistry;
import android.support.test.filters.SmallTest;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.chromium.base.test.util.CommandLineFlags;
import org.chromium.base.test.util.Restriction;
import org.chromium.base.test.util.RetryOnFailure;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.ChromeActivity;
import org.chromium.chrome.browser.ChromeSwitches;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.chrome.test.ChromeActivityTestRule;
import org.chromium.chrome.test.ChromeJUnit4ClassRunner;
import org.chromium.content.browser.test.util.DOMUtils;
import org.chromium.content.browser.test.util.JavaScriptUtils;
import org.chromium.net.test.EmbeddedTestServer;

/**
 * Integration test that checks that autoplay muted doesn't show a notification nor take audio focus
 */
@RunWith(ChromeJUnit4ClassRunner.class)
@RetryOnFailure
@CommandLineFlags.Add({ChromeSwitches.DISABLE_FIRST_RUN_EXPERIENCE,
        ChromeActivityTestRule.DISABLE_NETWORK_PREDICTION_FLAG})
public class AutoplayMutedNotificationTest {
    @Rule
    public ChromeActivityTestRule<ChromeActivity> mActivityTestRule =
            new ChromeActivityTestRule<>(ChromeActivity.class);

    private static final String TEST_PATH = "/content/test/data/media/session/autoplay-muted.html";
    private static final String VIDEO_ID = "video";
    private static final int AUDIO_FOCUS_CHANGE_TIMEOUT = 500;  // ms

    private EmbeddedTestServer mTestServer;

    private AudioManager getAudioManager() {
        return (AudioManager) mActivityTestRule.getActivity()
                .getApplicationContext()
                .getSystemService(Context.AUDIO_SERVICE);
    }

    private boolean isMediaNotificationVisible() {
        return MediaNotificationManager.hasManagerForTesting(R.id.media_playback_notification);
    }

    private class MockAudioFocusChangeListener implements AudioManager.OnAudioFocusChangeListener {
        private int mAudioFocusState = AudioManager.AUDIOFOCUS_LOSS;

        @Override
        public void onAudioFocusChange(int focusChange) {
            mAudioFocusState = focusChange;
        }

        public int getAudioFocusState() {
            return mAudioFocusState;
        }

        public void requestAudioFocus(int focusType) throws Exception {
            int result = getAudioManager().requestAudioFocus(
                    this, AudioManager.STREAM_MUSIC, focusType);
            if (result != AudioManager.AUDIOFOCUS_REQUEST_GRANTED) {
                Assert.fail("Did not get audio focus");
            } else {
                mAudioFocusState = focusType;
            }
        }
    }

    private MockAudioFocusChangeListener mAudioFocusChangeListener;

    @Before
    public void setUp() throws Exception {
        mTestServer = EmbeddedTestServer.createAndStartServer(
                InstrumentationRegistry.getInstrumentation().getContext());
        mAudioFocusChangeListener = new MockAudioFocusChangeListener();
        mActivityTestRule.startMainActivityWithURL(mTestServer.getURL(TEST_PATH));
    }

    @After
    public void tearDown() throws Exception {
        mTestServer.stopAndDestroyServer();
    }

    @Test
    @SmallTest
    @Restriction(RESTRICTION_TYPE_NON_LOW_END_DEVICE)
    public void testBasic() throws Exception {
        Tab tab = mActivityTestRule.getActivity().getActivityTab();

        // Taking audio focus.
        Assert.assertEquals(
                AudioManager.AUDIOFOCUS_LOSS, mAudioFocusChangeListener.getAudioFocusState());
        mAudioFocusChangeListener.requestAudioFocus(AudioManager.AUDIOFOCUS_GAIN);
        Assert.assertEquals(
                AudioManager.AUDIOFOCUS_GAIN, mAudioFocusChangeListener.getAudioFocusState());

        // The page will autoplay the video.
        DOMUtils.waitForMediaPlay(tab.getWebContents(), VIDEO_ID);

        // Audio focus notification is OS-driven.
        Thread.sleep(AUDIO_FOCUS_CHANGE_TIMEOUT);

        // Audio focus was not taken and no notification is visible.
        Assert.assertEquals(
                AudioManager.AUDIOFOCUS_GAIN, mAudioFocusChangeListener.getAudioFocusState());
        Assert.assertFalse(isMediaNotificationVisible());
    }

    @Test
    @SmallTest
    @Restriction(RESTRICTION_TYPE_NON_LOW_END_DEVICE)
    public void testDoesNotReactToAudioFocus() throws Exception {
        Tab tab = mActivityTestRule.getActivity().getActivityTab();

        // The page will autoplay the video.
        DOMUtils.waitForMediaPlay(tab.getWebContents(), VIDEO_ID);

        // Taking audio focus.
        Assert.assertEquals(
                AudioManager.AUDIOFOCUS_LOSS, mAudioFocusChangeListener.getAudioFocusState());
        mAudioFocusChangeListener.requestAudioFocus(AudioManager.AUDIOFOCUS_GAIN);
        Assert.assertEquals(
                AudioManager.AUDIOFOCUS_GAIN, mAudioFocusChangeListener.getAudioFocusState());

        // Audio focus notification is OS-driven.
        Thread.sleep(AUDIO_FOCUS_CHANGE_TIMEOUT);

        // Video did not pause.
        Assert.assertFalse(DOMUtils.isMediaPaused(tab.getWebContents(), VIDEO_ID));

        // Still no notification.
        Assert.assertFalse(isMediaNotificationVisible());
    }

    @Test
    @SmallTest
    @Restriction(RESTRICTION_TYPE_NON_LOW_END_DEVICE)
    public void testAutoplayMutedThenUnmute() throws Exception {
        Tab tab = mActivityTestRule.getActivity().getActivityTab();

        // Taking audio focus.
        Assert.assertEquals(
                AudioManager.AUDIOFOCUS_LOSS, mAudioFocusChangeListener.getAudioFocusState());
        mAudioFocusChangeListener.requestAudioFocus(AudioManager.AUDIOFOCUS_GAIN);
        Assert.assertEquals(
                AudioManager.AUDIOFOCUS_GAIN, mAudioFocusChangeListener.getAudioFocusState());

        // The page will autoplay the video.
        DOMUtils.waitForMediaPlay(tab.getWebContents(), VIDEO_ID);

        StringBuilder sb = new StringBuilder();
        sb.append("(function() {");
        sb.append("  var video = document.querySelector('video');");
        sb.append("  video.muted = false;");
        sb.append("  return video.muted;");
        sb.append("})();");

        // Unmute from script.
        String result = JavaScriptUtils.executeJavaScriptAndWaitForResult(
                tab.getWebContents(), sb.toString());
        Assert.assertTrue(result.trim().equalsIgnoreCase("false"));

        // Video is paused.
        Assert.assertTrue(DOMUtils.isMediaPaused(tab.getWebContents(), VIDEO_ID));

        // Audio focus notification is OS-driven.
        Thread.sleep(AUDIO_FOCUS_CHANGE_TIMEOUT);

        // Audio focus was not taken and no notification is visible.
        Assert.assertEquals(
                AudioManager.AUDIOFOCUS_GAIN, mAudioFocusChangeListener.getAudioFocusState());
        Assert.assertFalse(isMediaNotificationVisible());
    }
}
