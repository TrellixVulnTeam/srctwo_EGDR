// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.android_webview;

import android.content.Context;
import android.graphics.Color;
import android.view.View;
import android.view.ViewGroup;

import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;
import org.chromium.components.autofill.AutofillDelegate;
import org.chromium.components.autofill.AutofillPopup;
import org.chromium.components.autofill.AutofillSuggestion;
import org.chromium.content.browser.ContentViewCore;
import org.chromium.ui.DropdownItem;
import org.chromium.ui.base.WindowAndroid;

/**
 * Java counterpart to the AwAutofillClient. This class is owned by AwContents and has
 * a weak reference from native side.
 */
@JNINamespace("android_webview")
public class AwAutofillClient {

    private final long mNativeAwAutofillClient;
    private AutofillPopup mAutofillPopup;
    private ViewGroup mContainerView;
    private ContentViewCore mContentViewCore;

    @CalledByNative
    public static AwAutofillClient create(long nativeClient) {
        return new AwAutofillClient(nativeClient);
    }

    private AwAutofillClient(long nativeAwAutofillClient) {
        mNativeAwAutofillClient = nativeAwAutofillClient;
    }

    public void init(ContentViewCore contentViewCore) {
        mContentViewCore = contentViewCore;
        mContainerView = contentViewCore.getContainerView();
    }

    @CalledByNative
    private void showAutofillPopup(View anchorView, boolean isRtl,
            AutofillSuggestion[] suggestions) {

        if (mContentViewCore == null) return;

        if (mAutofillPopup == null) {
            Context context = mContentViewCore.getContext();
            if (WindowAndroid.activityFromContext(context) == null) {
                nativeDismissed(mNativeAwAutofillClient);
                return;
            }
            try {
                mAutofillPopup = new AutofillPopup(context, anchorView, new AutofillDelegate() {
                    @Override
                    public void dismissed() {
                        nativeDismissed(mNativeAwAutofillClient);
                    }
                    @Override
                    public void suggestionSelected(int listIndex) {
                        nativeSuggestionSelected(mNativeAwAutofillClient, listIndex);
                    }
                    @Override
                    public void deleteSuggestion(int listIndex) {}

                    @Override
                    public void accessibilityFocusCleared() {}
                });
            } catch (RuntimeException e) {
                // Deliberately swallowing exception because bad fraemwork implementation can
                // throw exceptions in ListPopupWindow constructor.
                nativeDismissed(mNativeAwAutofillClient);
                return;
            }
        }
        mAutofillPopup.filterAndShow(suggestions, isRtl, Color.TRANSPARENT /* backgroundColor */,
                Color.TRANSPARENT /* dividerColor */, 0 /* dropdownItemHeight */, 0 /* margin */);
    }

    @CalledByNative
    public void hideAutofillPopup() {
        if (mAutofillPopup == null) return;
        mAutofillPopup.dismiss();
        mAutofillPopup = null;
    }

    @CalledByNative
    private static AutofillSuggestion[] createAutofillSuggestionArray(int size) {
        return new AutofillSuggestion[size];
    }

    /**
     * @param array AutofillSuggestion array that should get a new suggestion added.
     * @param index Index in the array where to place a new suggestion.
     * @param name Name of the suggestion.
     * @param label Label of the suggestion.
     * @param uniqueId Unique suggestion id.
     */
    @CalledByNative
    private static void addToAutofillSuggestionArray(AutofillSuggestion[] array, int index,
            String name, String label, int uniqueId) {
        array[index] = new AutofillSuggestion(name, label, DropdownItem.NO_ICON,
                false /* isIconAtLeft */, uniqueId, false /* isDeletable */,
                false /* isMultilineLabel */, false /* isBoldLabel */);
    }

    private native void nativeDismissed(long nativeAwAutofillClient);
    private native void nativeSuggestionSelected(long nativeAwAutofillClient,
            int position);
}
