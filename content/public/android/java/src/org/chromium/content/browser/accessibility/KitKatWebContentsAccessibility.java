// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.content.browser.accessibility;

import android.accessibilityservice.AccessibilityServiceInfo;
import android.annotation.TargetApi;
import android.content.Context;
import android.os.Build;
import android.os.Bundle;
import android.view.ViewGroup;
import android.view.accessibility.AccessibilityNodeInfo;

import org.chromium.base.annotations.JNINamespace;
import org.chromium.content.browser.RenderCoordinates;
import org.chromium.content_public.browser.WebContents;

/**
 * Subclass of WebContentsAccessibility for KitKat.
 */
@JNINamespace("content")
@TargetApi(Build.VERSION_CODES.KITKAT)
public class KitKatWebContentsAccessibility extends WebContentsAccessibility {
    private String mSupportedHtmlElementTypes;

    KitKatWebContentsAccessibility(Context context, ViewGroup containerView,
            WebContents webContents, RenderCoordinates renderCoordinates,
            boolean shouldFocusOnPageLoad) {
        super(context, containerView, webContents, renderCoordinates, shouldFocusOnPageLoad);
        mSupportedHtmlElementTypes = nativeGetSupportedHtmlElementTypes(mNativeObj);
    }

    @Override
    protected void setAccessibilityNodeInfoKitKatAttributes(AccessibilityNodeInfo node,
            boolean isRoot, boolean isEditableText, String role, String roleDescription,
            String hint, int selectionStartIndex, int selectionEndIndex) {
        Bundle bundle = node.getExtras();
        bundle.putCharSequence("AccessibilityNodeInfo.chromeRole", role);
        bundle.putCharSequence("AccessibilityNodeInfo.roleDescription", roleDescription);
        bundle.putCharSequence("AccessibilityNodeInfo.hint", hint);
        if (isRoot) {
            bundle.putCharSequence(
                    "ACTION_ARGUMENT_HTML_ELEMENT_STRING_VALUES", mSupportedHtmlElementTypes);
        }
        if (isEditableText) {
            node.setEditable(true);
            node.setTextSelection(selectionStartIndex, selectionEndIndex);
        }
    }

    @Override
    protected int getAccessibilityServiceCapabilitiesMask() {
        int capabilitiesMask = 0;
        for (AccessibilityServiceInfo service :
                mAccessibilityManager.getEnabledAccessibilityServiceList(
                        AccessibilityServiceInfo.FEEDBACK_ALL_MASK)) {
            capabilitiesMask |= service.getCapabilities();
        }
        return capabilitiesMask;
    }
}
