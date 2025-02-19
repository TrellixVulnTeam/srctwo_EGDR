// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.omnibox;

import android.os.Bundle;
import android.speech.RecognizerIntent;
import android.support.test.annotation.UiThreadTest;
import android.support.test.filters.SmallTest;
import android.support.test.rule.UiThreadTestRule;
import android.text.TextUtils;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.RuleChain;
import org.junit.runner.RunWith;

import org.chromium.base.test.BaseJUnit4ClassRunner;
import org.chromium.base.test.util.Feature;
import org.chromium.chrome.browser.omnibox.OmniboxSuggestion.MatchClassification;
import org.chromium.chrome.browser.omnibox.VoiceSuggestionProvider.VoiceResult;
import org.chromium.chrome.browser.test.ChromeBrowserTestRule;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * Test the {@link VoiceSuggestionProvider} class through simulating omnibox results and voice
 * recognition result bundles.
 */
@RunWith(BaseJUnit4ClassRunner.class)
public class VoiceSuggestionProviderTest {
    @Rule
    public final RuleChain mChain =
            RuleChain.outerRule(new ChromeBrowserTestRule()).around(new UiThreadTestRule());

    private static OmniboxSuggestion createDummySuggestion(String text) {
        List<MatchClassification> classifications = new ArrayList<>();
        classifications.add(new MatchClassification(0, MatchClassificationStyle.NONE));
        return new OmniboxSuggestion(OmniboxSuggestionType.SEARCH_SUGGEST,
                true,
                0,
                1,
                text,
                classifications,
                null,
                classifications,
                null,
                null,
                "",
                "http://www.google.com",
                false,
                false);
    }

    private static List<OmniboxSuggestion> createDummySuggestions(String... texts) {
        List<OmniboxSuggestion> suggestions = new ArrayList<OmniboxSuggestion>(texts.length);
        for (int i = 0; i < texts.length; ++i) {
            suggestions.add(createDummySuggestion(texts[i]));
        }

        return suggestions;
    }

    private static Bundle createDummyBundle(String[] texts, float[] confidences) {
        Bundle b = new Bundle();

        b.putStringArrayList(RecognizerIntent.EXTRA_RESULTS, new ArrayList<String>(
                Arrays.asList(texts)));
        b.putFloatArray(RecognizerIntent.EXTRA_CONFIDENCE_SCORES, confidences);

        return b;
    }

    private static void assertVoiceResultsAreEqual(List<VoiceResult> results, String[] texts,
            float[] confidences) {
        Assert.assertTrue("Invalid array sizes",
                results.size() == texts.length && texts.length == confidences.length);

        for (int i = 0; i < texts.length; ++i) {
            VoiceResult result = results.get(i);
            Assert.assertEquals("Match text is not equal", texts[i], result.getMatch());
            Assert.assertEquals(
                    "Confidence is not equal", confidences[i], result.getConfidence(), 0);
        }
    }

    private static boolean assertSuggestionMatchesVoiceResult(OmniboxSuggestion a, VoiceResult b) {
        return a.getType() == OmniboxSuggestionType.VOICE_SUGGEST && !a.isStarred()
                && TextUtils.equals(a.getDisplayText(), b.getMatch());
    }

    private void assertArrayStartsWith(List<OmniboxSuggestion> a, List<OmniboxSuggestion> b) {
        Assert.assertTrue((a != null && b != null) || (a == null && b == null));
        if (a == null || b == null) return;

        Assert.assertTrue(a.size() >= b.size());
        for (int i = 0; i < b.size(); ++i) {
            Assert.assertEquals(
                    "The OmniboxSuggestion entries are not the same.", a.get(i), b.get(i));
        }
    }

    private void assertArrayEndsWith(List<OmniboxSuggestion> a, List<VoiceResult> b,
            int expectedResultCount) {
        Assert.assertTrue((a != null && b != null) || (a == null && b == null));
        if (a == null || b == null) return;

        expectedResultCount = Math.min(expectedResultCount, b.size());
        Assert.assertTrue(a.size() >= expectedResultCount);
        for (int i = 0; i < expectedResultCount; ++i) {
            Assert.assertTrue("The OmniboxSuggestion entry does not match the VoiceResult",
                    assertSuggestionMatchesVoiceResult(
                            a.get(a.size() - expectedResultCount + i), b.get(i)));
        }
    }

    private boolean isVoiceResultInSuggestions(List<OmniboxSuggestion> suggestions,
            VoiceResult result) {
        for (OmniboxSuggestion suggestion : suggestions) {
            if (assertSuggestionMatchesVoiceResult(suggestion, result)) return true;
        }

        return false;
    }

    @Test
    @SmallTest
    @UiThreadTest
    @Feature({"Omnibox"})
    public void testParseEmptyBundle() {
        VoiceSuggestionProvider provider = new VoiceSuggestionProvider(0.0f, 1.1f);

        provider.setVoiceResultsFromIntentBundle(new Bundle());

        Assert.assertNotNull("Results is null", provider.getResults());
        Assert.assertEquals(
                "SuggestionProvider added invalid results", 0, provider.getResults().size());

        provider.addVoiceSuggestions(null, 3);
    }

    @Test
    @SmallTest
    @UiThreadTest
    @Feature({"Omnibox"})
    public void testParseBundle() {
        VoiceSuggestionProvider provider = new VoiceSuggestionProvider(0.0f, 1.1f);

        String[] texts = new String[] {"a", "b", "c"};
        float[] confidences = new float[] {0.8f, 1.0f, 1.0f};

        provider.setVoiceResultsFromIntentBundle(createDummyBundle(texts, confidences));

        assertVoiceResultsAreEqual(provider.getResults(), texts, confidences);
    }

    @Test
    @SmallTest
    @UiThreadTest
    @Feature({"Omnibox"})
    public void testNoSuggestions() {
        VoiceSuggestionProvider provider = new VoiceSuggestionProvider(0.0f, 1.1f);
        List<OmniboxSuggestion> suggestions = createDummySuggestions("a", "b", "c");
        List<OmniboxSuggestion> updatedSuggestions = provider.addVoiceSuggestions(suggestions, 10);

        android.test.MoreAsserts.assertEquals(suggestions.toArray(), updatedSuggestions.toArray());
    }

    @Test
    @SmallTest
    @UiThreadTest
    public void testClearVoiceResults() {
        VoiceSuggestionProvider provider = new VoiceSuggestionProvider(0.0f, 1.1f);

        String[] texts = new String[] {"a", "b", "c"};
        float[] confidences = new float[] {1.0f, 0.99f, 0.98f};

        provider.setVoiceResultsFromIntentBundle(createDummyBundle(texts, confidences));

        Assert.assertNotNull(provider.getResults());
        Assert.assertEquals(
                "Invalid number of results", texts.length, provider.getResults().size());

        provider.clearVoiceSearchResults();

        Assert.assertEquals(0, provider.getResults().size());
    }

    @Test
    @SmallTest
    @UiThreadTest
    @Feature({"Omnibox"})
    public void testAddToEmtpyResultsCase() {
        VoiceSuggestionProvider provider = new VoiceSuggestionProvider(0.0f, 1.1f);

        String[] texts = new String[] {"a", "b", "c"};
        float[] confidences = new float[] {1.0f, 1.0f, 1.0f};

        provider.setVoiceResultsFromIntentBundle(createDummyBundle(texts, confidences));
        assertVoiceResultsAreEqual(provider.getResults(), texts, confidences);
        List<OmniboxSuggestion> suggestions = provider.addVoiceSuggestions(null, texts.length);
        assertArrayEndsWith(suggestions, provider.getResults(), texts.length);
    }

    @Test
    @SmallTest
    @UiThreadTest
    @Feature({"Omnibox"})
    public void testAddToEmptyOverflowCase() {
        VoiceSuggestionProvider provider = new VoiceSuggestionProvider(0.0f, 1.1f);

        String[] texts = new String[] {"a", "b", "c"};
        float[] confidences = new float[] {1.0f, 1.0f, 1.0f};

        provider.setVoiceResultsFromIntentBundle(createDummyBundle(texts, confidences));
        assertVoiceResultsAreEqual(provider.getResults(), texts, confidences);

        List<OmniboxSuggestion> suggestions = provider.addVoiceSuggestions(null, 2);
        assertArrayEndsWith(suggestions, provider.getResults(), 2);
    }

    @Test
    @SmallTest
    @UiThreadTest
    public void testAddToResultsCase() {
        VoiceSuggestionProvider provider = new VoiceSuggestionProvider(0.0f, 1.1f);

        String[] texts = new String[] {"a", "b", "c"};
        float[] confidences = new float[] {1.0f, 1.0f, 1.0f};

        provider.setVoiceResultsFromIntentBundle(createDummyBundle(texts, confidences));

        List<OmniboxSuggestion> suggestions = createDummySuggestions("oa", "ob", "oc");
        List<OmniboxSuggestion> updatedSuggestions =
                provider.addVoiceSuggestions(suggestions, texts.length);

        assertArrayStartsWith(updatedSuggestions, suggestions);
        assertArrayEndsWith(updatedSuggestions, provider.getResults(), texts.length);
    }

    @Test
    @SmallTest
    @UiThreadTest
    @Feature({"Omnibox"})
    public void testAddToResultsOverflowCase() {
        VoiceSuggestionProvider provider = new VoiceSuggestionProvider(0.0f, 1.1f);

        String[] texts = new String[] {"a", "b", "c"};
        float[] confidences = new float[] {1.0f, 1.0f, 1.0f};

        provider.setVoiceResultsFromIntentBundle(createDummyBundle(texts, confidences));

        List<OmniboxSuggestion> suggestions = createDummySuggestions("oa", "ob", "oc");
        List<OmniboxSuggestion> updatedSuggestions = provider.addVoiceSuggestions(suggestions, 2);

        assertArrayStartsWith(updatedSuggestions, suggestions);
        assertArrayEndsWith(updatedSuggestions, provider.getResults(), 2);
    }

    @Test
    @SmallTest
    @UiThreadTest
    @Feature({"Omnibox"})
    public void testAddDuplicateToResultsCase() {
        VoiceSuggestionProvider provider = new VoiceSuggestionProvider(0.0f, 1.1f);

        String[] texts = new String[] {"a", "b", "c"};
        float[] confidences = new float[] {1.0f, 1.0f, 1.0f};
        provider.setVoiceResultsFromIntentBundle(createDummyBundle(texts, confidences));

        List<OmniboxSuggestion> suggestions = createDummySuggestions("oa", "b", "oc");
        List<OmniboxSuggestion> updatedSuggestions =
                provider.addVoiceSuggestions(suggestions, texts.length);

        Assert.assertEquals(provider.getResults().get(0).getMatch(), texts[0]);
        Assert.assertTrue("Result 'a' was not found.",
                isVoiceResultInSuggestions(updatedSuggestions, provider.getResults().get(0)));
        Assert.assertEquals(provider.getResults().get(1).getMatch(), texts[1]);
        Assert.assertFalse("Result 'b' was found.",
                isVoiceResultInSuggestions(updatedSuggestions, provider.getResults().get(1)));
        Assert.assertEquals(provider.getResults().get(2).getMatch(), texts[2]);
        Assert.assertTrue("Result 'c' was not found.",
                isVoiceResultInSuggestions(updatedSuggestions, provider.getResults().get(2)));
    }

    @Test
    @SmallTest
    @UiThreadTest
    @Feature({"Omnibox"})
    public void testConfidenceThresholdHideLowConfidence() {
        VoiceSuggestionProvider provider = new VoiceSuggestionProvider(0.5f, 1.1f);

        String[] texts = new String[] {"a", "b", "c"};
        float[] confidences = new float[] {1.0f, 0.6f, 0.3f};
        provider.setVoiceResultsFromIntentBundle(createDummyBundle(texts, confidences));

        List<OmniboxSuggestion> suggestions = createDummySuggestions("oa", "ob", "oc");
        List<OmniboxSuggestion> updatedSuggestions =
                provider.addVoiceSuggestions(suggestions, texts.length);

        Assert.assertEquals(provider.getResults().get(0).getMatch(), texts[0]);
        Assert.assertTrue("Result 'a' was not found.",
                isVoiceResultInSuggestions(updatedSuggestions, provider.getResults().get(0)));
        Assert.assertEquals(provider.getResults().get(1).getMatch(), texts[1]);
        Assert.assertTrue("Result 'b' was not found.",
                isVoiceResultInSuggestions(updatedSuggestions, provider.getResults().get(1)));
        Assert.assertEquals(provider.getResults().get(2).getMatch(), texts[2]);
        Assert.assertFalse("Result 'c' was found.",
                isVoiceResultInSuggestions(updatedSuggestions, provider.getResults().get(2)));
    }

    @Test
    @SmallTest
    @UiThreadTest
    @Feature({"Omnibox"})
    public void testConfidenceThresholdHideAlts() {
        VoiceSuggestionProvider provider = new VoiceSuggestionProvider(0.5f, 0.5f);

        String[] texts = new String[] {"a", "b", "c"};
        float[] confidences = new float[] {0.8f, 1.0f, 1.0f};
        provider.setVoiceResultsFromIntentBundle(createDummyBundle(texts, confidences));

        List<OmniboxSuggestion> suggestions = createDummySuggestions("oa", "ob", "oc");
        List<OmniboxSuggestion> updatedSuggestions =
                provider.addVoiceSuggestions(suggestions, texts.length);

        Assert.assertEquals(provider.getResults().get(0).getMatch(), texts[0]);
        Assert.assertTrue("Result 'a' was not found (First entry.  Must be shown).",
                isVoiceResultInSuggestions(updatedSuggestions, provider.getResults().get(0)));
        Assert.assertEquals(provider.getResults().get(1).getMatch(), texts[1]);
        Assert.assertFalse("Result 'b' was found.",
                isVoiceResultInSuggestions(updatedSuggestions, provider.getResults().get(1)));
        Assert.assertEquals(provider.getResults().get(2).getMatch(), texts[2]);
        Assert.assertFalse("Result 'c' was found.",
                isVoiceResultInSuggestions(updatedSuggestions, provider.getResults().get(2)));
    }

    @Test
    @SmallTest
    @UiThreadTest
    @Feature({"Omnibox"})
    public void testVoiceResponseURLConversion() {
        VoiceSuggestionProvider provider = new VoiceSuggestionProvider();

        String[] texts = new String[] {"a", "www. b .co .uk", "engadget .com", "www.google.com"};
        float[] confidences = new float[] {1.0f, 1.0f, 1.0f, 1.0f};
        provider.setVoiceResultsFromIntentBundle(createDummyBundle(texts, confidences));

        assertVoiceResultsAreEqual(provider.getResults(),
                new String[] {"a", "www.b.co.uk", "engadget.com", "www.google.com"},
                new float[] {1.0f, 1.0f, 1.0f, 1.0f});
    }
}
