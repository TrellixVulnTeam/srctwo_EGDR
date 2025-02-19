// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.infobar;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;

import org.chromium.base.ContextUtils;
import org.chromium.base.annotations.CalledByNative;
import org.chromium.chrome.browser.ResourceId;
import org.chromium.chrome.browser.preferences.Preferences;
import org.chromium.chrome.browser.preferences.PreferencesLauncher;
import org.chromium.chrome.browser.preferences.website.SingleWebsitePreferences;
import org.chromium.chrome.browser.util.IntentUtils;

/**
 * An infobar to disclose to the user that the default search engine has geolocation access by
 * default.
 */
public class SearchGeolocationDisclosureInfoBar extends InfoBar {
    private final int mInlineLinkRangeStart;
    private final int mInlineLinkRangeEnd;

    @CalledByNative
    private static InfoBar show(int enumeratedIconId, String messageText, int inlineLinkRangeStart,
            int inlineLinkRangeEnd) {
        int drawableId = ResourceId.mapToDrawableId(enumeratedIconId);
        return new SearchGeolocationDisclosureInfoBar(
                drawableId, messageText, inlineLinkRangeStart, inlineLinkRangeEnd);
    }

    /**
     * Creates the infobar.
     * @param iconDrawableId       Drawable ID corresponding to the icon that the infobar will show.
     * @param messageText          Message to display in the infobar.
     * @param inlineLinkRangeStart Beginning of the link in the message.
     * @param inlineLinkRangeEnd   End of the link in the message.
     */
    private SearchGeolocationDisclosureInfoBar(int iconDrawableId, String messageText,
            int inlineLinkRangeStart, int inlineLinkRangeEnd) {
        super(iconDrawableId, null, messageText);
        mInlineLinkRangeStart = inlineLinkRangeStart;
        mInlineLinkRangeEnd = inlineLinkRangeEnd;
    }

    @Override
    public void createContent(InfoBarLayout layout) {
        super.createContent(layout);
        layout.setInlineMessageLink(mInlineLinkRangeStart, mInlineLinkRangeEnd);
    }

    @Override
    public boolean isLegalDisclosure() {
        return true;
    }

    @CalledByNative
    private static void showSettingsPage(String searchUrl) {
        Context context = ContextUtils.getApplicationContext();
        Intent settingsIntent = PreferencesLauncher.createIntentForSettingsPage(
                context, SingleWebsitePreferences.class.getName());
        Bundle fragmentArgs = SingleWebsitePreferences.createFragmentArgsForSite(searchUrl);
        settingsIntent.putExtra(Preferences.EXTRA_SHOW_FRAGMENT_ARGUMENTS, fragmentArgs);
        IntentUtils.safeStartActivity(context, settingsIntent);
    }
}
